#include "HostThread.h"

bool HostThread::running = false;
int * HostThread::maxClients = NULL;
vector<Client*> * HostThread::clients = NULL;

HANDLE HostThread::start(SOCKET socket) 
{
	running = true;
	return (HANDLE)_beginthreadex(NULL, 0, startHostThread, (LPVOID)socket, NULL, NULL); 
}

void HostThread::stopAll()
{
	running = false;
}

unsigned int WINAPI HostThread::startHostThread(LPVOID arg)
{
	SOCKET hostSocket = (SOCKET)arg;
	
	struct sockaddr_in clientInfo;
	int clientInfoSize = sizeof(clientInfo);
	if (getpeername(hostSocket, (sockaddr*)&clientInfo, &clientInfoSize) == -1) {
		printf("getpeername() failed\n");
		return 1;
	}
	
	DataBuffer * receiveBuffer = new DataBuffer(MAX_PACKET_SIZE);
	DataBuffer * sendBuffer = new DataBuffer(MAX_PACKET_SIZE);
	DataBuffer * tempSendBuffer;
	int returnValue;
	string username;
	USHORT portUDP;
	int clientID;

	#pragma region Initial Handshake
	//Receive Username and His UDP Port
	receiveBuffer->clear();
	returnValue = recv(hostSocket, receiveBuffer->getData(), MAX_PACKET_SIZE, 0);
	if (returnValue == 0) return 1;
	else if (returnValue == SOCKET_ERROR) {
		printf("recv() failed: %d\n", WSAGetLastError());
		return 1;
	}
	receiveBuffer->setSize(returnValue);
	receiveBuffer->set(returnValue, '\0');
	char tempData[32]; memset(tempData, '\0', 32);
	int argNum = 0; int charCount = 0;
	for (int i = 0; i < receiveBuffer->getSize(); i++) {
		if (receiveBuffer->getData()[i] == '\n') {
			if (argNum == 0) username = string(tempData);
			else if (argNum == 1) portUDP = atoi(tempData);
			memset(tempData, '\0', 32);
			argNum++; charCount = 0;
		}
		else {
			tempData[charCount] = receiveBuffer->getData()[i];
			charCount++;
		}
	}
	//printf("Received Username: %s, Port: %d\n", username.c_str(), port);

	//Aquire first available client ID
	clientID = getFirstAvailableClientID(username, clientInfo.sin_addr.s_addr, clientInfo.sin_port, htons(portUDP));
	if (clientID == -1) {
		printf("Server Full. Closing thread.\n");
		
		delete receiveBuffer;
		delete sendBuffer;
		closesocket(hostSocket);
		return 0;
	}

	printf("%s connected (Client %d)\n", username.c_str(), clientID);

	//Send Client ID and My UDP Port
	string idString = std::to_string(clientID);
	string portString = std::to_string(Settings::getPortUDP());
	sendBuffer->clear(); 
	sendBuffer->copy(0, idString.c_str(), idString.size());
	sendBuffer->copy(idString.size(), "\0", 1);
	sendBuffer->copy(idString.size() + 1, "\n", 1);
	sendBuffer->copy(idString.size() + 2, portString.c_str(), portString.size());
	sendBuffer->copy(idString.size() + 2 + portString.size(), "\0", 1);
	sendBuffer->copy(idString.size() + 2 + portString.size() + 1, "\n", 1);
	returnValue = send(hostSocket, sendBuffer->getData(), sendBuffer->getSize(), 0);
	if (returnValue == 0) {
		freeClient(clientID);
		delete receiveBuffer;
		delete sendBuffer;
		closesocket(hostSocket);
		return 1;
	}
	else if (returnValue == SOCKET_ERROR) {
		//printf("send() failed: %d\n", WSAGetLastError());
		freeClient(clientID);
		delete receiveBuffer;
		delete sendBuffer;
		closesocket(hostSocket);
		return 1;
	}
	//printf("Sent Client ID: %d, Port: %s\n", clientID, portString.c_str());
	sendBuffer->clear();
	#pragma endregion

	while(running) {
		returnValue = recv(hostSocket, receiveBuffer->getData(), MAX_PACKET_SIZE, 0);
		if (returnValue == 0) break;
		else if (returnValue == SOCKET_ERROR) {
            printf("recv() failed: %d\n", WSAGetLastError());
            break;
        }
		if (returnValue == 0) {
			running = false;
			break;
		}
		receiveBuffer->setSize(returnValue);
		receiveBuffer->set(returnValue, '\0');
		//printf("Host Rec[%d]: ", clientID);
		//printfNetworkPacket(receiveBuffer->getData(), receiveBuffer->getSize());
		//printf("\n");

		aquireClientsMutex(clientID);
			(*clients)[clientID]->pushReceiveData(receiveBuffer->getData(), receiveBuffer->getSize());
			//releaseClientsMutex(clientID);
			//aquireClientsMutex(clientID);
			tempSendBuffer = (*clients)[clientID]->popSendData();
			
		releaseClientsMutex(clientID);

		int packetSize = tempSendBuffer->getSize();
		sendBuffer->clear();
		sendBuffer->copy(0, &packetSize, sizeof(int));
		sendBuffer->copy(4, tempSendBuffer->getData(), tempSendBuffer->getSize());
		delete tempSendBuffer;

		returnValue = send(hostSocket, sendBuffer->getData(), sendBuffer->getSize(), 0);
		if (returnValue == 0) break;
		else if (returnValue == SOCKET_ERROR) {
            printf("send() failed: %d\n", WSAGetLastError());
            break;
        }
		//printf("Host Snd[%d]: ", clientID);
		//printfNetworkPacket(sendBuffer->getData(), sendBuffer->getSize());
		//printf("\n");

		Sleep(1000.0/64.0);
		//Sleep(1000.0f/NETWORK_UPDATE_RATE); //Change to factor in duration of the function
	}

	returnValue = shutdown(hostSocket, SD_SEND);
	if (returnValue == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(hostSocket);
		WSACleanup();
		return 1;
	}	
	do {
		returnValue = recv(hostSocket, receiveBuffer->getData(), MAX_PACKET_SIZE, 0);
	} while (returnValue > 0);

	delete receiveBuffer;
	delete sendBuffer;

	closesocket(hostSocket);
	freeClient(clientID);
	printf("Connection to client %d lost.\n", clientID);
	return 0;
}

