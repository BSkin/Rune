#ifndef PACKET_H
#define PACKET_H

//#include "GameObject.h"
#include "EventSerializer.h"
#include "DataBuffer.h"
#include "Settings.h"

#include <sstream>
#include <iostream>
#include <string>
#include <list>
#include <array>
using std::istringstream;
using std::getline;
using std::string;
using std::list;
using std::array;

#define NETWORK_STATE_OFFLINE 0
#define NETWORK_STATE_CLIENT 1
#define	NETWORK_STATE_SERVER 2

class Packet
{
public:
	Packet();
	~Packet();

	const char * getSendBuffer();
	int getSendBufferSize();
	
	const char * getReceiveBuffer();
	int getReceiveBufferSize();

	void appendToSendBuffer(const char * x, int size);
	void appendToSendBuffer(char * x, int size);
	void appendToSendBuffer(DataBuffer *);
	//void appendToSendBuffer(GameObject * o);
	void refillSendBuffer();
	void clearSendBuffer();

	void appendToReceiveBuffer(const char * x, int size);
	void appendToReceiveBuffer(char * x, int size);
	void appendToReceiveBuffer(DataBuffer *);
	//void appendToReceiveBuffer(GameObject * o);
	void refillReceiveBuffer();
	void clearReceiveBuffer();

	void cleanup();
private:
	void fillSendBuffer();
	void fillReceiveBuffer();

	char sendBuffer[MAX_PACKET_SIZE-2];
	int sendBufferSize;
	list<DataBuffer *> sendBackBuffer;

	char receiveBuffer[MAX_PACKET_SIZE-2];
	int receiveBufferSize;
	list<DataBuffer *> receiveBackBuffer;
};

#endif