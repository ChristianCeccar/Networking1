// UDP Client
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

int main() {

	//Initialize winsock
	WSADATA wsa;

	int error;
	error = WSAStartup(MAKEWORD(2, 2), &wsa);

	if (error != 0) {
		printf("Failed to initialize %d\n", error);
		return 1;
	}


	//Create a client socket

	struct addrinfo* ptr = NULL, hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	std::string ipAdd;
	printf("Please enter IP Address: \n");
	std::getline(std::cin, ipAdd);

	if (getaddrinfo(ipAdd.c_str(), "8888", &hints, &ptr) != 0) {
		printf("Getaddrinfo failed!! %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
		

	SOCKET cli_socket;

	cli_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (cli_socket == INVALID_SOCKET) {
		printf("Failed creating a socket %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	else {
		printf("Connected\n");
	}

	const unsigned int BUF_LEN = 512;

	char send_buf[BUF_LEN];
	memset(send_buf, 0, BUF_LEN);

	printf("\nEnter username: \n");
	std::string userName;
	std::getline(std::cin, userName);

	userName = "@" + userName;

	if (sendto(cli_socket, (char*) userName.c_str(), BUF_LEN, 0,
		ptr->ai_addr, ptr->ai_addrlen) == SOCKET_ERROR) {
		printf("sendto() failed %d\n", WSAGetLastError());
		return 1;
	}

	for (;;) {
		printf("Enter message: ");
		std::string line;
		std::getline(std::cin, line);
		line += "\n";
		char* message = (char*)line.c_str();

		// send msg to server

		if (sendto(cli_socket, message, BUF_LEN, 0,
			ptr->ai_addr, ptr->ai_addrlen) == SOCKET_ERROR) {
			printf("sendto() failed %d\n", WSAGetLastError());
			return 1;
		}

		printf("Message sent...\n");


	}


	//Shutdown the socket

	if (shutdown(cli_socket, SD_BOTH) == SOCKET_ERROR) {
		printf("Shutdown failed!  %d\n", WSAGetLastError());
		closesocket(cli_socket);
		WSACleanup();
		return 1;
	}

	closesocket(cli_socket);
	freeaddrinfo(ptr);
	WSACleanup();

	return 0;
}