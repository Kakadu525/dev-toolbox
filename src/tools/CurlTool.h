#pragma once
#include "ITool.h"

class CurlTool : public ITool {
public:
    std::string Execute(const std::string& action, const std::string& payload) override;
};