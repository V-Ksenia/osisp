#include <windows.h>
#include <iostream>
#include <string>

#define BUFFER_SIZE 1024               
#define SHARED_MEMORY_NAME "Local\\MySharedMemory"
#define MUTEX_NAME "Local\\MyMutex"
#define MAX_MESSAGES 10                
#define MESSAGE_SIZE 100               

using namespace std;

HANDLE hMapFile;
LPVOID pBuf;
HANDLE hMutex;
char* messageBuffer;


void initSharedMemory() {

    hMutex = CreateMutex(NULL, FALSE, MUTEX_NAME);
    if (hMutex == NULL) {
        cout << "CreateMutex error: " << GetLastError() << endl;
        exit(1);
    }

    hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,    
        NULL,                    
        PAGE_READWRITE,          
        0,                       
        BUFFER_SIZE,             
        SHARED_MEMORY_NAME);    

    if (hMapFile == NULL) {
        cout << "Could not create file mapping object: " << GetLastError() << endl;
        exit(1);
    }


    pBuf = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
    if (pBuf == NULL) {
        cout << "Could not map view of file: " << GetLastError() << endl;
        CloseHandle(hMapFile);
        exit(1);
    }

    messageBuffer = (char*)pBuf;
}


void writeMessage(const string& message) {
    WaitForSingleObject(hMutex, INFINITE); 

    int messageCount = 0;
    for (int i = 0; i < MAX_MESSAGES; ++i) {
        if (messageBuffer[i * MESSAGE_SIZE] == '\0') {
            strncpy(messageBuffer + (i * MESSAGE_SIZE), message.c_str(), MESSAGE_SIZE);
            Sleep(5000);
            cout << "Введенное сообщение: " << message << endl;
            break;
        }
        messageCount++;
    }

    if (messageCount == MAX_MESSAGES) {
        cout << "Максимальное количество сообщений - 10!" << endl;
    }

    ReleaseMutex(hMutex); 
}

void writeMessage2(const string& message) {
    WaitForSingleObject(hMutex, INFINITE); 

    int messageCount = 0;
    for (int i = 0; i < MAX_MESSAGES; ++i) {
        if (messageBuffer[i * MESSAGE_SIZE] == '\0') {
            strncpy(messageBuffer + (i * MESSAGE_SIZE), message.c_str(), MESSAGE_SIZE);
            Sleep(2000);
            cout << "Введенное сообщение: " << message << endl;
            break;
        }
        messageCount++;
    }

    if (messageCount == MAX_MESSAGES) {
        cout << "Максимальное количество сообщений - 10!" << endl;
    }

    ReleaseMutex(hMutex); 
}


void readMessages() {
    WaitForSingleObject(hMutex, INFINITE);

    cout << "Сообщения из общей памяти: " << endl;
    for (int i = 0; i < MAX_MESSAGES; ++i) {
        if (messageBuffer[i * MESSAGE_SIZE] != '\0') {
            cout << "Сообщение " << (i + 1) << ": " << messageBuffer + (i * MESSAGE_SIZE) << endl;
        }
        else {
            break;
        }
    }

    ReleaseMutex(hMutex);
}


void clearBuffer() {
    WaitForSingleObject(hMutex, INFINITE); 

    memset(messageBuffer, 0, BUFFER_SIZE);

    cout << "Буфер сообщений очищен." << endl;

    ReleaseMutex(hMutex); 
}

void writer() {
    initSharedMemory();

    string message;
    while (true) {
        cout << "Введите сообщение (введите 'exit' чтобы выйти): ";
        getline(cin, message);

        if (message == "exit") {
            break;
        }

        writeMessage(message);
    }
}

void writer2() {
    initSharedMemory();

    string message;
    while (true) {
        cout << "Введите сообщение (введите 'exit' чтобы выйти): ";
        getline(cin, message);

        if (message == "exit") {
            break;
        }

        writeMessage2(message);
    }
}

void reader() {
    initSharedMemory();

    readMessages();

    clearBuffer();
}

int main() {
    int choice;

    while (true)
    {
        cout << "Выберите режим:\n1. Запись\n2. Чтение\n";
        cin >> choice;
        cin.ignore();  

        if (choice == 1) {
            writer();
        }
        else if (choice == 2) {
            reader();
        }
        else if (choice == 3) {
            writer2();
        }
        else {
            break;
        }
    }

    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);
    CloseHandle(hMutex);

    return 0;
}
