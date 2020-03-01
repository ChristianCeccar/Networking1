///UDP Client
//Matthew Holt - 100622906
//Samuelle Lili Bouffard - 100582562
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

int serverSendRecv();

int main() {
	std::thread serverComm(serverSendRecv);
	//initial plan was to have a thread to communicate to the server for joining, and another thread for DMs with another client. 
	//In theory, it was going to work like a P2P w/ rendezvous for 1 on 1 chats. And it was gonna be in a loop, and if you left a chat, then it would send you back to the server
	//it was gonna be great
	serverComm.join(); 
	return 0;
}

int serverSendRecv() {
	//Initialize winsock
	WSADATA wsa;

	int error;
	error = WSAStartup(MAKEWORD(2, 2), &wsa);

	if (error != 0) {
		printf("Failed to initialize %d\n", error);
		return 1;
	}


	//Create a client socket

	struct addrinfo* addrPtr = NULL, hints;

	//we set up our protocols to be UDP
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	std::string ipAddr;
	printf("Please enter IP Address: \n");
	std::getline(std::cin, ipAddr); //prompt the user to enter the IP address of the server

	if (getaddrinfo(ipAddr.c_str(), "8888", &hints, &addrPtr) != 0) {
		printf("Getaddrinfo failed!! %d\n", WSAGetLastError());
		WSACleanup();
		return 1; //if that address doesn't work, exit the function
	}

	

	//create a UDP socket
	SOCKET cli_sock;
	cli_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (cli_sock == INVALID_SOCKET) {
		printf("Failed creating a socket %d\n", WSAGetLastError());
		WSACleanup();
		return 1; //if socket creation fails, exit the function
	}
	else {
		printf("Connected\n");
	}


	const unsigned int BUF_LEN = 512;

	//separate buffers for sending and receiving data from the server
	//send_buf is just used for sending the user's alias/username to the
	//server for human-readable identification later
	char sendBuf[BUF_LEN];
	char recvBuf[BUF_LEN];

	memset(sendBuf, 0, BUF_LEN);

	printf("\nEnter username: \n");
	std::string userName;
	std::getline(std::cin, userName); //prompts user for username

	userName = "@" + userName; //appends an @ so the server can differentiate join messages from other messages

	std::strcat(sendBuf, userName.c_str());
	if (sendto(cli_sock, sendBuf, BUF_LEN, 0,
		addrPtr->ai_addr, addrPtr->ai_addrlen) == SOCKET_ERROR) {
		printf("sendto() failed %d\n", WSAGetLastError());
		return 1;
	}

	//we use this for receiving messages
	struct sockaddr_in fromAddr;
	int fromlen;
	fromlen = sizeof(fromAddr);

	//we assign it the values from out input earlier
	memset((char*) &fromAddr, 0, sizeof(fromAddr));
	fromAddr.sin_family = AF_INET;
	fromAddr.sin_port = htons(8888);
	fromAddr.sin_addr.S_un.S_addr = inet_addr(ipAddr.c_str());

	//we set it up to be an unblocked socket, since recvfrom is a blocking function otherwise
	//we do this here because sendto gets blocked by our std::getline anyways
	u_long mode = 1;// 0 for blocking mode
	ioctlsocket(cli_sock, FIONBIO, &mode);

	for (;;) {
		printf("Enter message: ");
		std::string line;
		std::getline(std::cin, line);
		line += "\n";
		char* message = (char*)line.c_str();

		// send msg to server

		if (sendto(cli_sock, message, BUF_LEN, 0,
			addrPtr->ai_addr, addrPtr->ai_addrlen) == SOCKET_ERROR) {
			printf("sendto() failed %d\n", WSAGetLastError());
			return 1;
		}
		delete[] message; //just freeing up some memory

		printf("Message sent...\n"); //prints when a messages has been successfully send w/o socket erros


		memset(recvBuf, 0, BUF_LEN);
		recvfrom(cli_sock, recvBuf, BUF_LEN, 0, (sockaddr*) &fromAddr, &fromlen); //receives messages from the server
		printf(recvBuf); //prints those messages to the console window
	}


	//Shutdown the socket

	if (shutdown(cli_sock, SD_BOTH) == SOCKET_ERROR) {
		printf("Shutdown failed!  %d\n", WSAGetLastError());
		closesocket(cli_sock);
		WSACleanup();
		return 1;
	}

	closesocket(cli_sock); //close off all our sockets (just the one)
	freeaddrinfo(addrPtr); //free out addr pointer values
	WSACleanup();

	return 0;
}