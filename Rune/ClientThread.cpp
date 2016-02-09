#include "ClientThread.h"

bool ClientThread::running = false;
Client * ClientThread::client = NULL;
HANDLE ClientThread::runningMutex = CreateMutex(NULL, FALSE, NULL);

void ClientThread::cleanup()
{
	if (runningMutex != NULL) {
		CloseHandle(runningMutex);
		runningMutex = NULL;
	}
}

struct netAdr {
	unsigned long ip;
	USHORT port;
};

HANDLE ClientThread::connectToServer(unsigned long ipAddress, USHORT port)
{
	aquireRunningMutex();
		if (running) {
			releaseRunningMutex();	
			return NULL; //can only be connected to 1 host at a time
		}
		running = true;
	releaseRunningMutex(); 
	netAdr * netPointer = new netAdr(); netPointer->ip = ipAddress; netPointer->port = port;
	return (HANDLE)_beginthreadex(NULL, 0, startClientThread, (LPVOID)netPointer, NULL, NULL);
}

void ClientThread::disconnectFromServer()
{
	client->aquireMutex();
		client->setDisconnected();
	client->releaseMutex();

	aquireRunningMutex();
		running = false;
	releaseRunningMutex();
}

unsigned int ClientThread::startClientThread(LPVOID arg)
{
	#pragma region Initialize
	netAdr * netPointer = (netAdr *)arg;
	unsigned long serverIP = netPointer->ip;
	USHORT serverPortTCP = netPointer->port;
	delete arg;
	char hostName[128];

	WSADATA wsaData;
	SOCKET clientSocket = INVALID_SOCKET;
	int returnValue;

	sockaddr_in server;
	hostent * host = NULL;

	bool initError = false;

	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        printf("Failed to load Winsock library!\n");
        //return 1;
		running = false;
		initError = true;
    }

	struct in_addr tempAddr; tempAddr.s_addr = serverIP;
	string ipString = string(inet_ntoa(tempAddr));
	char portString[6]; memset(portString, '\0', 6);
	itoa(Settings::getPortTCP(), portString, 10);

	struct addrinfo * result = NULL, *ptr = NULL, hints;
	ZeroMemory(&hints, sizeof(hints));
	//hints.ai_family = AF_UNSPEC;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_NUMERICHOST;
	
	int iResult;

	// Resolve the server address and port
	iResult = getaddrinfo(ipString.c_str(), portString, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		running = false;
		initError = true;
	}
	
	printf("Attempting to connect to %s:%d\n", ipString.c_str(), serverPortTCP);

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		if (!initError) {
			// Create a SOCKET for connecting to server
			clientSocket = socket(ptr->ai_family, ptr->ai_socktype,	ptr->ai_protocol);
			if (clientSocket == INVALID_SOCKET) {
				printf("socket failed with error: %ld\n", WSAGetLastError());
				running = false;
				initError = true;
			}
			// Connect to server.
			iResult = connect(clientSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				printf("connect() failed: %d\n", WSAGetLastError());
				closesocket(clientSocket);
				clientSocket = INVALID_SOCKET;
			}
		}
	}

	freeaddrinfo(result);

	if (clientSocket == INVALID_SOCKET) {
		running = false;
		initError = true;
	}

		/*clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (clientSocket == INVALID_SOCKET) {
			printf("socket() failed: %d\n", WSAGetLastError());
			//return 1;
			running = false;
			initError = true;
		}
		memset(server.sin_zero, 0, 8);
		server.sin_family = AF_INET;
		server.sin_port = serverPort;
		server.sin_addr.s_addr = serverIP;*/
	

	/*if (!initError) {
		if (server.sin_addr.s_addr == INADDR_NONE) {
			host = gethostbyname(hostName);
			if (host == NULL) {
				printf("Unable to resolve server: %s\n", ipString.c_str());
				//return 1;
				running = false;
				initError = true;
			}
			else CopyMemory(&server.sin_addr, host->h_addr_list[0], host->h_length);
		}
	}

	if (!initError) {
		if (connect(clientSocket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
		{
			printf("connect() failed: %d\n", WSAGetLastError());
			//return 1;
			running = false;
			initError = true;
		}
	}*/

	if (!initError) {
		printf("Connected to %s:%d\n", ipString.c_str(), serverPortTCP);
	}
	#pragma endregion

	DataBuffer * receiveBuffer = NULL;
	DataBuffer * sendBuffer = NULL;
	DataBuffer * tempSendBuffer = NULL;

	if (!initError) {
		receiveBuffer = new DataBuffer(MAX_PACKET_SIZE);
		sendBuffer = new DataBuffer(MAX_PACKET_SIZE);
		tempSendBuffer = NULL;
	}

	#pragma region Initial Handshake
	string username = "";
	if (!initError) {
		//Send Username and my UDP Port
		client->aquireMutex();
			client->setName(Settings::getUsername());
			username = client->getName();
			client->setConnected(username);
		client->releaseMutex();

		string portString = std::to_string(Settings::getPortUDP());
		sendBuffer->clear();
		sendBuffer->copy(0, username.c_str(), username.size());
		sendBuffer->copy(username.size(), "\0", 1);
		sendBuffer->copy(username.size() + 1, "\n", 1);
		sendBuffer->copy(username.size() + 2, portString.c_str(), portString.size());
		sendBuffer->copy(username.size() + 2 + portString.size(), "\0", 1);
		sendBuffer->copy(username.size() + 2 + portString.size() + 1, "\n", 1);

		returnValue = send(clientSocket, sendBuffer->getData(), sendBuffer->getSize(), 0);
		if (returnValue == 0) {
			running = false;
			initError = true;
		}
		else if (returnValue == SOCKET_ERROR) {
			printf("send() failed: %d\n", WSAGetLastError());
			//return 1;
			running = false;
			initError = true;
		}
		//printf("Sent Username: %s, Port: %s\n", username.c_str(), portString.c_str());
	}

	//Receive Client ID and his UDP Port
	if (!initError) {
		receiveBuffer->clear();
		returnValue = recv(clientSocket, receiveBuffer->getData(), MAX_PACKET_SIZE, 0);
		if (returnValue == 0) {
			return 1;
			running = false;
			initError = true;
		}
		else if (returnValue == SOCKET_ERROR) {
			printf("recv() failed: %d\n", WSAGetLastError());
			//return 1;
			running = false;
			initError = true;
		}
		receiveBuffer->setSize(returnValue);
		receiveBuffer->set(returnValue, '\0');

		int clientID;
		int serverPortUDP;
		char tempData[32]; memset(tempData, '\0', 32);
		int argNum = 0; int charCount = 0;
		for (int i = 0; i < receiveBuffer->getSize(); i++) {
			if (receiveBuffer->getData()[i] == '\n') {
				if (argNum == 0) clientID = atoi(tempData);
				else if (argNum == 1) serverPortUDP = atoi(tempData);
				memset(tempData, '\0', 32);
				argNum++; charCount = 0;
			}
			else {
				tempData[charCount] = receiveBuffer->getData()[i];
				charCount++;
			}
		}

		Settings::setClientID(clientID);
		NetworkFunctions::setClientID(clientID);
		client->aquireMutex();
			client->setConnected(username);
			client->setIPAddress(serverIP);
			client->setPortUDP(serverPortUDP);
			client->setPortTCP(serverPortTCP);
			client->getPacketAddress()->clearReceiveBuffer();
			client->getPacketAddress()->clearSendBuffer();
		client->releaseMutex();
		//printf("Received Client ID: %d, Port: %d\n", clientID, serverPortUDP);
	}
	#pragma endregion
	
	while (running) {
		#pragma region Send
		client->aquireMutex();
			tempSendBuffer = client->popSendData();
		client->releaseMutex();

		int packetSize = tempSendBuffer->getSize();
		sendBuffer->clear();
		sendBuffer->copy(0, &packetSize, sizeof(int));
		sendBuffer->copy(4, tempSendBuffer->getData(), tempSendBuffer->getSize());
		delete tempSendBuffer;

		returnValue = send(clientSocket, sendBuffer->getData(), sendBuffer->getSize(), 0);
		if (returnValue == 0) break;
        else if (returnValue == SOCKET_ERROR) {
            printf("send() failed: %d\n", WSAGetLastError());
            break;
        }
		//printf("Client Snd[%dB]: ", sendBuffer->getSize());
		//printfNetworkPacket(sendBuffer->getData(), sendBuffer->getSize());
		//printf("\n");
		#pragma endregion

		#pragma region Receive
		receiveBuffer->clear();
		returnValue = recv(clientSocket, receiveBuffer->getData(), MAX_PACKET_SIZE, 0);
		if (returnValue == 0) break;
        else if (returnValue == SOCKET_ERROR) {
            printf("recv() failed: %d\n", WSAGetLastError());
            break;
        }
		receiveBuffer->setSize(returnValue);
        //receiveBuffer.set(returnValue, '\0');
		//printf("Client Rec[%dB]: ", receiveBuffer->getSize());
		//printfNetworkPacket(receiveBuffer->getData(), receiveBuffer->getSize());
		//printf("\n");

		client->aquireMutex();
			client->pushReceiveData(receiveBuffer->getData(), receiveBuffer->getSize());
		client->releaseMutex();
		#pragma endregion

		Sleep(1000.0 / 64.0);
		//Sleep(1000.0f/NETWORK_UPDATE_RATE); //Change to factor in duration of the function
	}

	if (!initError) {
		iResult = shutdown(clientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			printf("shutdown failed with error: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}
		do {
			iResult = recv(clientSocket, receiveBuffer->getData(), MAX_PACKET_SIZE, 0);
		} while (iResult > 0);
	}

	if (sendBuffer != NULL)delete sendBuffer;
	if (receiveBuffer != NULL)delete receiveBuffer;

	closesocket(clientSocket);
	printf("Closing Socket\n");
	WSACleanup();
	client->setDisconnected();
	disconnectFromServer();

	return 0;
}

bool ClientThread::isRunning()
{
	bool temp;
	aquireRunningMutex();
		temp = running;
	releaseRunningMutex();
	return temp;
}

void ClientThread::setStatics(Client * client)
{
	ClientThread::client = client;
}

int ClientThread::aquireRunningMutex()
{
	DWORD waitResult = WaitForSingleObject(runningMutex, INFINITE);
	if (waitResult != WAIT_OBJECT_0) {
		printf("Error aquiring running mutex.\n");
		return -1;
	}
	return 0;
}

int ClientThread::releaseRunningMutex()
{
	if (!ReleaseMutex(runningMutex)) {
		printf("Error releasing running mutex.\n");
		return -1;
	}
	return 0;
}