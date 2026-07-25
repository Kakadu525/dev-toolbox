#pragma once
#include "ITool.h"

class SettingsTool : public ITool {
public:
    std::string Execute(const std::string& action, const std::string& payload) override;

private:
    std::string GetSettingsFilePath();
};