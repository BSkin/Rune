#ifndef CLIENT_H
#define CLIENT_H

#include "Packet.h"
#include "DataBuffer.h"
#include "NetworkFunctions.h"

#include <string>
using std::string;

class Client
{
public:
	Client();
	~Client();

	DataBuffer * popSendData();
	void pushSendData(const char * c, int size);
	void pushSendData(char * c, int size) { pushSendData((const char *)c, size); }
	DataBuffer * popReceiveData();
	void pushReceiveData(const char * c, int size);
	void pushReceiveData(char * c, int size) { pushReceiveData((const char *)c, size); }
	Packet * getPacketAddress() { return &packetData; }
	unsigned long getIPAddress() { return ipAddress; }
	unsigned long getPortTCP() { return portTCP; }
	unsigned long getPortUDP() { return portUDP; }
	unsigned long getFrameCount() { return frameCount; }

	void setConnected(string name = "Peasant");
	void setDisconnected();
	bool isConnected() { return connected; }
	void setName(string x) { name = x; }
	string getName() { return name; }
	void setPing(int x) { ping = x; }
	void setID(int x) { clientID = x; }
	void setIPAddress(unsigned long x) { ipAddress = x; }
	void setPortTCP(unsigned long x) { portTCP = x; }
	void setPortUDP(unsigned long x) { portUDP = x; }
	void setFrameCount(unsigned long x) { frameCount = x; }

	void setSynch(bool x) { synch = x; }
	bool requiresSynch() { return synch; }

	int aquireMutex();
	int releaseMutex();
private:
	void init();

	Packet packetData;
	bool connected;
	string name;
	int ping;
	int clientID;
	unsigned long ipAddress, portTCP, portUDP;

	unsigned long frameCount;

	bool synch;

	HANDLE clientMutex;
};

#endif CLIENT_H