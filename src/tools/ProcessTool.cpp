#include "ProcessTool.h"
#include "../StringUtil.h"
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <sddl.h>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "advapi32.lib")

using json = nlohmann::json;

namespace {
    std::string GetSystemErrorMessage(DWORD err) {
        LPWSTR buf = nullptr;
        FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, err, 0, (LPWSTR)&buf, 0, nullptr);
        std::string result = buf ? WideToUtf8(buf) : ("System error code " + std::to_string(err));
        if (buf) LocalFree(buf);
        while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) result.pop_back();
        return result;
    }

    std::string GetProcessOwner(HANDLE hProcess) {
        HANDLE hToken = nullptr;
        if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) return "—";

        DWORD tokenInfoLen = 0;
        GetTokenInformation(hToken, TokenUser, nullptr, 0, &tokenInfoLen);
        if (tokenInfoLen == 0) { CloseHandle(hToken); return "—"; }

        std::vector<BYTE> buffer(tokenInfoLen);
        std::string result = "—";

        if (GetTokenInformation(hToken, TokenUser, buffer.data(), tokenInfoLen, &tokenInfoLen)) {
            auto* tokenUser = reinterpret_cast<TOKEN_USER*>(buffer.data());

            wchar_t nameBuf[256], domainBuf[256];
            DWORD nameLen = 256, domainLen = 256;
            SID_NAME_USE sidType;

            if (LookupAccountSidW(nullptr, tokenUser->User.Sid, nameBuf, &nameLen, domainBuf, &domainLen, &sidType)) {
                result = WideToUtf8(nameBuf);
            }
        }

        CloseHandle(hToken);
        return result;
    }

    std::string FormatStartTime(const FILETIME& ft) {
        SYSTEMTIME utcTime, localTime;
        if (!FileTimeToSystemTime(&ft, &utcTime)) return "—";
        SystemTimeToTzSpecificLocalTime(nullptr, &utcTime, &localTime);

        char buf[32];
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d",
            localTime.wYear, localTime.wMonth, localTime.wDay,
            localTime.wHour, localTime.wMinute);
        return buf;
    }

    json ListProcesses() {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("Failed to create process snapshot: " + GetSystemErrorMessage(GetLastError()));
        }

        PROCESSENTRY32W entry;
        entry.dwSize = sizeof(entry);

        json processes = json::array();

        if (Process32FirstW(snapshot, &entry)) {
            do {
                json procInfo;
                procInfo["pid"] = entry.th32ProcessID;
                procInfo["name"] = WideToUtf8(entry.szExeFile);
                procInfo["threads"] = entry.cntThreads;
                procInfo["parentPid"] = entry.th32ParentProcessID;

                std::string memoryStr = "—";
                long long memoryBytes = 0;
                std::string pathStr = "—";
                std::string userStr = "—";
                std::string startedStr = "—";

                HANDLE hProcess = OpenProcess(
                    PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ,
                    FALSE, entry.th32ProcessID);

                if (hProcess) {
                    PROCESS_MEMORY_COUNTERS_EX pmc = {};
                    if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
                        memoryBytes = (long long)pmc.WorkingSetSize;
                        double mb = pmc.WorkingSetSize / (1024.0 * 1024.0);
                        char buf[32];
                        snprintf(buf, sizeof(buf), "%.1f MB", mb);
                        memoryStr = buf;
                    }

                    wchar_t pathBuf[MAX_PATH];
                    DWORD pathSize = MAX_PATH;
                    if (QueryFullProcessImageNameW(hProcess, 0, pathBuf, &pathSize)) {
                        pathStr = WideToUtf8(pathBuf);
                    }

                    userStr = GetProcessOwner(hProcess);


                    FILETIME creationTime, exitTime, kernelTime, userTime;
                    if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
                        startedStr = FormatStartTime(creationTime);
                    }

                    CloseHandle(hProcess);
                }

                procInfo["memory"] = memoryStr;
                procInfo["memoryBytes"] = memoryBytes;
                procInfo["path"] = pathStr;
                procInfo["user"] = userStr;
                procInfo["started"] = startedStr;

                processes.push_back(procInfo);
            } while (Process32NextW(snapshot, &entry));
        }

        CloseHandle(snapshot);
        return processes;
    }

    json TerminateProcessById(DWORD pid) {
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (!hProcess) {
            DWORD err = GetLastError();
            return json{
                {"success", false},
                {"message", "Cannot open process (PID " + std::to_string(pid) + "): " + GetSystemErrorMessage(err)}
            };
        }

        BOOL ok = TerminateProcess(hProcess, 1);
        DWORD err = ok ? 0 : GetLastError();
        CloseHandle(hProcess);

        if (!ok) {
            return json{
                {"success", false},
                {"message", "Failed to terminate process (PID " + std::to_string(pid) + "): " + GetSystemErrorMessage(err) +
                            ". Возможно, процесс защищён системой (Protected Process) или требует больше прав."}
            };
        }

        return json{ {"success", true}, {"message", "Process terminated"} };
    }
}

std::string ProcessTool::Execute(const std::string& action, const std::string& payload) {
    if (action == "list") {
        json result = { {"processes", ListProcesses()} };
        return result.dump();
    }

    if (action == "terminate") {
        DWORD pid = 0;
        try {
            pid = (DWORD)std::stoul(payload);
        }
        catch (...) {
            throw std::runtime_error("Invalid PID: " + payload);
        }
        return TerminateProcessById(pid).dump();
    }

    throw std::runtime_error("Unknown action: " + action);
}