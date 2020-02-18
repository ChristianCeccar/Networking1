///// UDP Server
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <utility>
#include <string>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")
int multiThreading();

int main() {


	std::thread multiThread(multiThreading);
	multiThread.join();
	return 0;
}

int multiThreading() {
	//Initialize winsock
	WSADATA wsa;

	int error;
	error = WSAStartup(MAKEWORD(2, 2), &wsa);

	if (error != 0) {
		printf("Failed to initialize %d\n", error);
		return 1;
	}

	//Create a Server socket

	struct addrinfo* ptr = NULL, hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, "8888", &hints, &ptr) != 0) {
		printf("Getaddrinfo failed!! %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	SOCKET server_socket;

	server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (server_socket == INVALID_SOCKET) {
		printf("Failed creating a socket %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Bind socket

	if (bind(server_socket, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {
		printf("Bind failed: %d\n", WSAGetLastError());
		closesocket(server_socket);
		freeaddrinfo(ptr);
		WSACleanup();
		return 1;
	}

	printf("Waiting for Data...\n");

	// Receive msg from client
	const unsigned int BUF_LEN = 512;

	char recv_buf[BUF_LEN];
	char ipBuff[BUF_LEN];

	// Struct that will hold the IP address of the client that sent the message (we don't have accept() anymore to learn the address)
	struct sockaddr_in fromAddr;
	char* IPAdd;
	int fromlen;
	fromlen = sizeof(fromAddr);

	if (recvfrom(server_socket, recv_buf, sizeof(recv_buf), 0, (struct sockaddr*) & fromAddr, &fromlen) == SOCKET_ERROR) {
		printf("recvfrom() failed...%d\n", WSAGetLastError());
	}

	printf("%s Has joined\n", recv_buf);

	for (;;) {
		memset(recv_buf, 0, BUF_LEN);
		if (recvfrom(server_socket, recv_buf, sizeof(recv_buf), 0, (struct sockaddr*) & fromAddr, &fromlen) == SOCKET_ERROR) {
			printf("recvfrom() failed...%d\n", WSAGetLastError());
			return 1;
		}

		std::string ip = inet_ntop(fromAddr.sin_family, &fromAddr, ipBuff, sizeof(ipBuff));

		printf("IP: %s ", ip.c_str());
		printf("Received: %s\n", recv_buf);

		char ipbuf[INET_ADDRSTRLEN];
		//		printf("Dest IP address: %s\n", inet_ntop(AF_INET, &fromAddr, ipbuf, sizeof(ipbuf)));
		//		printf("Source IP address: %s\n", inet_ntop(AF_INET, &fromAddr, ipbuf, sizeof(ipbuf)));

	}
	closesocket(server_socket);
	freeaddrinfo(ptr);
	WSACleanup();
	return 0;
}