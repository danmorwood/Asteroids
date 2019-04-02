#pragma once
#include <string>
#include <vector>
#include <winsock2.h>

class NetworkManager {
public:
	NetworkManager();
	~NetworkManager();
	int CreateConnection(const char * port);
	void SendTo(int connection, std::string data);
	std::string Recv(int connection);
private:
	std::vector<SOCKET> _clientSockets;
	std::vector<std::string> _recvBuffer;
};

extern NetworkManager network;