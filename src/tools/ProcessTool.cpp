#include "ProcessTool.h"
#include "../StringUtil.h"
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <nlohmann/json.hpp>
#include <stdexcept>
#pragma comment(lib, "psapi.lib")

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

                    CloseHandle(hProcess);
                }

                procInfo["memory"] = memoryStr;
                procInfo["memoryBytes"] = memoryBytes;
                procInfo["path"] = pathStr;

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