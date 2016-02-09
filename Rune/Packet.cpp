#include "Packet.h"

Packet::Packet()
{
	//sendBuffer = "";
	//sendBackBuffer; = list<string>();
	//receiveBuffer = "";
	//receiveBackBuffer; = list<string>();
	sendBufferSize = 0;
	receiveBufferSize = 0;
}

Packet::~Packet()
{
	cleanup();
}

void Packet::cleanup()
{
	clearSendBuffer();
	clearReceiveBuffer();

	sendBackBuffer.clear();
	receiveBackBuffer.clear();
}

const char * Packet::getSendBuffer() 
{ 
	return sendBuffer;
}

int Packet::getSendBufferSize()
{
	return sendBufferSize;
}

const char * Packet::getReceiveBuffer() 
{ 
	return receiveBuffer;
}

int Packet::getReceiveBufferSize()
{
	return receiveBufferSize;
}

void Packet::appendToSendBuffer(const char * s, int size)
{
	string line = "";
	int lineSize = 0;
	for (int i = 0; i < size; i++) {
		line += s[i];
		lineSize++;
		if (s[i] == '\n') {
			DataBuffer * temp = new DataBuffer(line.c_str(), lineSize);
			sendBackBuffer.push_back(temp);
			line = "";
			lineSize = 0;
		}
	}
	fillSendBuffer();
}

void Packet::appendToSendBuffer(char * s, int size) { appendToSendBuffer((const char *)s, size); }

void Packet::appendToSendBuffer(DataBuffer * dataBuffer)
{
	sendBackBuffer.push_back(dataBuffer);
	fillSendBuffer();
}

/*void Packet::appendToSendBuffer(GameObject * o)
{
	if (o != NULL) {
		DataBuffer * temp = o->serialize();
		if (temp != NULL) {
			//appendToSendBuffer(temp->getData, o->getSerializedSize());
			//delete [] temp;
			appendToSendBuffer(temp);
		}
	}
}*/

void Packet::refillSendBuffer()
{
	sendBuffer[0] = '\0';
	sendBufferSize = 0;
	fillSendBuffer();
}

void Packet::clearSendBuffer()
{
	sendBuffer[0] = '\0';
	sendBufferSize = 0;
	while (sendBackBuffer.size() > 0) {
		fillSendBuffer();
		sendBuffer[0] = '\0';
		sendBufferSize = 0;
	}
}

void Packet::appendToReceiveBuffer(const char * s, int size)
{
	string line = "";
	int lineSize = 0;
	for (int i = 0; i < size; i++) {
		line += s[i];
		lineSize++;
		if (s[i] == '\n') {
			DataBuffer * temp = new DataBuffer(line.c_str(), lineSize);
			receiveBackBuffer.push_back(temp);
			line = "";
			lineSize = 0;
		}
	}
	fillReceiveBuffer();
}

void Packet::appendToReceiveBuffer(char * s, int size) { appendToReceiveBuffer((const char*)s, size); }

void Packet::appendToReceiveBuffer(DataBuffer * dataBuffer)
{
	receiveBackBuffer.push_back(dataBuffer);
	fillReceiveBuffer();
}

/*void Packet::appendToReceiveBuffer(GameObject * o)
{
	if (o != NULL) {
		DataBuffer * temp = o->serialize();
		if (temp != NULL) {
			//appendToReceiveBuffer(temp->getData(), o->getSerializedSize());
			//delete [] temp;
			appendToReceiveBuffer(temp);
		}
	}
}*/

void Packet::refillReceiveBuffer()
{
	receiveBuffer[0] = '\0';
	receiveBufferSize = 0;
	fillReceiveBuffer();
}

void Packet::clearReceiveBuffer()
{
	receiveBuffer[0] = '\0';
	receiveBufferSize = 0;
	while (receiveBackBuffer.size() > 0) {
		fillReceiveBuffer();
		receiveBuffer[0] = '\0';
		receiveBufferSize = 0;
	}
}

void Packet::fillSendBuffer()
{
	while (sendBackBuffer.size() > 0 && (sendBufferSize + sendBackBuffer.front()->getSize()) < (MAX_PACKET_SIZE-4)) {
		memcpy(sendBuffer+sendBufferSize, sendBackBuffer.front()->getData(), sendBackBuffer.front()->getSize());
		sendBufferSize += sendBackBuffer.front()->getSize();
		delete sendBackBuffer.front();
		sendBackBuffer.pop_front();
	}
}

void Packet::fillReceiveBuffer()
{
	while (receiveBackBuffer.size() > 0 && (receiveBufferSize + receiveBackBuffer.front()->getSize()) < (MAX_PACKET_SIZE-4)) {
		memcpy(receiveBuffer+receiveBufferSize, receiveBackBuffer.front()->getData(), receiveBackBuffer.front()->getSize());
		receiveBufferSize += receiveBackBuffer.front()->getSize();
		delete receiveBackBuffer.front();
		receiveBackBuffer.pop_front();
	}
}