void HostThread::setStatics(int * maxClients, vector<Client*> * clients)
{
	HostThread::maxClients = maxClients;
	HostThread::clients = clients;
}

int HostThread::aquireClientsMutex(int index)
{
	if (index > *maxClients) {
		printf("Client Mutex Index Out of bounds.\n");
		return -1;
	}
	return (*clients)[index]->aquireMutex();
}

int HostThread::releaseClientsMutex(int index)
{
	if (index > *maxClients) {
		printf("Client Mutex Index Out of bounds.\n");
		return -1;
	}
	return (*clients)[index]->releaseMutex();
}

int HostThread::getFirstAvailableClientID(string username, unsigned long ipAddress, unsigned long portTCP, unsigned long portUDP)
{
	for (int i = 0; i < *maxClients; i++) {
		aquireClientsMutex(i);
		if (!clients->at(i)->isConnected()) {
			clients->at(i)->setConnected(username);
			clients->at(i)->setIPAddress(ipAddress);
			clients->at(i)->setPortTCP(portTCP);
			clients->at(i)->setPortUDP(portUDP);
			clients->at(i)->setSynch(true);
			clients->at(i)->getPacketAddress()->clearReceiveBuffer();
			clients->at(i)->getPacketAddress()->clearSendBuffer();
			releaseClientsMutex(i);
			return i;
		}
		releaseClientsMutex(i);
	}
	return -1; //Server is Full
}

void HostThread::freeClient(int index)
{
	if (index >= *maxClients) return;

	aquireClientsMutex(index);
	(*clients)[index]->setDisconnected();
	releaseClientsMutex(index);
}