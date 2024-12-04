#include <windows.h>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>

void InsertCharacterMap(void* buffer, DWORD fileSize, char insertChar, DWORD position, void*& newBuffer, DWORD& newBufferSize) {

    newBufferSize = fileSize + 1;
    newBuffer = malloc(newBufferSize);

    if (!newBuffer) {
        std::cerr << "Ошибка выделения памяти для нового буфера." << std::endl;
        return;
    }

    memcpy(newBuffer, buffer, position);
    static_cast<char*>(newBuffer)[position] = insertChar;
    memcpy(static_cast<char*>(newBuffer) + position + 1, static_cast<char*>(buffer) + position, fileSize - position);
}

void MapFileToMemory(const char* inputFilename) {
    HANDLE hFile = CreateFileA(inputFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Ошибка открытия файла для чтения." << std::endl;
        return;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        std::cerr << "Ошибка получения размера файла." << std::endl;
        CloseHandle(hFile);
        return;
    }
    
    HANDLE hMapping = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hMapping == NULL) {
        std::cerr << "Ошибка создания отображения файла." << std::endl;
        CloseHandle(hFile);
        return;
    }
    std::cout << "MAP.Отображение файла создано." << std::endl;


    HANDLE hOutputFile = CreateFileA("output.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hOutputFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Ошибка открытия выходного файла." << std::endl;
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return;
    }

    char insertChar = 'X';
    DWORD insertPosition = 10;

    void* newBuffer = nullptr;
    DWORD newBufferSize = 0;

    auto start = std::chrono::high_resolution_clock::now();

    void* dataArray = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (dataArray == NULL) {
        std::cerr << "Ошибка отображения файла в память." << std::endl;
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return;
    }
    std::cout << "MAP.Файл отображен в память." << std::endl;

    std::cout << "MAP.Вставка символа." << std::endl;
    InsertCharacterMap(dataArray, fileSize, insertChar, insertPosition, newBuffer, newBufferSize);

    if (!newBuffer) {
        UnmapViewOfFile(dataArray);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return;
    }

    std::cout << "MAP.Перезапись файла." << std::endl;
    DWORD bytesWritten;
    if (!WriteFile(hOutputFile, newBuffer, newBufferSize, &bytesWritten, NULL)) {
        std::cerr << "Ошибка записи в выходной файл." << std::endl;
    }
    else if (bytesWritten != newBufferSize) {
        std::cerr << "Количество записанных байт не соответствует размеру буфера!" << std::endl;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "MAP.Время копирования данных: " << elapsed.count() << " секунд." << std::endl;


    UnmapViewOfFile(dataArray);
    CloseHandle(hOutputFile);
    CloseHandle(hMapping);
    CloseHandle(hFile);
}

void InsertCharacter(char* buffer, DWORD& bytesRead, char insertChar, DWORD insertPos, char*& newBuffer, DWORD& newBufferSize) {
    newBufferSize = bytesRead + 1;
    newBuffer = new char[newBufferSize];

    memcpy(newBuffer, buffer, insertPos);

    newBuffer[insertPos] = insertChar;

    memcpy(newBuffer + insertPos + 1, buffer + insertPos, bytesRead - insertPos);
}

void CopyFileData(const char* sourceFilename, const char* destFilename, char insertChar, DWORD insertPosition) {

    
    HANDLE hSourceFile = CreateFileA(sourceFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSourceFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Ошибка открытия файла для чтения: " << GetLastError() << std::endl;
        return;
    }

    DWORD fileSize = GetFileSize(hSourceFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        std::cerr << "Ошибка получения размера файла: " << GetLastError() << std::endl;
        CloseHandle(hSourceFile);
        return;
    }


    char* buffer = new char[fileSize];

    std::cout << "SYNC.Чтение файла." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    DWORD bytesRead;
    if (!ReadFile(hSourceFile, buffer, fileSize, &bytesRead, NULL)) {
        std::cerr << "Ошибка чтения файла: " << GetLastError() << std::endl;
        delete[] buffer;
        CloseHandle(hSourceFile);
        return;
    }
    CloseHandle(hSourceFile);  

    std::cout << "SYNC.Вставка символа." << std::endl;
    char* newBuffer = nullptr;
    DWORD newBufferSize = 0;

    if (insertPosition <= fileSize) {
        InsertCharacter(buffer, bytesRead, insertChar, insertPosition, newBuffer, newBufferSize);
    }
    else {
        std::cerr << "Позиция вставки превышает размер файла." << std::endl;
        delete[] buffer;
        return;
    }

    delete[] buffer; 

    HANDLE hDestFile = CreateFileA(destFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDestFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Ошибка открытия файла для записи: " << GetLastError() << std::endl;
        delete[] newBuffer;
        return;
    }

    std::cout << "SYNC.Перезапись файла." << std::endl;
    DWORD bytesWritten;
    if (!WriteFile(hDestFile, newBuffer, newBufferSize, &bytesWritten, NULL) || bytesWritten != newBufferSize) {
        std::cerr << "Ошибка записи в файл: " << GetLastError() << std::endl;
        delete[] newBuffer;
        CloseHandle(hDestFile);
        return;
    }

    CloseHandle(hDestFile);
    delete[] newBuffer;

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "SYNC. Время обработки: " << elapsed.count() << " секунд." << std::endl;

}
int main() {
    
    const char* filename = "file.txt";
    char insertChar = 'X';
    DWORD insertPosition = 10;
    int n = 0;
    while (n < 3)
    {
        CopyFileData(filename, "destFilename.txt", insertChar, insertPosition);
        MapFileToMemory(filename);
        n++;
    }
    

    return 0;
}
