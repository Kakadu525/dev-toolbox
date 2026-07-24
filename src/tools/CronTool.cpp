#include "CronTool.h"
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <set>
#include <ctime>
#include <iomanip>

using json = nlohmann::json;

namespace {
    std::set<int> ParseField(const std::string& field, int minVal, int maxVal) {
        std::set<int> result;

        std::stringstream ss(field);
        std::string part;
        while (std::getline(ss, part, ',')) {
            if (part == "*") {
                for (int i = minVal; i <= maxVal; i++) result.insert(i);
            }
            else if (part.find('/') != std::string::npos) {
                // формат */N или M-N/step
                auto slashPos = part.find('/');
                std::string rangePart = part.substr(0, slashPos);
                int step = std::stoi(part.substr(slashPos + 1));

                int start = minVal, end = maxVal;
                if (rangePart != "*") {
                    auto dashPos = rangePart.find('-');
                    if (dashPos != std::string::npos) {
                        start = std::stoi(rangePart.substr(0, dashPos));
                        end = std::stoi(rangePart.substr(dashPos + 1));
                    }
                    else {
                        start = std::stoi(rangePart);
                        end = maxVal;
                    }
                }
                for (int i = start; i <= end; i += step) result.insert(i);
            }
            else if (part.find('-') != std::string::npos) {
                auto dashPos = part.find('-');
                int start = std::stoi(part.substr(0, dashPos));
                int end = std::stoi(part.substr(dashPos + 1));
                for (int i = start; i <= end; i++) result.insert(i);
            }
            else {
                result.insert(std::stoi(part));
            }
        }

        for (int v : result) {
            if (v < minVal || v > maxVal) {
                throw std::runtime_error("Value " + std::to_string(v) + " out of range [" +
                    std::to_string(minVal) + "-" + std::to_string(maxVal) + "]");
            }
        }

        return result;
    }

    struct CronSchedule {
        std::set<int> minutes;
        std::set<int> hours;
        std::set<int> daysOfMonth;
        std::set<int> months;
        std::set<int> daysOfWeek;
    };

    CronSchedule ParseCron(const std::string& expr) {
        std::vector<std::string> fields;
        std::stringstream ss(expr);
        std::string field;
        while (ss >> field) fields.push_back(field);

        if (fields.size() != 5) {
            throw std::runtime_error("Cron expression must have 5 fields (minute hour day month weekday), got " +
                std::to_string(fields.size()));
        }

        CronSchedule schedule;
        schedule.minutes = ParseField(fields[0], 0, 59);
        schedule.hours = ParseField(fields[1], 0, 23);
        schedule.daysOfMonth = ParseField(fields[2], 1, 31);
        schedule.months = ParseField(fields[3], 1, 12);
        schedule.daysOfWeek = ParseField(fields[4], 0, 6); // 0 = воскресенье
        return schedule;
    }

    // Перебор по минутам, максимум 2 года вперёд для защиты от зависания
    std::vector<std::string> FindNextRuns(const CronSchedule& schedule, int count) {
        std::vector<std::string> results;
        std::time_t now = std::time(nullptr);
        std::tm current;
        localtime_s(&current, &now);
        current.tm_sec = 0;
        current.tm_min += 1; // начинаем со следующей минуты

        int maxIterations = 60 * 24 * 366 * 2; // ~2 года в минутах, защита
        for (int i = 0; i < maxIterations && (int)results.size() < count; i++) {
            std::time_t t = std::mktime(&current);
            std::tm normalized;
            localtime_s(&normalized, &t);

            bool minuteOk = schedule.minutes.count(normalized.tm_min);
            bool hourOk = schedule.hours.count(normalized.tm_hour);
            bool domOk = schedule.daysOfMonth.count(normalized.tm_mday);
            bool monthOk = schedule.months.count(normalized.tm_mon + 1);
            bool dowOk = schedule.daysOfWeek.count(normalized.tm_wday);

            if (minuteOk && hourOk && domOk && monthOk && dowOk) {
                std::ostringstream dateStr;
                dateStr << std::put_time(&normalized, "%Y-%m-%d %H:%M");
                results.push_back(dateStr.str());
            }

            current = normalized;
            current.tm_min += 1;
        }

        return results;
    }
}

std::string CronTool::Execute(const std::string& action, const std::string& payload) {
    if (action != "parse") {
        throw std::runtime_error("Unknown action: " + action);
    }

    CronSchedule schedule = ParseCron(payload);
    std::vector<std::string> nextRuns = FindNextRuns(schedule, 5);

    json result = {
        {"nextRuns", nextRuns}
    };

    return result.dump(4, ' ', false);
}