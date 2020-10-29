#include <iostream>
#include <sstream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <thread>

using namespace std;

struct client {
	SOCKET socket = SOCKET_ERROR;
	SOCKADDR_IN sockADDR_IN;
	string name = "";
	int id = -1;

	client() {};
	client(SOCKET _socket, SOCKADDR_IN sADDR_IN, string _name, int _id) : socket(_socket), sockADDR_IN(sADDR_IN), name(_name), id(_id) {};
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
vector<client> clients;
int userAmount;

DWORD WINAPI chat(LPVOID clientIA) {

	char recvText[RECVTEXTLEN];
	int retVal;
	int erasedId;

	client clientI = *((client*)clientIA);

	while (true) {
		retVal = recv(clientI.socket, recvText, RECVTEXTLEN, 0);

		if (retVal == SOCKET_ERROR) {
			cerr << "bind failed with error: " << WSAGetLastError() << "\n";
			closesocket(clientI.socket);

			erasedId = clientI.id;

			clients.erase(clients.begin() + erasedId);
			for (int i = 0; i < clients.size(); i++) {
				clients[i].id = i;
			}

			erasedId = 0;
			userAmount--;
			WSACleanup();
			return SOCKET_ERROR;
		}
		
		string result = clientI.name;

		if (strcmp(recvText, "exit; ") == 0) {
			result.append(" left the chat\n");

			cout << result;


			for (client i : clients) {
				if (i.name == clientI.name) {
					erasedId = i.id;
				}
			}
			
			clients.erase(clients.begin() + erasedId);

			for (int i = 0; i < clients.size(); i++) {
				clients[i].id = i;
			}

			userAmount--;
			closesocket(clientI.socket);

			for (client i : clients) {
				retVal = send(i.socket, result.c_str(), RECVTEXTLEN, 0);
			}
			
			erasedId = 0;

			return 1;
		}

		result.append(": ");
		result.append(recvText);
		result.append("\n");
		cout << result;

		for (client i : clients) {
			
			if (i.id != clientI.id) {
				retVal = send(i.socket, result.c_str(), RECVTEXTLEN, 0);
			}

			if (retVal == SOCKET_ERROR) {
				cerr << "send failed with error: " << WSAGetLastError() << "\n";
				closesocket(i.socket);

				erasedId = i.id;

				clients.erase(clients.begin() + erasedId);
				for (int i = 0; i < clients.size(); i++) {
					clients[i].id = i;
				}

				return 1;
			}
		}
	}
}

int server() {
	WSADATA wsaData;
	char ip[IPLEN];
	userAmount = 0;
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
			cerr << "accept error at socket: " << WSAGetLastError() << "\n";
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

			tempClient = client(clientSocket, from, clientName, userAmount - 1);
			inet_ntop(hints.ai_family, &tempClient.sockADDR_IN.sin_addr, ip, INET_ADDRSTRLEN);
			char newConnection[RECVTEXTLEN] = "New client connected:\n";
			strcat_s(newConnection, ip);
			strcat_s(newConnection, "\nHello, ");
			strcat_s(newConnection, tempClient.name.c_str());
			strcat_s(newConnection, "\n");

			clients.push_back(tempClient);

			for (client i : clients) {
				retVal = send(i.socket, newConnection, RECVTEXTLEN, 0); 

				if (retVal == SOCKET_ERROR) {
					cerr << "send failed with error: " << WSAGetLastError() << "\n";
					closesocket(i.socket);
					int erasedId = i.id;

					clients.erase(clients.begin() + erasedId);

					for (int i = 0; i < clients.size(); i++) {
						clients[i].id = i;
					}
					return 1;
				}
			}


		}

		else {
			--userAmount;
			cout << "Server is full\n" << endl;
			retVal = send(clientSocket, "Server is full\n", RECVTEXTLEN, 0);

			if (retVal == SOCKET_ERROR) {
				cerr << "send failed with error: " << WSAGetLastError() << "\n";
				closesocket(clientSocket);
				return 1;
			}

			closesocket(clientSocket);
		}

		DWORD threadID;
		CreateThread(NULL, NULL, chat, &tempClient, NULL, &threadID);
		//thread th(chat, rt,ef(tempClient));
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