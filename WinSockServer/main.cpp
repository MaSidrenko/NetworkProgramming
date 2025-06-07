#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include<Windows.h>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iphlpapi.h>
#include<iostream>
#include<thread>
#pragma comment(lib, "WS2_32.lib")

#define DEFAULT_PORT				"27015"
#define DEFAULT_BUFFER_LENGTH		1500
#define SZ_SORRY					"Sorry,but all is busy"

VOID HandleClient(SOCKET ClientSocket);

void main()
{
	setlocale(LC_ALL, "");
	std::cout << "==============" << std::endl;
	std::cout << "Server" << std::endl;
	std::cout << "==============" << std::endl;
	//1. Инициализация WinSock
	WSAData wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (iResult != 0)
	{
		std::cout << "Error: WSAstartup failed: " << iResult << std::endl;
		return;
	}


	//2. Проверям, не занят ли порт, на котором мы хотим запустить свой сервер:
	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; //TCP/IPv4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	addrinfo* result = NULL;

	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		WSACleanup();
		std::cout << "Error: getaddrinfo failed: " << iResult << std::endl;
		return;
	}
	std::cout << hints.ai_addr << std::endl;

	//3. Создаем Сокет, который будет прослушивать Сервер:
	SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET)
	{
		std::cout << "Error: Socket creation failed: " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		return;
	}
	//4. Связываем сокет с сетевой картой, которую он будет прослушивать:
	iResult = bind(ListenSocket, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		std::cout << "Error: bind failed with code " << WSAGetLastError() << std::endl;
		closesocket(ListenSocket);
		freeaddrinfo(result);
		WSACleanup();
		return;
	}
	//5. Запускаем Сокет:
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		std::cout << "Error: Listen failed with code " << WSAGetLastError() << std::endl;
		closesocket(ListenSocket);
		freeaddrinfo(result);
		WSACleanup();
		return;
	}
	CONST INT MAX_CLIENST = 3;
	SOCKET clients[MAX_CLIENST] = {};
	DWORD dwThreadIDs[MAX_CLIENST] = {};
	HANDLE hThreads[MAX_CLIENST] = {};
	INT i = 0;

	while (true)
	{

		SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
		if (i < MAX_CLIENST)
		{
			//HandleClient(ClientSocket);
			clients[i] = ClientSocket;
			hThreads[i] = CreateThread
			(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)HandleClient,
				(LPVOID)clients[i],
				0,
				&dwThreadIDs[i]
			);
			i++;
		}
		else
		{
			CHAR receive_buffer[DEFAULT_BUFFER_LENGTH] = {};
			INT iResult = recv(ClientSocket, receive_buffer, DEFAULT_BUFFER_LENGTH, 0);
			if (iResult > 0)
			{
				std::cout << "Bytes received: " << iResult << std::endl;
				std::cout << "Message: " << receive_buffer << std::endl;
				//CHAR sz_sorry[] = "Sorry, but all is busy";
				INT iSendResult = send(ClientSocket, SZ_SORRY, strlen(SZ_SORRY), 0);
				closesocket(ClientSocket);
			}
		}
	}

	WaitForMultipleObjects(MAX_CLIENST, hThreads, TRUE, INFINITE);

	closesocket(ListenSocket);
	freeaddrinfo(result);
	WSACleanup();
}

VOID HandleClient(SOCKET ClientSocket)
{

	INT iResult = 0;
	//6. Зациклимаем Сокет на получение соединений от клиентов:
	CHAR recvbuffer[DEFAULT_BUFFER_LENGTH] = {};
	int recv_buffer_length = DEFAULT_BUFFER_LENGTH;
	do
	{
		ZeroMemory(recvbuffer, sizeof(recvbuffer));
		iResult = recv(ClientSocket, recvbuffer, recv_buffer_length, 0);
		if (iResult > 0)
		{
			std::cout << "Bytes received: " << iResult << std::endl;
			CHAR sz_responce[] = "Hello I am Server! Nice to meet you";
			std::cout << "Message: " << recvbuffer << std::endl;
			INT iSendResult = send(ClientSocket, recvbuffer, strlen(recvbuffer), 0);
			if (iSendResult == SOCKET_ERROR)
			{
				std::cout << "Error: Send failed with code: " << WSAGetLastError() << std::endl;
				closesocket(ClientSocket);
			}

			std::cout << "Bytes sent: " << iSendResult << std::endl;
		}
		else if (iResult == 0)
		{
			std::cout << "Connetion closed" << std::endl;
		}
		else
		{
			std::cout << "Error: recv() failed with code " << WSAGetLastError() << std::endl;
		}
	} while (iResult > 0);

}