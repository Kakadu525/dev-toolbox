#pragma once
#include "ITool.h"

class CronTool : public ITool {
public:
    std::string Execute(const std::string& action, const std::string& payload) override;
};