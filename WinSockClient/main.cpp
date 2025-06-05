#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN


#include<Windows.h>
#include<iphlpapi.h>
#include<iostream>
#include<WinSock2.h>
#include<WS2tcpip.h>

#define DEFAULT_PORT			"27015"
#define DEFAULT_BUFFER_LENGTH	1500

#pragma comment(lib, "Ws2_32.lib")

void main()
{
	setlocale(LC_ALL, "");
	std::cout << "==============" << std::endl;
	std::cout << "Client" << std::endl;
	std::cout << "==============" << std::endl;

	//1. Инициализируем WinSock:"
	WSAData wsaData;
	INT iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);


	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC();
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//2. Выполняем разрешение имен:
	addrinfo* result = NULL;
	iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		std::cout << "getaddrinfo() failed with code: " << iResult << std::endl;
		WSACleanup();
		return;
	}
	//3. Создаем Сокет для подключения к серверу:
	SOCKET connect_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (connect_socket == INVALID_SOCKET)
	{
		std::cout << "Socket creation failed with code: " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	//4. Подключаемся к серверу:
	iResult = connect(connect_socket, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		std::cout << "Unable to connect to Server" << std::endl;
		closesocket(connect_socket);
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	//5. Получение и отправка данных:
	CHAR send_buffer[DEFAULT_BUFFER_LENGTH] = "Hello Server, I am Client";
	CHAR recvbuffer[DEFAULT_BUFFER_LENGTH]{};

	do
	{
		iResult = send(connect_socket, send_buffer, strlen(send_buffer), 0);
		if (iResult == SOCKET_ERROR)
		{
			std::cout << "Send data failed with " << WSAGetLastError() << std::endl;
			closesocket(connect_socket);
			freeaddrinfo(result);
			WSACleanup();
			return;
		}
		std::cout << iResult << " Bytes sent" << std::endl;

		

		//6. Receivie data:
		iResult = recv(connect_socket, recvbuffer, DEFAULT_BUFFER_LENGTH, 0);
		if (iResult > 0)
		{
			std::cout << "Bytes received: " << iResult << ", Message: " << recvbuffer << std::endl;
		}
		else if (iResult == 0)
		{
			std::cout << "Connection closed" << std::endl;
		}
		else
		{
			std::cout << "Receive failed with code " << WSAGetLastError() << std::endl;
		}
		std::cout << "Введите сообщение: ";
		ZeroMemory(send_buffer, sizeof(send_buffer));
		ZeroMemory(recvbuffer, sizeof(recvbuffer));
		SetConsoleCP(1251);
		std::cin.getline(send_buffer, DEFAULT_BUFFER_LENGTH);
		SetConsoleCP(866);
		//for (int i = 0; send_buffer[i]; i++)send_buffer[i] = tolower(send_buffer[i]);
	} while (iResult > 0 && strcmp(send_buffer, "exit"));

	//7. Disconnect:
	iResult = shutdown(connect_socket, SD_SEND);
	closesocket(connect_socket);
	freeaddrinfo(result);
	WSACleanup();
}