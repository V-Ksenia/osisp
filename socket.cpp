#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")

extern "C" __declspec(dllexport) bool init_winsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Ошибка: инициализация Winsock не удалась.\n";
        return false;
    }
    return true;
}

extern "C" __declspec(dllexport) SOCKET create_server_socket(int port) {
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Ошибка: не удалось создать сокет.\n";
        return INVALID_SOCKET;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Ошибка: не удалось привязать сокет.\n";
        closesocket(serverSocket);
        return INVALID_SOCKET;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Ошибка: не удалось начать прослушивание.\n";
        closesocket(serverSocket);
        return INVALID_SOCKET;
    }

    return serverSocket;
}

extern "C" __declspec(dllexport) SOCKET create_client_socket(int port) {
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Ошибка: не удалось создать сокет.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);


    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Ошибка. Не удалось подключиться к серверу :(.\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    return clientSocket;
}

extern "C" __declspec(dllexport) SOCKET accept_client(SOCKET serverSocket) {
    sockaddr_in clientAddr{};
    int clientAddrSize = sizeof(clientAddr);
    SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Ошибка: не удалось принять соединение.\n";
    }
    return clientSocket;
}