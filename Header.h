#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

extern "C" __declspec(dllexport) bool init_winsock();

extern "C" __declspec(dllexport) SOCKET create_server_socket(int);

extern "C" __declspec(dllexport) SOCKET create_client_socket(int);

extern "C" __declspec(dllexport) SOCKET accept_client(SOCKET);