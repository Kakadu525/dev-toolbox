#include "XmlTool.h"
#include <pugixml.hpp>
#include <sstream>
#include <stdexcept>

std::string XmlTool::Execute(const std::string& action, const std::string& payload) {
    pugi::xml_document doc;
    pugi::xml_parse_result parseResult = doc.load_string(payload.c_str());

    if (!parseResult) {
        // pugixml даёт понятное описание ошибки и позицию символа, где всё сломалось
        throw std::runtime_error(std::string("Invalid XML: ") + parseResult.description());
    }

    std::ostringstream stream;

    if (action == "pretty") {
        // format_indent — расставляет отступы, indent(4 пробела) задаём отдельно
        doc.save(stream, "    ", pugi::format_indent);
    }
    else if (action == "minify") {
        // format_raw — без отступов и переносов строк между тегами
        doc.save(stream, "", pugi::format_raw);
    }
    else {
        throw std::runtime_error("Unknown action: " + action);
    }

    return stream.str();
}