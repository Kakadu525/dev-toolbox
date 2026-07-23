#pragma once
#include <string>

class ITool {
public:
    virtual ~ITool() = default;
    virtual std::string Execute(const std::string& action, const std::string& payload) = 0;
};