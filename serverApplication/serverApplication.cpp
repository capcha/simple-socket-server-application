#include <iostream>
#include <sstream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <list>
#include <thread>

using namespace std;

struct client {
	SOCKET socket = SOCKET_ERROR;
	SOCKADDR_IN sockADDR_IN;
	string name = "";

	client() {};
	client(SOCKET _socket, SOCKADDR_IN sADDR_IN, string _name) : socket(_socket), sockADDR_IN(sADDR_IN), name(_name) {};
};

// Необходимо, чтобы линковка происходила с DLL-библиотекой 
// Для работы с сокетам
#pragma comment(lib, "Ws2_32.lib")

const char* SERVERPORT = "2001";
const int TEXTLEN = 1024;
const int RECVTEXTLEN = 2048;
const int PORTLEN = 6;
const int IPLEN = 16;
const int MAXUSERAMOUNT = 50;
list<client> clients;


int chat(client& clientI) {

	char recvText[RECVTEXTLEN];
	int retVal;

	while (true) {
		retVal = recv(clientI.socket, recvText, RECVTEXTLEN, 0);

		if (retVal == SOCKET_ERROR) {
			cerr << "bind failed with error: " << WSAGetLastError() << "\n";
			closesocket(clientI.socket);
			WSACleanup();
			return 1;
		}
		
		string result = clientI.name;


		if (recvText == "exit") {
			result.append(" left the chat");

			closesocket(clientI.socket);

			clients.remove(clientI);

			return 1;
		}


		for (client i : clients) {
			result = clientI.name;
			result.append(": ");
			result.append(recvText);
			result.append("\n");

			retVal = send(i.socket, result.c_str(), RECVTEXTLEN, 0);

			if (retVal == SOCKET_ERROR) {
				cerr << "send failed with error: " << WSAGetLastError() << "\n";
				closesocket(i.socket);
				clients.remove(i);
				return 1;
			}
		}
	}
}

int server() {
	WSADATA wsaData;
	char ip[IPLEN];
	int userAmount = 0;
	client tempClient;

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

	retVal = getaddrinfo(INADDR_ANY, SERVERPORT, &hints, &addr);

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

		clientSocket = accept(serverSocket, (struct sockaddr*) &from, &fromlen);

		if (clientSocket == INVALID_SOCKET) {
			cerr << "accpet error at socket: " << WSAGetLastError() << "\n";
			WSACleanup();
			return 1;
		}

		inet_ntop(hints.ai_family, &from.sin_addr, ip, INET_ADDRSTRLEN);

		cout << "New connection accepted from " << ip << " port: " << htons(from.sin_port) << endl;
		cout << "Users on server: " << ++userAmount << endl;

		char clientName[RECVTEXTLEN];
		char recvText[RECVTEXTLEN];

		retVal = recv(clientSocket, clientName, TEXTLEN, 0);

		if (retVal == SOCKET_ERROR) {
			cerr << "recv failed with error: " << WSAGetLastError() << "\n";
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}

		if (userAmount <= MAXUSERAMOUNT) {

			tempClient = client(clientSocket, from, clientName);
			
			char newConnection[RECVTEXTLEN] = "New client connected:\n";
			strcat_s(newConnection, inet_ntoa(tempClient.sockADDR_IN.sin_addr));
			strcat_s(newConnection, "\nHello, ");
			strcat_s(newConnection, tempClient.name.c_str());

			for (client i : clients) {
				retVal = send(i.socket, newConnection, RECVTEXTLEN, 0); 

				if (retVal == SOCKET_ERROR) {
					cerr << "send failed with error: " << WSAGetLastError() << "\n";
					closesocket(i.socket);
					clients.remove(i);
					return 1;
				}
			}

			clients.push_back(tempClient);

		}

		else {
			--userAmount;
			cout << "Server is full" << endl;
			retVal = send(clientSocket, "Server is full", RECVTEXTLEN, 0);

			if (retVal == SOCKET_ERROR) {
				cerr << "send failed with error: " << WSAGetLastError() << "\n";
				closesocket(clientSocket);
				return 1;
			}

			closesocket(clientSocket);
		}

		thread threadI = thread(chat, tempClient.socket);
	}

	closesocket(serverSocket);

	WSACleanup();

	return 0;
}

int main() {
	cout << "SERVER STARTED" << endl;
	server();
	cout << "SERVER STOPPED" << endl;
}