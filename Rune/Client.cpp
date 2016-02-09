#include "Client.h"

Client::Client()
{
	clientID = -1;
	clientMutex = CreateMutex(NULL, FALSE, NULL);
	init();
	NetworkFunctions::translateReceivedEvents(&packetData);
}

Client::~Client()
{
	packetData.cleanup();
	if (clientMutex != NULL) {
		CloseHandle(clientMutex);
		clientMutex = NULL;
	}
}

void Client::init()
{
	ipAddress = inet_addr("127.0.0.1");
	portUDP = 0;
	connected = false;
	name = "";
	ping = 0;
	synch = false;
	frameCount = 0;
}

DataBuffer * Client::popSendData()
{
	DataBuffer * temp = new DataBuffer(packetData.getSendBuffer(), packetData.getSendBufferSize());
	packetData.refillSendBuffer();
	return temp;
}

void Client::pushSendData(const char * data, int size)
{
	packetData.appendToSendBuffer(data, size);
}

DataBuffer * Client::popReceiveData()
{
	DataBuffer * temp;
	if (packetData.getReceiveBufferSize() == 0)
		temp = new DataBuffer("", 0);
	else
		temp = new DataBuffer(packetData.getReceiveBuffer()+4, packetData.getReceiveBufferSize()-4);
	packetData.refillReceiveBuffer();
	return temp;
}

void Client::pushReceiveData(const char * data, int size)
{
	packetData.appendToReceiveBuffer(data, size);
}

void Client::setConnected(string name)
{
	connected = true;
	this->name = name;
	//Send Event
}

void Client::setDisconnected()
{
	init();
}

int Client::aquireMutex()
{
	DWORD waitResult = WaitForSingleObject(clientMutex, INFINITE);
	if (waitResult != WAIT_OBJECT_0) {
		printf("Error aquiring client %d mutex.\n", clientID);
		return -1;
	}
	return 0;
}

int Client::releaseMutex()
{
	if (!ReleaseMutex(clientMutex)) {
		printf("Error releasing client %d mutex.\n", clientID);
		return -1;
	}
	return 0;
}