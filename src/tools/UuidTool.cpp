#include "UuidTool.h"
#include <windows.h>
#include <rpc.h>
#include <stdexcept>
#pragma comment(lib, "rpcrt4.lib")

std::string UuidTool::Execute(const std::string& action, const std::string& payload) {
    if (action != "generate") throw std::runtime_error("Unknown action: " + action);

    UUID uuid;
    UuidCreate(&uuid);

    RPC_CSTR uuidStr = nullptr;
    UuidToStringA(&uuid, &uuidStr);

    std::string result((char*)uuidStr);
    RpcStringFreeA(&uuidStr);

    return result;
}