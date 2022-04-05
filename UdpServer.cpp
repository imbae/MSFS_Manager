#include "udpServer.h"

UdpServer::UdpServer()
{
	currentUdpPtr = this;
	isAbort = false;
}
UdpServer::~UdpServer()
{

}

void UdpServer::ReceivedFromClient()
{
	//Startup Winsock
	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	int wsok = WSAStartup(version, &data);
	if (wsok != 0) {
		cout << "Can't start Winsock!" << wsok;
	}

	// Bind socket to ip address and port
	currentUdpPtr->in = socket(AF_INET, SOCK_DGRAM, 0);
	SOCKADDR_IN serverHint{};
	serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
	serverHint.sin_family = AF_INET;
	serverHint.sin_port = htons(7755);

	if (bind(currentUdpPtr->in, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR) {
		cout << "Can't bind socket!" << WSAGetLastError() << endl;
		return;
	}

	int clientLength = sizeof(currentUdpPtr->client);
	ZeroMemory(&currentUdpPtr->client, sizeof(currentUdpPtr->client));

	// Enter a loop
	while (!currentUdpPtr->isAbort)
	{
		ZeroMemory(currentUdpPtr->buf, MAX_BUF_SIZE);

		// Wait for message
		int byesIn = recvfrom(currentUdpPtr->in, currentUdpPtr->buf, MAX_BUF_SIZE, 0, (sockaddr*)&currentUdpPtr->client, &clientLength);
		if (byesIn == SOCKET_ERROR) {
			cout << "Error receiving from client " << WSAGetLastError() << endl;
			continue;
		}

		// Display message and client info
		char clientIP[256];
		ZeroMemory(clientIP, 256);

		inet_ntop(AF_INET, &currentUdpPtr->client.sin_addr, clientIP, 256);
		cout << "Message rect from " << clientIP << " :" << currentUdpPtr->buf << endl;

		//Send to client
		int sendSize = sendto(currentUdpPtr->in, reinterpret_cast<char*>(currentUdpPtr->buf),
			byesIn, 0, (struct sockaddr*)&currentUdpPtr->client, sizeof(currentUdpPtr->client));

		if (sendSize < 0)
		{
			cout << "Failed to send " << " :" << currentUdpPtr->buf << endl;
		}
	}

	// close socket
	closesocket(currentUdpPtr->in);

	// Shutdown winsock
	WSACleanup();
}