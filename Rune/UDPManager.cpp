#include "UDPManager.h"

SOCKET UDPManager::sock = SOCKET();
struct sockaddr_in UDPManager::myAddress;
bool UDPManager::initErrors = false;

//int UDPManager::netID = -1;

UDPManager::UDPManager()
{
	
}

UDPManager::~UDPManager()
{
	
}

int UDPManager::init(USHORT port)
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed Initializing Winsock : %d\n", WSAGetLastError());
		initErrors = true;
		return -1;
	}

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;

	char portString[6]; memset(portString, '\0', 6);
	itoa(Settings::getPortUDP(), portString, 10);
	int iResult = getaddrinfo(NULL, portString, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (sock == INVALID_SOCKET) {
		printf("socket() failed: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	/*if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		printf("Cannot create UDP socket.\n");
		initErrors = true;
		return -1;
	}*/

	//myAddress.sin_family = AF_INET;
	//myAddress.sin_addr.s_addr = INADDR_ANY;
	//myAddress.sin_port = htons(Settings::getPortUDP());
	//if (isServer)	myAddress.sin_port = DEFAULT_PORT_UDP_CLIENT;
	//else			myAddress.sin_port = DEFAULT_PORT_UDP_HOST;

	/*if (bind(sock, (const struct sockaddr *)&myAddress, sizeof(myAddress)) < 0) {
		printf("UDP Socket Bind failed : %d\n", WSAGetLastError());
		closesocket(sock);
		initErrors = true;
		return -1;
	}*/
	iResult = bind(sock, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(sock);
		initErrors = true;
		return -1;
	}
	freeaddrinfo(result);

	printf("Receiving UDP Packets over Port %d\n", (int)port);

	return 0;
}

int UDPManager::cleanup()
{
	if (initErrors) return -1;
	
	closesocket(sock);
	return 0;
}

int UDPManager::sendDatagram(DataBuffer * data, unsigned long ipAddress, unsigned long port, unsigned long frameCount) {
	if (initErrors || data == NULL || port == 0) return -1;

	sockaddr_in otherAddress;
	otherAddress.sin_family = AF_INET;
	otherAddress.sin_addr.s_addr = ipAddress;
	otherAddress.sin_port = port;

	int packetSize = data->getSize();
	DataBuffer * sendBuffer = new DataBuffer(packetSize + 4);
	//sendBuffer->copy(0, &packetSize, sizeof(int));
	sendBuffer->copy(0, &frameCount, 4);
	sendBuffer->copy(4, data->getData(), data->getSize());

	if (sendto(sock, sendBuffer->getData(), sendBuffer->getSize(), 0, (struct sockaddr *)&otherAddress, sizeof(otherAddress)) < 0) {
		perror("UDP sendto() failed\n");
		delete sendBuffer;
		return -1;
	}

	//printf("UDP Send:");
	//Utilities::printfNetworkPacket(sendBuffer->getData(), sendBuffer->getSize());
	delete sendBuffer;

	return 0;
}

int UDPManager::sendDatagram(const char * data, int length, unsigned long ipAddress, unsigned long port, unsigned long frameCount)
{
	if (initErrors || data == NULL || length == 0 || port == 0) return -1;

	sockaddr_in otherAddress;
	otherAddress.sin_family = AF_INET;
	otherAddress.sin_addr.s_addr = ipAddress;
	otherAddress.sin_port = port;

	DataBuffer * sendBuffer = new DataBuffer(length + 4);
	//sendBuffer->copy(0, &length, sizeof(int));
	sendBuffer->copy(0, &frameCount, 4);
	sendBuffer->copy(4, data, length);
	
	if (sendto(sock, sendBuffer->getData(), sendBuffer->getSize(), 0, (struct sockaddr *)&otherAddress, sizeof(otherAddress)) < 0) {
		perror("UDP sendto() failed\n");
		delete sendBuffer;
		return -1;
	}

	//printf("UDP Send:");
	//Utilities::printfNetworkPacket(sendBuffer->getData(), sendBuffer->getSize());
	delete sendBuffer;

	return 0;
}

DataBuffer * UDPManager::recvDatagram(sockaddr_in ** originAddress)
{
	if (initErrors) return NULL;

	struct timeval timeout;
	struct fd_set fds;

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(sock, &fds);
	
	int selectValue = select(0, &fds, 0, 0, &timeout);

	if (selectValue > 0) {
		DataBuffer * receiveBuffer = new DataBuffer(MAX_PACKET_SIZE);
		sockaddr_in * recvAddress = new sockaddr_in();
		int recvBufferSize = MAX_PACKET_SIZE;
		int returnValue = recvfrom(sock, receiveBuffer->getData(), MAX_PACKET_SIZE, 0, (struct sockaddr *)recvAddress, &recvBufferSize);

		if (returnValue == SOCKET_ERROR) {
			printf("recvfrom() failed: %d\n", WSAGetLastError());
			delete receiveBuffer;
			return NULL;
		}
		receiveBuffer->setSize(returnValue);

		if (originAddress == NULL) delete recvAddress;
		else *originAddress = recvAddress;

		//printf("UDP Rec:");
		//Utilities::printfNetworkPacket(receiveBuffer->getData(), receiveBuffer->getSize());
		
		//check that data was sent from client?
		
		return receiveBuffer;
	}
	else if (selectValue < 0) printf("Error polling for UDP Data\n");

	return NULL;	
}