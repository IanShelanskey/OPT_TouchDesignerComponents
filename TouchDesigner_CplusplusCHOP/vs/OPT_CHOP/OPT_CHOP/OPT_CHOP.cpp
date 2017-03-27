//Copyright (c) 2017-2016 Ian Shelanskey ishelanskey@gmail.com 
//All rights reserved.

#include "stdafx.h"
#include <vector>
#include <map>
#include "OPT_CHOP.h"
#include <winsock2.h>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"


#ifdef OPT_CHOP_API  
#define OPT_CHOP_API __declspec(dllexport)   
#else  
#define OPT_CHOP_API __declspec(dllimport)   
#endif  
#define PORT 21234

#pragma comment(lib,"ws2_32.lib") //Winsock Library

//Required functions.
extern "C"
{
	//Lets Touch know the API version.
	OPT_CHOP_API int GetCHOPAPIVersion(void)
	{
		// Always return CHOP_CPLUSPLUS_API_VERSION in this function.
		return CHOP_CPLUSPLUS_API_VERSION;
	}

	//Lets Touch create a new Instance of the CHOP
	OPT_CHOP_API CHOP_CPlusPlusBase* CreateCHOPInstance(const OP_NodeInfo* info)
	{
		// Return a new instance of your class every time this is called.
		// It will be called once per CHOP that is using the .dll
		return new OPT_CHOP(info);
	}

	//Let's Touch Destroy Instance of CHOP
	OPT_CHOP_API void DestroyCHOPInstance(CHOP_CPlusPlusBase* instance)
	{
		// Delete the instance here, this will be called when
		// Touch is shutting down, when the CHOP using that instance is deleted, or
		// if the CHOP loads a different DLL
		delete (OPT_CHOP*)instance;
	}
};


OPT_CHOP::OPT_CHOP(const OP_NodeInfo * info)
{
	//Create a UDP Socket.
	DWORD timeout = 1;
	slen = sizeof(si_other);

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed. Error code: %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket: %d", WSAGetLastError());
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
		perror("Error");
	}
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
		printf("WinSock bind failed");
	}
}

OPT_CHOP::~OPT_CHOP()
{
	//Destroy UDP Socket
	closesocket(s);
	WSACleanup();
}

void OPT_CHOP::getGeneralInfo(CHOP_GeneralInfo * ginfo)
{
	//Forces check on socket each time TouchDesigner cooks. 
	ginfo->cookEveryFrame = true;
}

bool OPT_CHOP::getOutputInfo(CHOP_OutputInfo * info)
{
	//OPT Streaming data (ID, age, confidence, x, y, height).
	info->numChannels = 6;

	//Sets the maximum number of people to be tracked.
	info->numSamples = info->opInputs->getParInt("Maxtracked");
	
	return true;
}

const char* OPT_CHOP::getChannelName(int index, void* reserved)
{
	//ID, age, confidence, x, y, height
	return names[index];
}

void OPT_CHOP::execute(const CHOP_Output* output, OP_Inputs* inputs, void* reserved)
{
	fflush(stdout);

	//clear buffer
	memset(buf, '\0', BUFLEN);

	if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR) {
		//Should be an error here. Touch doesn't like that.
	}

	//If buffer isn't empty then parse and update channels.
	if (buf[0] != '\0') {

		//Create objects to hold incoming data
		std::vector<float> NewTracks;
		rapidjson::Document d;

		//Parse json from UDP buffer.
		d.Parse(buf);

		//Get all of the tracks from JSON. 
		const rapidjson::Value& tracks = d["tracks"].GetArray();


		//For each new track.
		for (rapidjson::SizeType i = 0; i < tracks.Size(); i++)
		{
			//Create a vector of new tracks data.
			std::vector<float> incoming = {	float(tracks[i]["age"].GetFloat()),
											float(tracks[i]["confidence"].GetFloat()),
											float(tracks[i]["x"].GetFloat()),
											float(tracks[i]["y"].GetFloat()),
											float(tracks[i]["height"].GetFloat()) };

			//Get id of track.
			float newid = float(tracks[i]["id"].GetInt());
			
			//Add id to vector of ids to use in find and replace operation later.
			NewTracks.push_back(newid);

			//Map data to its id.
			data.insert_or_assign(newid, incoming);
		}


		//Create counters for ID matching and sorting.
		int offset = 0; //New data
		int i = 0; //Old data

		//Go through each Track ID. 
		while(offset < NewTracks.size())
		{	
			//Find tracks not being used.
			if (i < output->numSamples) {
				//If the previous track is not in the new frame of data. 
				if (NewTracks[offset] != output->channels[0][i]) {
					//Change its id to -1.
					output->channels[0][i] = -1;
					//Increase old track counter.   
					i++;
					continue;
				}
				else {
					//We are still tracking this old ID. 
					i++;
					offset++;
				}
			}
			else {
				//Look for any ids that are -1 to replace with overflow track data. 
				float * p;
				p = std::find(output->channels[0], output->channels[0]+output->numSamples, -1);
				if (p == output->channels[0] + output->numSamples) {
					//There are no empty slots for new data.
					break;
				}
				else {
					//Replace empty slot with overflow track data.
					if (NewTracks.size() - offset > 0 ) {
						//TODO add filters - age, confidence. 
						*p = NewTracks[offset];
					}
					else {
						//No overflow track data exists. Leave them as -1.
						break;
					}
				}
				offset++;
			}
		}
		//Iterate through IDs.
		for (int i = 0; i < output->numSamples; i++) {
			float lookupid = output->channels[0][i];
			
			if (lookupid < 0) {
				//Set any open slots to 0.
				output->channels[1][i] = 0;
				output->channels[2][i] = 0;
				output->channels[3][i] = 0;
				output->channels[4][i] = 0;
				output->channels[5][i] = 0;
			}
			else {
				//Lookup track data based on ID in data map.
				output->channels[1][i] = data[lookupid][0]; //age
				output->channels[2][i] = data[lookupid][1]; //confidence
				output->channels[3][i] = data[lookupid][2]; //x
				output->channels[4][i] = data[lookupid][3]; //y
				output->channels[5][i] = data[lookupid][4]; //height
			}
		}
	}
}

void OPT_CHOP::setupParameters(OP_ParameterManager* manager) 
{
	//Create new parameter 
	OP_NumericParameter MaxTracked;

	//Parameter details
	MaxTracked.name = "Maxtracked";
	MaxTracked.label = "Max Tracked";
	MaxTracked.page = "OPT General";
	MaxTracked.defaultValues[0] = 1;
	MaxTracked.minValues[0] = 1;

	//Add it to CHOP.
	ParAppendResult res = manager->appendInt(MaxTracked);
	assert(res == PARAMETER_APPEND_SUCCESS);

}