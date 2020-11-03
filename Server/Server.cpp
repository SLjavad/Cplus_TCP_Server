#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment (lib,"Ws2_32.lib")

#define SOCKET_PORT "2345"

using namespace std;
HANDLE threadHandle;
bool sendMsg(char data[], int len, SOCKET clientSocket);

void recvThread(void* data)
{
	SOCKET clientSock = *(SOCKET*)data;
	int iResult;
	while (true) {

		char recvbuf[512];
		iResult = recv(clientSock, recvbuf, 512, 0);
		if (iResult == SOCKET_ERROR)
		{
			cerr << "Error in recv : " << WSAGetLastError() << endl;
			break;
		}
		else if (iResult > 0)
		{
			recvbuf[iResult] = '\0';
			sendMsg(recvbuf, iResult, clientSock);
			cout << "RECEIVE : " << recvbuf << endl;
		}
		else if (iResult == 0)
		{
			cout << "Socket Closed" << endl;
			getchar();
			break;
		}
	}

	// Shutting Down the socket
	iResult = shutdown(clientSock, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		cerr << "Shutdown Failed " << WSAGetLastError() << endl;
	}
	TerminateThread(threadHandle, GetExitCodeThread(threadHandle, 0));
	closesocket(clientSock);
	WSACleanup();
}

bool sendMsg(char data[], int len, SOCKET clientSocket) {
	int iSendResult = send(clientSocket, data, len, 0);
	if (iSendResult == SOCKET_ERROR)
	{
		cout << "Sending ERROR : " << WSAGetLastError() << endl;
		return false;
	}
	cout << "Message Sent : " << data << endl;
	return true;
}


int main(int argc, char **argv)
{
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	// Initializing
	int result = WSAStartup(ver, &wsData);

	if (result != 0)
	{
		cerr << "WSAStartup failed" << endl;
		return 0;
	}

	//Address Resolve
	struct addrinfo *addrresult = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	result = getaddrinfo(NULL, SOCKET_PORT, &hints, &addrresult);
	if (result != 0)
	{
		cerr << "getaddrinfo failed : " << result << endl;
		WSACleanup();
		return 0;
	}

	//Create Socket
	SOCKET serverSocket = socket(addrresult->ai_family, addrresult->ai_socktype, addrresult->ai_protocol);
	if (serverSocket == INVALID_SOCKET)
	{
		cerr << "serverSocket Failed : " << WSAGetLastError() << endl;
		freeaddrinfo(addrresult);
		WSACleanup();
		return 0;
	}

	cout << "Socket Created" << endl;

	//Binding Socket To ip and port
	result = bind(serverSocket, addrresult->ai_addr, (int)addrresult->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		cerr << "bind failed : " << WSAGetLastError() << endl;
		freeaddrinfo(addrresult);
		closesocket(serverSocket);
		WSACleanup();
		return 0;
	}

	cout << "Socket Binded" << endl;

	cout << "Listening ..." << endl;
	// Listening
	result = listen(serverSocket, SOMAXCONN);
	if (result == SOCKET_ERROR)
	{
		cerr << "listen failed : " << WSAGetLastError() << endl;
		closesocket(serverSocket);
		WSACleanup();
		return 0;
	}

	//Accepting Client Socket
	sockaddr clientAddr;
	int clientSize = sizeof(clientAddr);

	SOCKET clientSocket = accept(serverSocket, (sockaddr *)&clientAddr, &clientSize);
	if (clientSocket == INVALID_SOCKET || clientSocket == SOCKET_ERROR)
	{
		cerr << "accept failed : " << WSAGetLastError() << endl;
		closesocket(serverSocket);
		WSACleanup();
		return 0;
	}

	cout << "Client Accepted" << endl;

	closesocket(serverSocket);


	// Receiving Data From Client On another thead

	threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)recvThread, &clientSocket, 0, 0);
	// while loop for input keyborad
	while (true)
	{
		char input[10];
		cin >> input;
		if (strcmp(input,"w") == 0)
		{
			char input[10] = "0,0,600";
			sendMsg(input, sizeof(input) / sizeof(char), clientSocket);
			char input2[10] = "1,0,600";
			sendMsg(input2, sizeof(input2) / sizeof(char), clientSocket);
		}
		else if (strcmp(input, "s") == 0)
		{
			char input[10] = "0,1,600";
			sendMsg(input, sizeof(input) / sizeof(char), clientSocket);
			char input2[10] = "1,1,600";
			sendMsg(input2, sizeof(input2) / sizeof(char), clientSocket);
		}
		else
		{
			sendMsg(input, sizeof(input) / sizeof(char), clientSocket);
		}
	}


	system("pause");
}

