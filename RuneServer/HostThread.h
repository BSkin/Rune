#ifndef HOST_THREAD_H
#define HOST_THREAD_H

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <process.h>
#include <iostream>
#include "..\Rune\Settings.h"
#include "..\Rune\Client.h"
using std::cout;
using std::cin;

class HostThread
{
public:
	static HANDLE start(SOCKET hostSocket);
	static void stopAll();

	static int aquireClientsMutex(int index);
	static int releaseClientsMutex(int index);
	static int getFirstAvailableClientID(string clientName, unsigned long ipAddress, unsigned long portTCP, unsigned long portUDP);
	static void freeClient(int index);

	static void setStatics(int * maxClients, vector<Client*> * clients);
private:
	static unsigned int WINAPI startHostThread(LPVOID);
	static bool running;

	static int * maxClients;
	static vector<Client*> * clients;
};

#endif