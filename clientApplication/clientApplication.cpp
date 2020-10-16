#include <iostream>
#include <sstream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>

using namespace std;

// Необходимо, чтобы линковка происходила с DLL-библиотекой 
// Для работы с сокетам
#pragma comment(lib, "Ws2_32.lib")

const int TEXTLEN = 1024;
const int RECVTEXTLEN = 2048;
const int PORTLEN = 6;
const int IPLEN = 16;

int sendClient(SOCKET& clientSocket) {

}

int recvClient(SOCKET& clientSocket) {

}

int client() {
	WSADATA wsaData;
	char ip[IPLEN], port[PORTLEN], text[TEXTLEN] = "", buf[TEXTLEN];

	int retVal = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (retVal != 0) {
		cerr << "WSAStartup failed: " << retVal << "\n";
		return retVal;
	}

	struct addrinfo* addr = NULL;

	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));

	// AF_INET определяет, что используется сеть для работы с сокетом
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM; // Задаем потоковый тип сокета
	hints.ai_protocol = IPPROTO_TCP; // Используем протокол TCP
	// Сокет биндится на адрес, чтобы принимать входящие соединения
	hints.ai_flags = AI_PASSIVE;

	// Если инициализация структуры адреса завершилась с ошибкой,
	// выведем сообщением об этом и завершим выполнение программы 
	if (retVal != 0) {
		cerr << "getaddrinfo failed: " << retVal << "\n";
		WSACleanup(); // выгрузка библиотеки Ws2_32.dll
		return 1;
	}

	SOCKET clientSocket = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);

	if (clientSocket == INVALID_SOCKET) {
		cerr << "Error at socket: " << WSAGetLastError() << "\n";
		WSACleanup();
		return 1;
	}

	// Инициализируем структуру, хранящую адрес сокета - addr.
	cin >> ip >> port;

	retVal = getaddrinfo(ip, port, &hints, &addr);

	retVal = connect(clientSocket, addr->ai_addr, (int)addr->ai_addrlen);

	if (retVal == SOCKET_ERROR) {
		cerr << "connect failed with error: " << WSAGetLastError() << "\n";
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}

	string name;

	cout << "Enter your name: " << endl;
	cin >> name;

	retVal = send(clientSocket, name.c_str(), name.length() + 1, 0);

	if (retVal == SOCKET_ERROR) {
		cerr << "send failed with error: " << WSAGetLastError() << "\n";
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}

	while (buf[strlen(buf) - 1] != ';') {
		cin >> buf;
		strcat_s(text, buf);
		strcat_s(text, " ");
	}

	if (strlen(buf) == 0) {
		cerr << "Input reader error" << "\n";
		return 1;
	}

	retVal = send(clientSocket, text, strlen(text) + 1, 0);

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

		cout << charRecvText << endl;

	}

	while (true) {
		thread threadI = thread(sendClient, recvClient, clientSocket);
	}

	closesocket(clientSocket);
	WSACleanup();
	system("pause");
	return 0;
}


int main() {
	client();
}