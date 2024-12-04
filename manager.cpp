#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

const int BUFFER_SIZE = 1024;

int main() {
    WSADATA wsaData;


    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Ошибка: инициализация Winsock не удалась.\n";
        return 1;
    }


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

    std::cout << "Подключено к серверу.\n";

    // Ввод и отправка команд
    char buffer[BUFFER_SIZE];
    while (true) {
        std::string command;
        std::cout << "Введите команду: ";
        std::getline(std::cin, command);

        send(clientSocket, command.c_str(), command.size(), 0);

        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived > 0) {
            std::cout << "Результат:\n" << buffer << "\n";
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
