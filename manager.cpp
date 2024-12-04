#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

const int BUFFER_SIZE = 1024;

int main()
{
    HINSTANCE load;
    load = LoadLibrary(L"SocketDLLib.dll");

    typedef bool (*init_winsock) ();
    init_winsock Init_Winsock;
    Init_Winsock = (init_winsock)GetProcAddress(load, "init_winsock");


    typedef SOCKET(*create_client_socket) (int);
    create_client_socket Create_Client_Socket;
    Create_Client_Socket = (create_client_socket)GetProcAddress(load, "create_client_socket");


    Init_Winsock();
    SOCKET clientSocket = Create_Client_Socket(8080);

    std::cout << "Подключено к серверу.\n";

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
    FreeLibrary(load);
    return 0;
}
