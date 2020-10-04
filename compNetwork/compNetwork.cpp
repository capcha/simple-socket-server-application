#include <iostream>
#include <sstream>
#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace std;

// Необходимо, чтобы линковка происходила с DLL-библиотекой 
// Для работы с сокетам
#pragma comment(lib, "Ws2_32.lib")

const char* SERVERIP = "127.0.0.1";
const char* SERVERPORT = "8001";
const int TEXTLEN = 1024;
const int RECVTEXTLEN = 2048;
const int PORTLEN = 6;
const int IPLEN = 16;

int server() {
	WSADATA wsaData;
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
	// HTTP-сервер будет висеть на 8000-м порту локалхоста
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

	SOCKADDR_IN sin;

	sin.sin_family = PF_INET;

	sin.sin_port = htons(stoi(SERVERPORT));

	sin.sin_addr.s_addr = INADDR_ANY;

	retVal = bind(serverSocket, (LPSOCKADDR)&sin, sizeof(sin));

	if (retVal == SOCKET_ERROR) {
		cerr << "bind failed with error: " << WSAGetLastError() << "\n";
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

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

		char recvText[RECVTEXTLEN];

		retVal = recv(clientSocket, recvText, TEXTLEN, 0);

		if (retVal == SOCKET_ERROR) {
			cerr << "recv failed with error: " << WSAGetLastError() << "\n";
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}

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

		closesocket(clientSocket);
	}

	closesocket(serverSocket);

	WSACleanup();

	return 0;
}

int client() {
	WSADATA wsaData;
	char ip[IPLEN], port[PORTLEN], text[TEXTLEN];

	int retVal = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (retVal != 0) {
		cerr << "WSAStartup failed: " << retVal << "\n";
		return retVal;
	}

	SOCKET clientSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (clientSocket == INVALID_SOCKET) {
		cerr << "Error at socket: " << WSAGetLastError() << "\n";
		WSACleanup();
		return 1;
	}

	cin >> ip >> port;

	SOCKADDR_IN serverInfo;

	serverInfo.sin_family = PF_INET;

	serverInfo.sin_addr.S_un.S_addr = INADDR_ANY;

	serverInfo.sin_port = htons(stoi(port));

	retVal = connect(clientSocket, (LPSOCKADDR)&serverInfo, sizeof(serverInfo));

	if (retVal == SOCKET_ERROR) {
		cerr << "connect failed with error: " << WSAGetLastError() << "\n";
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}

	cin >> text;

	retVal = send(clientSocket, text, strlen(text), 0);

	if (retVal == SOCKET_ERROR) {
		cerr << "send failed with error: " << WSAGetLastError() << "\n";
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}

	char charRecvText[RECVTEXTLEN];

	retVal = recv(clientSocket, charRecvText, RECVTEXTLEN, 0);

	if (retVal == SOCKET_ERROR) {
		cerr << "recv failed with error: " << WSAGetLastError() << "\n";
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}

	if (charRecvText[0] != 0x00) {

		cout << charRecvText;

	}

	closesocket(clientSocket);
	WSACleanup();
	system("pause");
	return 0;
}


int main() {
	return 0;
}