///// UDP Server
// Hugo Suzuki - 100559448
// in collaboration with christian
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <utility>
#include <string>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

int main() 
{
	//Initialize winsock
	WSADATA wsa;
	SYSTEMTIME stime;
	int error;
	error = WSAStartup(MAKEWORD(2, 2), &wsa);

	if (error != 0) 
	{
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


	if (getaddrinfo(NULL, "8888", &hints, &ptr) != 0)
	{
		printf("Getaddrinfo failed!! %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}	

	SOCKET server_socket;

	server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (server_socket == INVALID_SOCKET) 
	{
		printf("Failed creating a socket %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Bind socket

	if (bind(server_socket, ptr->ai_addr, ptr->ai_addrlen) == SOCKET_ERROR)
	{
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

	// Struct that will hold the IP address of the client that sent the message
	struct sockaddr_in fromAddr;
	int fromlen;
	fromlen = sizeof(fromAddr);

	std::vector<sockaddr_in> sockAddrlist;
	std::vector<std::string> IPlist;
	std::vector<std::string> usernamelist;
	std::vector<std::string> statuslist;

	std::vector<std::pair<int, int>> connectionlist;

	for (;;)
	{
		GetLocalTime(&stime);
		memset(recv_buf, 0, BUF_LEN);
		if (recvfrom(server_socket, recv_buf, sizeof(recv_buf), 0, (struct sockaddr*) & fromAddr, &fromlen) == SOCKET_ERROR)
		{
			printf("recvfrom() failed...%d\n", WSAGetLastError());
			return 1;
		}
		
		std::string ip = inet_ntop(fromAddr.sin_family, &fromAddr, ipBuff, sizeof(ipBuff));

		if (recv_buf[0] == '@') 
		{			
			printf("%s Has joined\n", recv_buf);
			sockAddrlist.push_back((struct sockaddr_in) fromAddr);
			IPlist.push_back(ip.c_str());
			usernamelist.push_back(recv_buf);
			statuslist.push_back("Online");
		}
		else if (recv_buf[0] == '!')
		{
			if (std::strstr(recv_buf, "!status") != nullptr) 
			{
				std::string statusMessage = "Connected users(status):\n";

				for (int i = 0; i < IPlist.size(); i++)
				{
					statusMessage.append(usernamelist[i] + "(" + statuslist[i] + ")\n");
				}

				if (sendto(server_socket, statusMessage.c_str(), BUF_LEN, 0, (struct sockaddr*) & fromAddr, fromlen) == SOCKET_ERROR) 
				{
					printf("sendto() failed %d\n", WSAGetLastError());
					return 1;
				}				
			}
			
			if (std::strstr(recv_buf, "!connect") != nullptr)
			{
				for (int i = 0; i < usernamelist.size(); i++) {
					if (std::strstr(recv_buf, usernamelist[i].c_str()) != nullptr)
					{
						if (ip != IPlist[i])
						{
							connectionlist.push_back(std::make_pair(IPlist.size(), i));
						}
					}
				}
			}
		}
		else 
		{
			
			int index = -1;

			for (int i = 0; i < IPlist.size(); i++)
			{
				if (IPlist[i] == ip.c_str())
				{
					std::string tempMessage;
					tempMessage.push_back('(');
					tempMessage += std::to_string(stime.wHour);
					tempMessage.push_back(':');
					tempMessage += std::to_string(stime.wMinute);
					tempMessage.push_back(')');
					tempMessage.push_back(' ');
					tempMessage += usernamelist[i];
					tempMessage.push_back(':');
					tempMessage.push_back(' ');
					tempMessage += recv_buf;
					tempMessage.push_back('\n');
					
					// This for loop compares the receiving client's position in the list with the position of recorded, and sends the message if so
					for (int j = 0; j < IPlist.size(); j++)
					{
						if (!connectionlist.empty())
						{
							if (connectionlist[j].first == j)
							{
								if (sendto(server_socket, tempMessage.c_str(), BUF_LEN, 0, (struct sockaddr*) & sockAddrlist[j], fromlen) == SOCKET_ERROR)
								{
									printf("sendto(connection 2) failed %d\n", WSAGetLastError());
									return 1;
								}
								break;
							}
							else if (connectionlist[j].second == j)
							{
								if (sendto(server_socket, tempMessage.c_str(), BUF_LEN, 0, (struct sockaddr*) & sockAddrlist[j], fromlen) == SOCKET_ERROR)
								{
									printf("sendto(connection 1) failed %d\n", WSAGetLastError());
									return 1;
								}
								break;
							}
						}
					}
					printf(tempMessage.c_str());
					break;
				}
			}
		}
	}
	closesocket(server_socket);
	freeaddrinfo(ptr);
	WSACleanup();
	return 0;
}