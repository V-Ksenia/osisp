#include <iostream>
#include <string>
#include <map>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <cctype>

#pragma comment(lib, "ws2_32.lib")

const int BUFFER_SIZE = 1024;

std::string handle_command(const std::string& command) {
    // reverse <текст>
    if (command.rfind("reverse ", 0) == 0) {
        std::string text = command.substr(8);
        std::reverse(text.begin(), text.end());
        return "Результат: " + text + "\n";
    }

    // length <текст>
    if (command.rfind("length ", 0) == 0) {
        std::string text = command.substr(7);
        return "Длина текста: " + std::to_string(text.length()) + "\n";
    }

    // uppercase <текст>
    if (command.rfind("uppercase ", 0) == 0) {
        std::string text = command.substr(10);
        std::transform(text.begin(), text.end(), text.begin(), ::toupper);
        return "Результат: " + text + "\n";
    }


    static std::map<std::string, std::string> predefinedCommands = {
        {"hello", "Hello world!"},
        {"help", "Доступные команды: hello, help, reverse <текст>, length <текст>, uppercase <текст>, quit."},
        {"quit", "Завершение работы с сервером."}
    };

    auto it = predefinedCommands.find(command);
    if (it != predefinedCommands.end()) {
        return it->second + "\n";
    }

    return "Неизвестная команда. Введите 'help' для списка команд.\n";
}

int main() {
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Ошибка инициализации Winsock.\n";
        return 1;
    }


    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Ошибка создания сокета.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);


    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Ошибка привязки сокет.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }


    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Ошибка начала прослушивания.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Сервер запущен. Ожидание подключения...\n";


    sockaddr_in clientAddr{};
    int clientAddrSize = sizeof(clientAddr);
    SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Ошибка. Не удалось принять соединение.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Клиент подключен.\n";


    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);


        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived <= 0) {
            std::cout << "Клиент отключился.\n";
            break;
        }

        std::string command(buffer);
        std::cout << "Получена команда: " << command << "\n";


        if (command == "quit") {
            std::cout << "Команда завершения работы от клиента.\n";
            std::string goodbyeMessage = handle_command(command);
            send(clientSocket, goodbyeMessage.c_str(), goodbyeMessage.size(), 0);
            break;
        }


        std::string result = handle_command(command);

        send(clientSocket, result.c_str(), result.size(), 0);
    }

    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
