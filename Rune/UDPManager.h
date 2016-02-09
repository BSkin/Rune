#ifndef UDP_MANAGER_H
#define UDP_MANAGER_H

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <stdio.h>

#include "DataBuffer.h"
#include "Settings.h"
#include "Utilities.h"

class UDPManager
{
public:
	UDPManager();
	~UDPManager();
	
	//static void setNetID(int netID) { UDPManager::netID = netID; }

	static int init(USHORT port);
	static int cleanup();

	static DataBuffer * recvDatagram(sockaddr_in ** originAddress = NULL);
	static int sendDatagram(DataBuffer * data, unsigned long ipAddress, unsigned long port, unsigned long frameCount);
	static int sendDatagram(const char * data, int length, unsigned long ipAddress, unsigned long port, unsigned long frameCount);
private:
	static SOCKET sock;
	static struct sockaddr_in myAddress;
	static bool initErrors;
};

#endif