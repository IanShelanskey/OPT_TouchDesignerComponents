#pragma once
#include "stdafx.h"
#include "OPT_Client.h"
#include <winsock2.h>
#include <thread>


#define BUFLEN 512*4  //Max length of buffer
#define PORT 21234   //The port on which to listen for incoming data

class OPT_Client
{
public:
	OPT_Client() 
	{
		//buf[BUFLEN];
		//printf("Memory address of buf: %d", &buf);
		//memset(buf, '\0', BUFLEN);
		//std::thread (&OPT_Client::Listen, this).detach();
	};

	virtual ~OPT_Client() 
	{
	};

	void startListener(char* outStr) {
		std::thread listener(&OPT_Client::Listen, this, outStr);
	}

	void Listen(char* outStr) {
		SOCKET s;
		struct sockaddr_in server, si_other;
		int slen, recv_len;
		WSADATA wsa;
		slen = sizeof(si_other);
		//initialise winsock
		printf("\nInitialising Winsock");
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
			//printf("Failed. Error code: %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		printf("Initailised.\n");

		//Create a socket
		if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
			printf("Could not create socket: %d", WSAGetLastError());
		}
		printf("Socket created. \n");

		//prep sockaddr_in structure
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_port = htons(PORT);

		//Bind
		if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
			printf("Bind failed");
			//exit(EXIT_FAILURE);
		}
		printf("Bind done");
		while (1) {
			//printf("waiting for data...");
			fflush(stdout);

			//clear buffer
			memset(outStr, '\0', BUFLEN);

			if ((recv_len = recvfrom(s, outStr, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR) {
				printf("recvfrom failed");
				//exit(EXIT_FAILURE);
			}
			//seq = header["seq"].GetInt();
			//print deets
			//printf("Received packet from %s:%d\n", InetNtop(si_other.sin_addr), ntohs(si_other.sin_port));
			//printf("Data: %s", buf);

		}
		//printf("\nData printed");
		//closesocket(s);
		//WSACleanup();
		//printf("\nCleanupDone");
	}

	char buf[BUFLEN];
};

