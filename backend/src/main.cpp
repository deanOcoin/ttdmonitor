#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "client/client.hpp"

#pragma comment(lib, "Ws2_32.lib")

int main()
{
	FreeConsole();

	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		std::cerr << "WSAStartup failed: " << result << std::endl;
		return 1;
	}

	SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_socket == INVALID_SOCKET)
	{
		std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return 1;
	}

	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &(server_addr.sin_addr));
	server_addr.sin_port = htons(9745);

	result = bind(listen_socket, (sockaddr*)&server_addr, sizeof(server_addr));
	if (result == SOCKET_ERROR)
	{
		std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
		closesocket(listen_socket);
		WSACleanup();
		return 1;
	}

	result = listen(listen_socket, SOMAXCONN);
	if (result == SOCKET_ERROR)
	{
		std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
		closesocket(listen_socket);
		WSACleanup();
		return 1;
	}

	SOCKET client_socket = accept(listen_socket, NULL, NULL);
	if (client_socket == INVALID_SOCKET)
	{
		std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
		closesocket(listen_socket);
		WSACleanup();
		return 1;
	}

	char recv_buffer[1024];
	Client* ttdclient = nullptr;
	double key_rate = 0.0;
	double percent_complete = 0.0;
	uint32_t update_delay = 500;

	while (true) {
		if (ttdclient != nullptr)
		{
			key_rate = ttdclient->vanitysearch_process->GetKeyRate();
			percent_complete = ttdclient->vanitysearch_process->GetPercentCompleted();

			if (key_rate < 0.0 || percent_complete < 0.0)
			{
				ttdclient->UpdateSubProcesses();
				ttdclient->vanitysearch_process->ChangeUpdateDelay(update_delay);
				key_rate = 0.0;
				percent_complete = 0.0;
			}
		}
		
		uint32_t bytes_received = recv(client_socket, recv_buffer, sizeof(recv_buffer) - 1, 0);
		if (bytes_received > 0)
		{
			recv_buffer[bytes_received] = '\0'; // manually add null terminator

			if (recv_buffer[0] == '?')
			{
				std::string buf = std::to_string(key_rate) + ":" + std::to_string(percent_complete);
				send(client_socket, buf.c_str(), strlen(buf.c_str()), 0);
				continue;
			}
			else if (recv_buffer[0] == '*')
			{
				uint32_t new_delay = std::atoi(recv_buffer + 1); // skip '*'
				update_delay = new_delay;
				if (ttdclient != nullptr) { ttdclient->vanitysearch_process->ChangeUpdateDelay(new_delay); }
				continue;
			}
			else
			{
				ttdclient = new Client(std::string(recv_buffer));
				if (ttdclient->GetSubProcesses().size() == 0)
				{
					delete ttdclient;
					ttdclient = nullptr;
				}
			}

		}
		else if (bytes_received == 0)
		{
			std::cout << "Connection closed" << std::endl;
			break;
		}
		else
		{
			std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
			break;
		}
	}
	
	closesocket(client_socket);
	closesocket(listen_socket);
	WSACleanup();
	delete ttdclient;
	return 0;
}
