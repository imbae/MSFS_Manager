#pragma once
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "ws2_32.lib")

using namespace std;

constexpr auto MAX_BUF_SIZE = 1024;

class UdpServer
{
public:
	UdpServer();
	~UdpServer();

	static void ReceivedFromClient();
	
	SOCKET in;
	sockaddr_in client;

	int clientLength;
	char buf[MAX_BUF_SIZE];
	bool isAbort;

};

static UdpServer* currentUdpPtr;
