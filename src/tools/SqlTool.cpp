#include "SqlTool.h"
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <set>

namespace {
    std::string ToUpper(const std::string& s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    }

    const std::set<std::string> kNewlineKeywords = {
        "SELECT", "FROM", "WHERE", "AND", "OR", "JOIN", "LEFT", "RIGHT", "INNER", "OUTER",
        "ORDER", "GROUP", "HAVING", "LIMIT", "INSERT", "UPDATE", "DELETE", "SET", "VALUES",
        "UNION", "ON", "WITH"
    };

    const std::set<std::string> kCapitalizeOnly = {
        "AS", "ASC", "DESC", "BY", "INTO", "TABLE", "DISTINCT", "NOT", "NULL",
        "CASE", "WHEN", "THEN", "ELSE", "END"
    };

    std::vector<std::string> Tokenize(const std::string& sql) {
        std::vector<std::string> tokens;
        std::string current;
        bool inString = false;
        char stringChar = 0;

        for (size_t i = 0; i < sql.size(); i++) {
            char c = sql[i];

            if (inString) {
                current += c;
                if (c == stringChar) {
                    inString = false;
                    tokens.push_back(current);
                    current.clear();
                }
                continue;
            }

            if (c == '\'' || c == '"') {
                if (!current.empty()) { tokens.push_back(current); current.clear(); }
                inString = true;
                stringChar = c;
                current += c;
                continue;
            }

            if (std::isspace((unsigned char)c)) {
                if (!current.empty()) { tokens.push_back(current); current.clear(); }
                continue;
            }

            if (c == ',' || c == '(' || c == ')') {
                if (!current.empty()) { tokens.push_back(current); current.clear(); }
                tokens.push_back(std::string(1, c));
                continue;
            }

            current += c;
        }
        if (!current.empty()) tokens.push_back(current);

        return tokens;
    }

    std::string Format(const std::string& sql) {
        auto tokens = Tokenize(sql);
        std::ostringstream result;
        int indentLevel = 0;
        bool firstToken = true;
        bool atLineStart = true;

        // Список колонок после SELECT: помним, на каком уровне вложенности он начался
        bool selectListActive = false;
        int selectListDepth = 0;

        // Стек уровней отступа, на которые нужно вернуться при END (для вложенных CASE)
        std::vector<int> caseIndentStack;

        auto writeIndent = [&](int level) {
            result << "\n";
            for (int j = 0; j < level; j++) result << "    ";
            };

        for (size_t i = 0; i < tokens.size(); i++) {
            std::string token = tokens[i];
            std::string upperToken = ToUpper(token);

            if (token == "(") {
                result << "(";
                indentLevel++;
                firstToken = false;
                atLineStart = false;
                continue;
            }
            if (token == ")") {
                indentLevel = std::max(0, indentLevel - 1);
                result << ")";
                firstToken = false;
                atLineStart = false;
                continue;
            }

            if (token == ",") {
                result << ",";
                // Внутри активного списка колонок SELECT разрываем строку на "своём" уровне вложенности
                if (selectListActive && indentLevel == selectListDepth) {
                    writeIndent(indentLevel + 1);
                    atLineStart = true;
                }
                continue;
            }

            // --- CASE / WHEN / THEN / ELSE / END ---
            if (upperToken == "CASE") {
                caseIndentStack.push_back(indentLevel);
                if (!atLineStart) result << " ";
                result << "CASE";
                firstToken = false;
                atLineStart = false;
                continue;
            }
            if (upperToken == "WHEN" || upperToken == "ELSE") {
                int level = caseIndentStack.empty() ? indentLevel : caseIndentStack.back() + 1;
                writeIndent(level);
                result << upperToken;
                atLineStart = false;
                continue;
            }
            if (upperToken == "THEN") {
                result << " THEN";
                atLineStart = false;
                continue;
            }
            if (upperToken == "END") {
                int level = caseIndentStack.empty() ? indentLevel : caseIndentStack.back();
                writeIndent(level);
                result << "END";
                if (!caseIndentStack.empty()) caseIndentStack.pop_back();
                atLineStart = false;
                continue;
            }

            // --- обычные ключевые слова, начинающие новую строку ---
            bool startsNewline = kNewlineKeywords.count(upperToken) > 0 && !firstToken;

            if (upperToken == "SELECT") {
                selectListActive = true;
                selectListDepth = indentLevel;
            }
            else if (selectListActive && indentLevel == selectListDepth &&
                kNewlineKeywords.count(upperToken) > 0 && upperToken != "SELECT") {
                // Любое следующее ключевое слово (FROM, WHERE и т.д.) завершает список колонок
                selectListActive = false;
            }

            if (startsNewline) {
                writeIndent(indentLevel);
                atLineStart = true;
            }
            else if (!firstToken && !atLineStart) {
                result << " ";
            }

            if (kNewlineKeywords.count(upperToken) > 0 || kCapitalizeOnly.count(upperToken) > 0) {
                result << upperToken;
            }
            else {
                result << token;
            }

            // Если только что вывели SELECT — следующая колонка должна начаться с новой строки
            if (upperToken == "SELECT") {
                writeIndent(indentLevel + 1);
                atLineStart = true;
            }
            else {
                atLineStart = false;
            }

            firstToken = false;
        }

        return result.str();
    }
}

std::string SqlTool::Execute(const std::string& action, const std::string& payload) {
    if (action != "format") {
        throw std::runtime_error("Unknown action: " + action);
    }

    if (payload.empty()) {
        throw std::runtime_error("Empty SQL query");
    }

    return Format(payload);
}