#include "EmbeddedUi.h"
#include "StringUtil.h"
#include <windows.h>
#include <miniz.h>
#include <stdexcept>
#include <string>

namespace {
    // Создаёт все промежуточные папки для полного пути к файлу
    void CreateDirsForFile(const std::wstring& filePath) {
        size_t pos = 0;
        while ((pos = filePath.find(L'\\', pos + 1)) != std::wstring::npos) {
            std::wstring sub = filePath.substr(0, pos);
            CreateDirectoryW(sub.c_str(), nullptr); // ошибку "уже существует" игнорируем
        }
    }

    void DeleteDirectoryRecursive(const std::wstring& dir) {
        std::wstring searchPath = dir + L"\\*";
        WIN32_FIND_DATAW findData;
        HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
        if (hFind == INVALID_HANDLE_VALUE) return;

        do {
            std::wstring name = findData.cFileName;
            if (name == L"." || name == L"..") continue;

            std::wstring fullPath = dir + L"\\" + name;
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                DeleteDirectoryRecursive(fullPath);
            }
            else {
                DeleteFileW(fullPath.c_str());
            }
        } while (FindNextFileW(hFind, &findData));

        FindClose(hFind);
        RemoveDirectoryW(dir.c_str());
    }
}

std::wstring EmbeddedUi::ExtractToTempDir() {
    HMODULE hModule = GetModuleHandleW(nullptr);
    HRSRC hRes = FindResourceW(hModule, MAKEINTRESOURCEW(201), RT_RCDATA);
    if (!hRes) throw std::runtime_error("Embedded UI resource not found");

    HGLOBAL hData = LoadResource(hModule, hRes);
    if (!hData) throw std::runtime_error("Failed to load embedded UI resource");

    DWORD size = SizeofResource(hModule, hRes);
    void* pData = LockResource(hData);
    if (!pData || size == 0) throw std::runtime_error("Embedded UI resource is empty");

    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    std::wstring extractDir = std::wstring(tempPath) + L"DevToolbox-ui-" + std::to_wstring(GetCurrentProcessId());
    CreateDirectoryW(extractDir.c_str(), nullptr);

    // Открываем zip прямо из памяти
    mz_zip_archive zipArchive;
    memset(&zipArchive, 0, sizeof(zipArchive));

    if (!mz_zip_reader_init_mem(&zipArchive, pData, size, 0)) {
        throw std::runtime_error("Failed to open embedded UI archive");
    }

    int fileCount = (int)mz_zip_reader_get_num_files(&zipArchive);
    for (int i = 0; i < fileCount; i++) {
        mz_zip_archive_file_stat fileStat;
        if (!mz_zip_reader_file_stat(&zipArchive, i, &fileStat)) continue;
        if (mz_zip_reader_is_file_a_directory(&zipArchive, i)) continue;

        std::wstring relativePath = Utf8ToWide(fileStat.m_filename);
        for (auto& c : relativePath) if (c == L'/') c = L'\\';

        std::wstring fullPath = extractDir + L"\\" + relativePath;
        CreateDirsForFile(fullPath);


        size_t outSize = 0;
        void* extractedData = mz_zip_reader_extract_to_heap(&zipArchive, i, &outSize, 0);
        if (extractedData) {
            HANDLE hFile = CreateFileW(fullPath.c_str(), GENERIC_WRITE, 0, nullptr,
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (hFile != INVALID_HANDLE_VALUE) {
                DWORD written = 0;
                WriteFile(hFile, extractedData, (DWORD)outSize, &written, nullptr);
                CloseHandle(hFile);
            }
            mz_free(extractedData);
        }
    }

    mz_zip_reader_end(&zipArchive);
    return extractDir;
}

void EmbeddedUi::Cleanup(const std::wstring& dir) {
    if (!dir.empty()) {
        DeleteDirectoryRecursive(dir);
    }
}