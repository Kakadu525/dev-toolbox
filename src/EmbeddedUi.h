#pragma once
#include <string>

class EmbeddedUi {
public:
    // Извлекает встроенный в exe zip-архив ui/ во временную папку, возвращает путь к ней.
    static std::wstring ExtractToTempDir();

    // Удаляет ранее извлечённую временную папку 
    static void Cleanup(const std::wstring& dir);
};