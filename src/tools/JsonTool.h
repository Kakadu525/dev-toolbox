#pragma once
#include "ITool.h"

class JsonTool : public ITool {
public:
    std::string Execute(const std::string& action, const std::string& payload) override;
};