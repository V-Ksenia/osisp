#include <iostream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <cctype>
#include <map>
#include <Windows.h>

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
	HINSTANCE load;
	load = LoadLibrary(L"SocketDLLib.dll");

	typedef bool (*init_winsock) ();
	init_winsock Init_Winsock;
	Init_Winsock = (init_winsock)GetProcAddress(load, "init_winsock");


    typedef SOCKET (*create_server_socket) (int);
    create_server_socket Create_Server_Socket;
    Create_Server_Socket = (create_server_socket)GetProcAddress(load, "create_server_socket");


    typedef SOCKET(*accept_client) (SOCKET);
    accept_client Accept_Client;
    Accept_Client = (accept_client)GetProcAddress(load, "accept_client");


    Init_Winsock();

    SOCKET serverSocket = Create_Server_Socket(8080);

    std::cout << "Сервер запущен. Ожидание подключения...\n";

    SOCKET clientSocket = Accept_Client(serverSocket);

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
	FreeLibrary(load);

    return 0;
}