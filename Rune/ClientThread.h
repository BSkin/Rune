#ifndef CLIENT_THREAD_H
#define CLIENT_THREAD_H

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <process.h>
#include "Settings.h"
#include "Client.h"

class ClientThread
{
public:
	static HANDLE connectToServer(unsigned long ipAddress, USHORT port);
	static void disconnectFromServer();
	static bool isRunning();

	static int aquireRunningMutex();
	static int releaseRunningMutex();

	static void setStatics(Client *);
	static void cleanup();
private:
	static unsigned int WINAPI startClientThread(LPVOID);
	static bool running;
	static HANDLE runningMutex;

	static Client * client;
};

#endif