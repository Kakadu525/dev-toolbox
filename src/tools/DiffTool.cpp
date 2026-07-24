#include "DiffTool.h"
#include <nlohmann/json.hpp>
#include <vector>
#include <sstream>
#include <stdexcept>

using json = nlohmann::json;

namespace {
    std::vector<std::string> SplitLines(const std::string& text) {
        std::vector<std::string> lines;
        std::stringstream ss(text);
        std::string line;
        while (std::getline(ss, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();      // Убирем возможный '\r' на конце 
            lines.push_back(line);
        }
        return lines;
    }

    // Классический LCS через динамическое программирование
    json ComputeDiff(const std::vector<std::string>& a, const std::vector<std::string>& b) {
        size_t n = a.size(), m = b.size();
        std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));

        for (size_t i = 1; i <= n; i++) {
            for (size_t j = 1; j <= m; j++) {
                if (a[i - 1] == b[j - 1])
                    dp[i][j] = dp[i - 1][j - 1] + 1;
                else
                    dp[i][j] = std::max(dp[i - 1][j], dp[i][j - 1]);
            }
        }

        // Восстанавливаем путь с конца, собираем результат в обратном порядке
        std::vector<json> reversedResult;
        size_t i = n, j = m;
        while (i > 0 && j > 0) {
            if (a[i - 1] == b[j - 1]) {
                reversedResult.push_back({ {"type", "same"}, {"line", a[i - 1]} });
                i--; j--;
            }
            else if (dp[i - 1][j] >= dp[i][j - 1]) {
                reversedResult.push_back({ {"type", "removed"}, {"line", a[i - 1]} });
                i--;
            }
            else {
                reversedResult.push_back({ {"type", "added"}, {"line", b[j - 1]} });
                j--;
            }
        }
        while (i > 0) { reversedResult.push_back({ {"type", "removed"}, {"line", a[i - 1]} }); i--; }
        while (j > 0) { reversedResult.push_back({ {"type", "added"}, {"line", b[j - 1]} }); j--; }

        json result = json::array();
        for (auto it = reversedResult.rbegin(); it != reversedResult.rend(); ++it) {
            result.push_back(*it);
        }
        return result;
    }
}

std::string DiffTool::Execute(const std::string& action, const std::string& payload) {
    if (action != "compare") {
        throw std::runtime_error("Unknown action: " + action);
    }

    json request;
    try {
        request = json::parse(payload);
    }
    catch (const json::parse_error&) {
        throw std::runtime_error("Invalid request format");
    }

    std::string textA = request.value("textA", "");
    std::string textB = request.value("textB", "");

    auto linesA = SplitLines(textA);
    auto linesB = SplitLines(textB);

    json diff = ComputeDiff(linesA, linesB);

    json result = { {"diff", diff} };
    return result.dump(4, ' ', false);
}