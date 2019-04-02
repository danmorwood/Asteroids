#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "Network.h"



NetworkManager network;

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512

NetworkManager::NetworkManager()
{
	WSADATA wsaData;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
	}

}
NetworkManager::~NetworkManager() {
	// shutdown the connection since we're done
	for (SOCKET socket : _clientSockets) {
		shutdown(socket, SD_SEND);
		closesocket(socket);
	}
	WSACleanup();
}
int NetworkManager::CreateConnection(const char* port) {
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iSendResult;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, port, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
	}

	// No longer need server socket
	closesocket(ListenSocket);
	_clientSockets.push_back(ClientSocket);
	_recvBuffer.push_back("");
	return _clientSockets.size() - 1;
	
}
void NetworkManager::SendTo(int connection, std::string data) {
	int sent = 0;
	while (sent != data.length()) {
		int ret = send(_clientSockets[connection], data.c_str() + sent, data.length(), 0);
		if (ret == 0)
			printf("Nothing sent I think something might be wrong...\n");
			return;
		sent += ret;
	}
}
std::string NetworkManager::Recv(int connection) {
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	bool messageFound = false;
	int count = 0;
	std::string output = "";
	while (!messageFound) {
		while (_recvBuffer[connection].length() == 0) {
			int size = recv(_clientSockets[connection], recvbuf, recvbuflen-1, 0);
			if (size == 0) return output;
			recvbuf[size] = 0;
			_recvBuffer[connection] += recvbuf;
		}
		char c = _recvBuffer[connection][0];
		_recvBuffer[connection].erase(0, 1);
		output += c;
		if (c == '{') {
			count++;
		}
		if (c == '}' && count > 0) {
			count--;
			if (count == 0)
				messageFound = true;
		}
	}
	return output;
}