#include <iostream>
#include <sstream>
#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace std;

// Необходимо, чтобы линковка происходила с DLL-библиотекой 
// Для работы с сокетам
#pragma comment(lib, "Ws2_32.lib")

const char* SERVERIP = "127.0.0.1";
const char* SERVERPORT = "2001";
const int TEXTLEN = 1024;
const int RECVTEXTLEN = 2048;
const int PORTLEN = 6;
const int IPLEN = 16;

int server() {
	WSADATA wsaData;
	char ip[IPLEN], port[PORTLEN];

	int retVal = WSAStartup(MAKEWORD(2, 2), &wsaData);

	struct addrinfo* addr = NULL;

	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));

	// AF_INET определяет, что используется сеть для работы с сокетом
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM; // Задаем потоковый тип сокета
	hints.ai_protocol = IPPROTO_TCP; // Используем протокол TCP
	// Сокет биндится на адрес, чтобы принимать входящие соединения
	hints.ai_flags = AI_PASSIVE;

	// Инициализируем структуру, хранящую адрес сокета - addr.

	retVal = getaddrinfo(SERVERIP, SERVERPORT, &hints, &addr);

	// Если инициализация структуры адреса завершилась с ошибкой,
	// выведем сообщением об этом и завершим выполнение программы 
	if (retVal != 0) {
		cerr << "getaddrinfo failed: " << retVal << "\n";
		WSACleanup(); // выгрузка библиотеки Ws2_32.dll
		return 1;
	}

	int serverSocket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

	if (serverSocket == INVALID_SOCKET) {
		cerr << "Error at socket: " << WSAGetLastError() << "\n";
		freeaddrinfo(addr);
		WSACleanup();
		return 1;
	}

	cout << "SERVER SOCKET CREATED" << endl;

	retVal = bind(serverSocket, addr->ai_addr, (int)addr->ai_addrlen);

	if (retVal == SOCKET_ERROR) {
		cerr << "bind failed with error: " << WSAGetLastError() << "\n";
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	cout << "SOCKET BINDED" << endl;

	while (true) {
		retVal = listen(serverSocket, 10);

		if (retVal == SOCKET_ERROR) {
			cerr << "listen failed with error: " << WSAGetLastError() << "\n";
			closesocket(serverSocket);
			WSACleanup();
			return 1;
		}

		SOCKET clientSocket;

		SOCKADDR_IN from;

		int fromlen = sizeof(from);

		clientSocket = accept(serverSocket, (struct sockaddr*) & from, &fromlen);

		if (clientSocket == INVALID_SOCKET) {
			cerr << "accpet error at socket: " << WSAGetLastError() << "\n";
			WSACleanup();
			return 1;
		}
		cout << "ACCEPTED";

		char recvText[RECVTEXTLEN];

		retVal = recv(clientSocket, recvText, TEXTLEN, 0);

		if (retVal == SOCKET_ERROR) {
			cerr << "recv failed with error: " << WSAGetLastError() << "\n";
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}

		cout << "RECIEVED TEXT: " << recvText << endl;

		if (recvText == "stop") {
			cout << "STOPPED";
			break;
		}

		strcat_s(recvText, to_string(strlen(recvText)).c_str());

		retVal = send(clientSocket, recvText, RECVTEXTLEN, 0);

		if (retVal == SOCKET_ERROR) {
			cerr << "send failed with error: " << WSAGetLastError() << "\n";
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}

		cout << "MESSAGE IS SENT" << endl;

		closesocket(clientSocket);

		cout << "CLIENT SOCKET IS CLOSED" << endl;
	}

	closesocket(serverSocket);

	WSACleanup();

	return 0;
}

int main() {
	cout << "SERVER STARTED" << endl;
	server();
}