#include "Settings.h"

int Settings::windowWidth = 1;
int Settings::windowHeight = 1;
int Settings::windowState = WINDOWED;
int Settings::maxFPS = 1000;
string Settings::username = "Peasant";
int Settings::clientID = -1;
unsigned long Settings::serverIP = inet_addr("127.0.0.1");
unsigned long Settings::portTCP = 0;
unsigned long Settings::portUDP = 0;

HANDLE Settings::clientIDMutex = CreateMutex(NULL, FALSE, NULL);
HANDLE Settings::usernameMutex = CreateMutex(NULL, FALSE, NULL);
HANDLE Settings::serverIPMutex = CreateMutex(NULL, FALSE, NULL);
HANDLE Settings::portTCPMutex = CreateMutex(NULL, FALSE, NULL);
HANDLE Settings::portUDPMutex = CreateMutex(NULL, FALSE, NULL);

void Settings::cleanup()
{
	if (clientIDMutex != NULL) 
		CloseHandle(clientIDMutex);
	if (usernameMutex != NULL) 
		CloseHandle(usernameMutex);
	if (serverIPMutex != NULL)
		CloseHandle(serverIPMutex);
	if (portTCPMutex != NULL)
		CloseHandle(portTCPMutex);
	if (portUDPMutex != NULL)
		CloseHandle(portUDPMutex);
}

void Settings::setClientID(int x)
{
	aquireClientIDMutex();
		clientID = x;
	releaseClientIDMutex();
}

int Settings::getClientID()
{
	int temp;
	aquireClientIDMutex();
		temp = clientID;
	releaseClientIDMutex();
	return temp;
}

void Settings::setUsername(string x)
{
	aquireUsernameMutex();
		username = x;
	releaseUsernameMutex();
}

string Settings::getUsername()
{
	string temp;
	aquireUsernameMutex();
		temp = username;
	releaseUsernameMutex();
	return temp;
}

void Settings::setServerIP(unsigned long x)
{
	aquireServerIPMutex();
		serverIP = x;
	releaseServerIPMutex();
}

unsigned long Settings::getServerIP()
{
	unsigned long temp;
	aquireServerIPMutex();
		 temp = serverIP;
	releaseServerIPMutex();
	return temp;
}

void Settings::setServerIPString(string x)
{
	unsigned long tempVal = inet_addr(x.c_str());
	aquireServerIPMutex();
		serverIP = tempVal;
	releaseServerIPMutex();
}

string Settings::getServerIPString()
{
	struct in_addr tempAddr; 
	aquireServerIPMutex();
		tempAddr.s_addr = serverIP;
	releaseServerIPMutex();
	return string(inet_ntoa(tempAddr));
}

void Settings::setPortTCP(unsigned long x)
{
	aquirePortTCPMutex();
		portTCP = x;
	releasePortTCPMutex();
}

unsigned long Settings::getPortTCP()
{
	unsigned long temp;
	aquirePortTCPMutex();
		temp = portTCP;
	releasePortTCPMutex();
	return temp;
}

void Settings::setPortUDP(unsigned long x)
{
	aquirePortUDPMutex();
		portUDP = x;
	releasePortUDPMutex();
}

unsigned long Settings::getPortUDP()
{
	unsigned long temp;
	aquirePortUDPMutex();
		temp = portUDP;
	releasePortUDPMutex();
	return temp;
}

int Settings::aquireClientIDMutex()
{
	DWORD waitResult = WaitForSingleObject(clientIDMutex, INFINITE);
	if (waitResult != WAIT_OBJECT_0) {
		printf("Error aquiring clientID mutex.\n");
		return -1;
	}
	return 0;
}

int Settings::releaseClientIDMutex()
{
	if (!ReleaseMutex(clientIDMutex)) {
		printf("Error releasing clientID mutex.\n");
		return -1;
	}
	return 0;
}

int Settings::aquireUsernameMutex()
{
	DWORD waitResult = WaitForSingleObject(usernameMutex, INFINITE);
	if (waitResult != WAIT_OBJECT_0) {
		printf("Error aquiring clientID mutex.\n");
		return -1;
	}
	return 0;
}

int Settings::releaseUsernameMutex()
{
	if (!ReleaseMutex(usernameMutex)) {
		printf("Error releasing clientID mutex.\n");
		return -1;
	}
	return 0;
}

int Settings::aquireServerIPMutex()
{
	DWORD waitResult = WaitForSingleObject(serverIPMutex, INFINITE);
	if (waitResult != WAIT_OBJECT_0) {
		printf("Error aquiring serverIP mutex.\n");
		return -1;
	}
	return 0;
}

int Settings::releaseServerIPMutex()
{
	if (!ReleaseMutex(serverIPMutex)) {
		printf("Error releasing serverIP mutex.\n");
		return -1;
	}
	return 0;
}

int Settings::aquirePortTCPMutex()
{
	DWORD waitResult = WaitForSingleObject(portTCPMutex, INFINITE);
	if (waitResult != WAIT_OBJECT_0) {
		printf("Error aquiring portTCP mutex.\n");
		return -1;
	}
	return 0;
}

int Settings::releasePortTCPMutex()
{
	if (!ReleaseMutex(portTCPMutex)) {
		printf("Error releasing portTCP mutex.\n");
		return -1;
	}
	return 0;
}

int Settings::aquirePortUDPMutex()
{
	DWORD waitResult = WaitForSingleObject(portUDPMutex, INFINITE);
	if (waitResult != WAIT_OBJECT_0) {
		printf("Error aquiring portUDP mutex.\n");
		return -1;
	}
	return 0;
}

int Settings::releasePortUDPMutex()
{
	if (!ReleaseMutex(portUDPMutex)) {
		printf("Error releasing portUDP mutex.\n");
		return -1;
	}
	return 0;
}