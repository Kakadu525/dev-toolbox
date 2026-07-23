#include "HashTool.h"
#include <windows.h>
#include <bcrypt.h>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <iomanip>
#pragma comment(lib, "bcrypt.lib")

namespace {
    std::string ComputeHash(LPCWSTR algorithmId, const std::string& input) {
        BCRYPT_ALG_HANDLE hAlg = nullptr;
        BCRYPT_HASH_HANDLE hHash = nullptr;
        DWORD cbData = 0, cbHashObject = 0, cbHash = 0;
        std::vector<BYTE> hashObject;
        std::vector<BYTE> hashResult;
        std::string outputHex;

        // 1: Открываем нужный алгоритм (MD5 или SHA256)
        NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg, algorithmId, nullptr, 0);
        if (status != 0) throw std::runtime_error("Failed to open algorithm provider");

        // 2: Узнаём, сколько памяти нужно под "объект хэша" 
        BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbHashObject, sizeof(DWORD), &cbData, 0);
        hashObject.resize(cbHashObject);

        // 3: Узнаём длину итогового хэша в байтах (16 для MD5, 32 для SHA256)
        BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH, (PBYTE)&cbHash, sizeof(DWORD), &cbData, 0);
        hashResult.resize(cbHash);

        // 4: Создаём объект хэширования
        status = BCryptCreateHash(hAlg, &hHash, hashObject.data(), cbHashObject, nullptr, 0, 0);
        if (status != 0) {
            BCryptCloseAlgorithmProvider(hAlg, 0);
            throw std::runtime_error("Failed to create hash");
        }

        // 5: Ввлдим входные данные 
        BCryptHashData(hHash, (PBYTE)input.data(), (ULONG)input.size(), 0);

        // 6: Получаем финальный хэш
        BCryptFinishHash(hHash, hashResult.data(), cbHash, 0);

        // Освобождаем ресурсы
        BCryptDestroyHash(hHash);
        BCryptCloseAlgorithmProvider(hAlg, 0);

        // Переводим байты в привычную hex-строку
        std::ostringstream hexStream;
        for (BYTE b : hashResult) {
            hexStream << std::hex << std::setw(2) << std::setfill('0') << (int)b;
        }
        return hexStream.str();
    }
}

std::string HashTool::Execute(const std::string& action, const std::string& payload) {
    if (action == "md5") return ComputeHash(BCRYPT_MD5_ALGORITHM, payload);
    if (action == "sha256") return ComputeHash(BCRYPT_SHA256_ALGORITHM, payload);
    throw std::runtime_error("Unknown action: " + action);
}