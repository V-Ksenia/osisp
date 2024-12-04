#include <iostream>
#include <windows.h>
#include <vector>
#include <random>   
#include <chrono>
#include <thread>

int getRandomValue() {
    static thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(0, 1000);
    return distribution(generator);
}

class SharedMemory {
private:
    std::vector<int> memory;
    std::vector<HANDLE> blockMutexes;  
    int blockSize;

public:
    SharedMemory(int totalSize, int blockSize)
        : memory(totalSize, 0), blockSize(blockSize) {
        int blockCount = totalSize / blockSize;
        blockMutexes.resize(blockCount);

        for (int i = 0; i < blockCount; ++i) {
            blockMutexes[i] = CreateMutex(NULL, FALSE, NULL);
            if (blockMutexes[i] == NULL) {
                std::cerr << "Ошибка создания мьютекса для блока " << i << std::endl;
                exit(1);
            }
        }
    }

    ~SharedMemory() {
        for (HANDLE mutex : blockMutexes) {
            CloseHandle(mutex);
        }
    }

    void readBlock(int blockIndex, int readerId, HANDLE coutMutex) {
        WaitForSingleObject(blockMutexes[blockIndex], INFINITE); 

        WaitForSingleObject(coutMutex, INFINITE);
        std::cout << "Читатель " << readerId << " читает блок " << blockIndex << ": ";

        for (int i = blockIndex * blockSize; i < (blockIndex + 1) * blockSize; ++i) {
            std::cout << memory[i] << " ";
        }
        std::cout << std::endl;

        ReleaseMutex(coutMutex); 
        ReleaseMutex(blockMutexes[blockIndex]); 
    }

    void writeBlock(int blockIndex, int writerId, HANDLE coutMutex) {
        WaitForSingleObject(blockMutexes[blockIndex], INFINITE); 
        int value = getRandomValue();

        WaitForSingleObject(coutMutex, INFINITE);
        std::cout << "Писатель " << writerId << " записывает в блок " << blockIndex << ": " << value << std::endl;

        for (int i = blockIndex * blockSize; i < (blockIndex + 1) * blockSize; ++i) {
            memory[i] = value;
        }

        ReleaseMutex(coutMutex);  
        ReleaseMutex(blockMutexes[blockIndex]);  ]
    }

    int getBlockCount() const {
        return blockMutexes.size();
    }
};

int getRandomBlockIndex(int blockCount) {
    static thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(0, blockCount - 1);
    return distribution(generator);
}


DWORD WINAPI readerTask(LPVOID lpParam) {
    auto params = static_cast<std::tuple<SharedMemory*, int, HANDLE>*>(lpParam);
    SharedMemory* sharedMemory = std::get<0>(*params);
    int readerId = std::get<1>(*params);
    HANDLE coutMutex = std::get<2>(*params);

    int blockCount = sharedMemory->getBlockCount();
    for (int i = 0; i < 5; ++i) {
        int blockIndex = getRandomBlockIndex(blockCount);
        sharedMemory->readBlock(blockIndex, readerId, coutMutex);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}


DWORD WINAPI writerTask(LPVOID lpParam) {
    auto params = static_cast<std::tuple<SharedMemory*, int, HANDLE>*>(lpParam);
    SharedMemory* sharedMemory = std::get<0>(*params);
    int writerId = std::get<1>(*params);
    HANDLE coutMutex = std::get<2>(*params);

    int blockCount = sharedMemory->getBlockCount();
    for (int i = 0; i < 5; ++i) {
        int blockIndex = getRandomBlockIndex(blockCount);
        sharedMemory->writeBlock(blockIndex, writerId, coutMutex);
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }

    return 0;
}

int main() {
    int memorySize = 20;
    int blockSize = 5;

    int readersCount = 3;
    int writersCount = 2;

    HANDLE coutMutex = CreateMutex(NULL, FALSE, NULL);  

    if (coutMutex == NULL) {
        std::cerr << "Ошибка создания мьютекса для вывода" << std::endl;
        return 1;
    }

    SharedMemory sharedMemory(memorySize, blockSize);

    std::vector<HANDLE> readers;
    std::vector<HANDLE> writers;

    
    for (int i = 0; i < readersCount; ++i) {
        auto params = new std::tuple<SharedMemory*, int, HANDLE>(&sharedMemory, i + 1, coutMutex);
        HANDLE reader = CreateThread(NULL, 0, readerTask, params, 0, NULL);
        readers.push_back(reader);
    }


    for (int i = 0; i < writersCount; ++i) {
        auto params = new std::tuple<SharedMemory*, int, HANDLE>(&sharedMemory, i + 1, coutMutex);
        HANDLE writer = CreateThread(NULL, 0, writerTask, params, 0, NULL);
        writers.push_back(writer);
    }

    for (HANDLE reader : readers) {
        WaitForSingleObject(reader, INFINITE);
        CloseHandle(reader);
    }

    for (HANDLE writer : writers) {
        WaitForSingleObject(writer, INFINITE);
        CloseHandle(writer);
    }

    CloseHandle(coutMutex);

    std::cout << "Все потоки завершили работу." << std::endl;

    return 0;
}
