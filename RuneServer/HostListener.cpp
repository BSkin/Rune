#include "HostListener.h"

bool HostListener::running = false;

HANDLE HostListener::start() {
	if (running) return 0; //only 1 listener at a time
	running = true;
	return (HANDLE)_beginthreadex(NULL, 0, startHostListenerThread,  NULL, NULL, NULL); 
}

void HostListener::stop()
{
	running = false;
}

unsigned int HostListener::startHostListenerThread(LPVOID)
{
	WSADATA				wsaData;
    SOCKET				listenSocket, clientSocket;
	struct sockaddr_in	localAddress, clientAddress;
	int					iAddressSize;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        printf("Failed to load Winsock!\n");
        return 1;
    }

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	//hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	//hints.ai_flags = AI_NUMERICHOST;
	hints.ai_flags = AI_PASSIVE;

	char portString[6]; memset(portString, '\0', 6);
	itoa(Settings::getPortTCP(), portString, 10);
	int iResult = getaddrinfo(NULL, portString, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	//listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (listenSocket == INVALID_SOCKET) {
		printf("socket() failed: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	/*
	BOOL yes = TRUE;
	if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes)) == SOCKET_ERROR)
	{
		fprintf(stderr, "setsockopt error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(listenSocket);
		getchar();
		exit(1);
	}*/

	/*int opt = 1;
	if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) < 0) {
		fprintf(stderr, "setsockopt error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
	}*/	

	/*localAddress.sin_addr.s_addr = INADDR_ANY;
    localAddress.sin_family = AF_INET;
    localAddress.sin_port = htons(Settings::getPortTCP());

	if (bind(listenSocket, (struct sockaddr *)&localAddress, sizeof(localAddress)) < 0) {
        printf("bind() failed: %d\n", WSAGetLastError());
        return 1;
    }*/
	// Setup the TCP listening socket
	iResult = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	freeaddrinfo(result);

	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	printf("Listen Server Initialized on port %d\n", Settings::getPortTCP());

	TIMEVAL tv = { 0 };
	fd_set readSet;
	
	int returnValue;

	while (running) {
		tv.tv_sec = 0;
		tv.tv_usec = 1000000; // microseconds
        FD_ZERO(&readSet);
		FD_SET(listenSocket, &readSet);
		returnValue = select( 0, &readSet, NULL, NULL, &tv );

		if (returnValue == SOCKET_ERROR) {
			cout << "Error listening to clients\n";
			break;
		}
		else if (returnValue) {
			iAddressSize = sizeof(clientAddress);
			clientSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &iAddressSize);
			//clientSocket = accept(listenSocket, NULL, NULL);
			if (clientSocket == INVALID_SOCKET) {
				printf("accept() failed: %d\n", WSAGetLastError());
				break;
			}

			HostThread::start(clientSocket);
			printf("Accepted client: %s:%d\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
		}
		else {
			//cout << "timeOut\n";
		}
	}

	closesocket(listenSocket);
	WSACleanup();

	return 0;
}