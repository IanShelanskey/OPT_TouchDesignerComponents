#pragma once

#ifdef OPT_CHOP_EXPORTS
#define OPT_CHOP_API __declspec(dllexport)
#else
#define OPT_CHOP_API __declspec(dllimport)
#endif
#define BUFLEN 65507
#include "CHOP_CPlusPlusBase.h"
#include <map>
#include <vector>
#include <winsock2.h>

class OPT_CHOP : public CHOP_CPlusPlusBase
{
public:


	OPT_CHOP(const OP_NodeInfo * info);

	virtual ~OPT_CHOP();

	virtual void getGeneralInfo(CHOP_GeneralInfo* ginfo) override;

	virtual bool getOutputInfo(CHOP_OutputInfo * info)override;

	virtual const char * getChannelName(int index, void * reserved) override;

	virtual void execute(const CHOP_Output*, OP_Inputs*, void* reserved) override;

	virtual void setupParameters(OP_ParameterManager * manager) override;

private:
	const OP_NodeInfo *myNodeInfo;
	char buf[BUFLEN];
	bool listening;
	int seq;
	const char* names[6] = { "id", "age", "confidence", "x", "y", "height"};
	std::map<float, std::vector<float>> data;

	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len;
	WSADATA wsa;


};


