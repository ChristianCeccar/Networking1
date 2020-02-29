///// UDP Server
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <utility>
#include <string>
#include <thread>
#include <vector>

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
	SYSTEMTIME stime;
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

	

	
	printf("Waiting for Data...\n");

	// Receive msg from client
	const unsigned int BUF_LEN = 512;
	int sendLen, recvLen;
	char recv_buf[BUF_LEN];
	char ipBuff[BUF_LEN];

	

	// Struct that will hold the IP address of the client that sent the message (we don't have accept() anymore to learn the address)
	struct sockaddr_in fromAddr, toAddr;
	int fromlen;
	fromlen = sizeof(fromAddr);

	sendLen = sizeof(toAddr);

	toAddr.sin_family = AF_INET;
	toAddr.sin_addr.s_addr = INADDR_ANY;
	toAddr.sin_port = htons(8888);

	if (bind(server_socket, ptr->ai_addr, sizeof(fromAddr)) == SOCKET_ERROR) {
		printf("Bind failed: %d\n", WSAGetLastError());
		closesocket(server_socket);
		freeaddrinfo(ptr);
		WSACleanup();
		return 1;
	}

	/*First is IP second pair is first username second status <IP, <Username, Status>> */
	std::vector<std::pair<std::string, std::pair<std::string, std::string>>> ipUsernameStatus;

	for (;;) {

		GetLocalTime(&stime);
		memset(recv_buf, 0, BUF_LEN);
		if (recvfrom(server_socket, recv_buf, sizeof(recv_buf), 0, (struct sockaddr*) & fromAddr, &fromlen) == SOCKET_ERROR) {
			printf("recvfrom() failed...%d\n", WSAGetLastError());
			return 1;
		}
		
		std::string ip = inet_ntop(fromAddr.sin_family, &fromAddr, ipBuff, sizeof(ipBuff));

		if (recv_buf[0] == '@') {
			
			printf("%s Has joined\n", recv_buf);
			ipUsernameStatus.push_back(std::make_pair<std::string, std::pair<std::string, std::string>>(ip.c_str(), std::make_pair<std::string, std::string>(recv_buf, "Online")));
		}
		else if (recv_buf[0] == '!'){
			if (std::strstr(recv_buf, "!status")) {
				int temp = -1;
				std::string tempList = "Connected users(status):\n";
				for (int i = 0; i < ipUsernameStatus.size(); i++) {
					tempList.append(ipUsernameStatus[i].second.first + "(" + ipUsernameStatus[i].second.second + ")\n");
					temp = i;
				}
				if (temp != -1) {
					std::string ipTest = "99.249.19.58";
					toAddr.sin_addr.S_un.S_addr = inet_addr(ipTest.c_str());
					printf(tempList.c_str());
					sendto(server_socket, tempList.c_str(), BUF_LEN, 0, ptr->ai_addr, sendLen);

					//sendto(send_to, tempList.c_str(), BUF_LEN, 0, (struct sockaddr*) & fromAddr, fromlen); //send message to client
				}
			}
			
			if (std::strstr(recv_buf, "!connect ") != nullptr){
				for (int i = 0; i < ipUsernameStatus.size(); i++) {
					if (std::strstr(recv_buf, ipUsernameStatus[i].second.first.c_str()) != nullptr) {
						// Connect both clients to chat, exchange data, and change their status to busy
					}
				}
			}
		}
		else {
			int index = -1;

			for (int i = 0; i < ipUsernameStatus.size(); i++) {
				if (ipUsernameStatus[i].first == ip.c_str()) {
					index = i;
					break;
				}
			}

			if (index >= 0) {

				printf("(%02d:%02d) ", stime.wHour, stime.wMinute);
				printf("%s: ", ipUsernameStatus[index].second.first.c_str());
				printf("%s \n", recv_buf);
			}
		}

		char ipbuf[INET_ADDRSTRLEN];
		//		printf("Dest IP address: %s\n", inet_ntop(AF_INET, &fromAddr, ipbuf, sizeof(ipbuf)));
		//		printf("Source IP address: %s\n", inet_ntop(AF_INET, &fromAddr, ipbuf, sizeof(ipbuf)));

	}
	closesocket(server_socket);
	freeaddrinfo(ptr);
	WSACleanup();
	return 0;
}