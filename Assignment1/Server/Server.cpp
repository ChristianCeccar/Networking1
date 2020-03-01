///UDP Server
//Matthew Holt - 100622906
//Samuelle Lili Bouffard - 100582562
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <utility>
#include <string>
#include <thread>
#include <vector>

//hey, so we couldn't get this fully working, some sorta issue with our sendto here and recvfrom in the client, I have suspicions on the reason, but I'm not entirely sure

//this class stores all relevant data for the rendezvous server, from ip address to the user's status
struct Clients {
	Clients(std::string _ip, std::string _username, std::string _status)
	{
		ipAddr = _ip;
		username = _username;
		status = _status;
	}

	std::string ipAddr; //stores IP address of client
	std::string username; //stores username
	std::string status; //stores their status, Online or Busy atm
};

#pragma comment(lib, "Ws2_32.lib")
int ServerThreading(); //just a forward declaration

int main() {
	std::thread serverThread(ServerThreading);
	serverThread.join(); //used a thread to accept input from multiple clients, cause otherwise it would stop after just one
	return 0;
}

//this function handle all multithreading of the server so it can accept multiple clients
int ServerThreading() {
	//Initialize winsock
	WSADATA wsa;
	SYSTEMTIME stime; //using system time to determine what times the server received messages
	int error;
	error = WSAStartup(MAKEWORD(2, 2), &wsa);
	
	if (error != 0) {
		printf("Failed to initialize %d\n", error);
		return 1; //exiting if winsock fails to initialize
	}

	//Create a Server socket

	struct addrinfo* addrPtr = NULL, hints;

	//setting upt UDP protocols for sending/receiving messages
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, "8888", &hints, &addrPtr) != 0) {
		printf("Getaddrinfo failed!! %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	//initializing a UDP socket
	SOCKET server_socket;
	server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (server_socket == INVALID_SOCKET) {
		printf("Failed creating a socket %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	printf("Waiting for Data...\n");

	// Receive msg from client in this buffer
	const unsigned int BUF_LEN = 512;
	char recvBuf[BUF_LEN];
	char ipBuff[BUF_LEN];

	// Struct that will hold the IP address of the client that sent the message so we can send back to them
	//struct is mostly used for sendto instructions
	struct sockaddr_in fromAddr, toAddr;
	int fromlen, sendLen;
	fromlen = sizeof(fromAddr);

	sendLen = sizeof(toAddr);

	toAddr.sin_family = AF_INET;
	toAddr.sin_addr.s_addr = INADDR_ANY;
	toAddr.sin_port = htons(8888);

	if (bind(server_socket, addrPtr->ai_addr, sizeof(fromAddr)) == SOCKET_ERROR) { //we bind the server so we can recv stuff on it
		printf("Bind failed: %d\n", WSAGetLastError());
		closesocket(server_socket);
		freeaddrinfo(addrPtr);
		WSACleanup();
		return 1;
	}

	//vector stores a variable number of clients and their data
	std::vector<Clients> clientData;

	for (;;) {

		//we're grabbing local time at the beginning of the loop so we can display it near the end
		GetLocalTime(&stime);
		//memset the buffer so nothing goes wrong
		memset(recvBuf, 0, BUF_LEN);
		if (recvfrom(server_socket, recvBuf, sizeof(recvBuf), 0, (struct sockaddr*) & fromAddr, &fromlen) == SOCKET_ERROR) {
			printf("recvfrom() failed...%d\n", WSAGetLastError());
			return 1;
		}
		
		//gets ip address of client that sent message for use in most of our use cases
		std::string ip = inet_ntop(fromAddr.sin_family, &fromAddr, ipBuff, sizeof(ipBuff));

		if (recvBuf[0] == '@') {
			
			printf("%s Has joined\n", recvBuf); //prints out @username has joined
			clientData.push_back(Clients(ip, recvBuf, "Online")); //adds a client to the list
		}
		else if (recvBuf[0] == '!'){ //! is the front character for sending commands to the server, such as requesting status fo all users, or connecting directly to a user
			if (std::strstr(recvBuf, "!status")) {
				int temp = -1;
				std::string tempList = "Connected users(status):\n";
				for (int i = 0; i < clientData.size(); i++) {
					//gathers list of users and their statuses, which are set to online when joining
					tempList.append(clientData[i].username + " (" + clientData[i].status + ")\n");
					temp = i;
				}
				if (temp != -1) {
					toAddr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
					printf(tempList.c_str()); //print out list of users on server
					sendto(server_socket, tempList.c_str(), BUF_LEN, 0, addrPtr->ai_addr, sendLen); //sends messages to client
				}
			}
			
			if (std::strstr(recvBuf, "!connect ") != nullptr){
				for (int i = 0; i < clientData.size(); i++) {
					if (std::strstr(recvBuf, clientData[i].username.c_str()) != nullptr) {
						// Connect both clients to chat/game, exchange data, and change their status to busy
					}
				}
			}
		}
		else {
			int index = -1;

			for (int i = 0; i < clientData.size(); i++) {
				if (clientData[i].ipAddr == ip.c_str()) {
					//we grab the client whose IP matches the one that sent the message so we can display their username later
					index = i;
					break;
				}
			}

			if (index >= 0) {
				//if input wasn't anything special, we print out a timestamp, their username, and their message to the server
				printf("[%02d:%02d] %s: %s\n", stime.wHour, stime.wMinute, clientData[index].username.c_str(), recvBuf);
			}
		}

		//stores an IP buffer for use in the connection process
		char ipbuf[INET_ADDRSTRLEN];

	}
	closesocket(server_socket);
	freeaddrinfo(addrPtr);
	WSACleanup();
	return 0;
}