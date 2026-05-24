#include "json.h"

void Json::skip_ws(const std::string& str, size_t& pos) {
    while (pos < str.size() && (str[pos] == ' ' || str[pos] == '\t' || str[pos] == '\n' || str[pos] == '\r'))
        pos++;
}

std::string Json::parse_string(const std::string& str, size_t& pos) {
    skip_ws(str, pos);
    if (pos >= str.size() || str[pos] != '"')
        throw std::runtime_error("expected string");
    pos++;
    std::string result;
    while (pos < str.size() && str[pos] != '"') {
        if (str[pos] == '\\') {
            pos++;
            if (pos >= str.size()) throw std::runtime_error("unterminated string escape");
            switch (str[pos]) {
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                case '/': result += '/'; break;
                case 'b': result += '\b'; break;
                case 'f': result += '\f'; break;
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                case 'u': {
                    if (pos + 4 >= str.size()) throw std::runtime_error("invalid unicode escape");
                    std::string hex = str.substr(pos + 1, 4);
                    unsigned int cp = std::stoul(hex, nullptr, 16);
                    if (cp <= 0x7F) {
                        result += static_cast<char>(cp);
                    } else if (cp <= 0x7FF) {
                        result += static_cast<char>(0xC0 | (cp >> 6));
                        result += static_cast<char>(0x80 | (cp & 0x3F));
                    } else {
                        result += static_cast<char>(0xE0 | (cp >> 12));
                        result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                        result += static_cast<char>(0x80 | (cp & 0x3F));
                    }
                    pos += 4;
                    break;
                }
                default: result += str[pos]; break;
            }
        } else {
            result += str[pos];
        }
        pos++;
    }
    if (pos >= str.size()) throw std::runtime_error("unterminated string");
    pos++;
    return result;
}

double Json::parse_number(const std::string& str, size_t& pos) {
    skip_ws(str, pos);
    size_t start = pos;
    if (pos < str.size() && str[pos] == '-') pos++;
    while (pos < str.size() && str[pos] >= '0' && str[pos] <= '9') pos++;
    if (pos < str.size() && str[pos] == '.') {
        pos++;
        while (pos < str.size() && str[pos] >= '0' && str[pos] <= '9') pos++;
    }
    if (pos < str.size() && (str[pos] == 'e' || str[pos] == 'E')) {
        pos++;
        if (pos < str.size() && (str[pos] == '+' || str[pos] == '-')) pos++;
        while (pos < str.size() && str[pos] >= '0' && str[pos] <= '9') pos++;
    }
    return std::stod(str.substr(start, pos - start));
}

Json Json::parse_value(const std::string& str, size_t& pos) {
    skip_ws(str, pos);
    if (pos >= str.size()) throw std::runtime_error("unexpected end of json");

    if (str[pos] == '"') {
        return Json(parse_string(str, pos));
    } else if (str[pos] == '{') {
        pos++;
        Json obj;
        obj.data = JsonObject();
        auto& map = std::get<JsonObject>(obj.data);
        skip_ws(str, pos);
        if (pos < str.size() && str[pos] != '}') {
            while (true) {
                skip_ws(str, pos);
                std::string key = parse_string(str, pos);
                skip_ws(str, pos);
                if (pos >= str.size() || str[pos] != ':') throw std::runtime_error("expected ':'");
                pos++;
                map[key] = parse_value(str, pos);
                skip_ws(str, pos);
                if (pos < str.size() && str[pos] == ',') { pos++; continue; }
                break;
            }
        }
        skip_ws(str, pos);
        if (pos >= str.size() || str[pos] != '}') throw std::runtime_error("expected '}'");
        pos++;
        return obj;
    } else if (str[pos] == '[') {
        pos++;
        Json arr;
        arr.data = JsonArray();
        auto& vec = std::get<JsonArray>(arr.data);
        skip_ws(str, pos);
        if (pos < str.size() && str[pos] != ']') {
            while (true) {
                vec.push_back(parse_value(str, pos));
                skip_ws(str, pos);
                if (pos < str.size() && str[pos] == ',') { pos++; continue; }
                break;
            }
        }
        skip_ws(str, pos);
        if (pos >= str.size() || str[pos] != ']') throw std::runtime_error("expected ']'");
        pos++;
        return arr;
    } else if (str.substr(pos, 4) == "true") { pos += 4; return Json(true); }
    else if (str.substr(pos, 5) == "false") { pos += 5; return Json(false); }
    else if (str.substr(pos, 4) == "null") { pos += 4; return Json(); }
    else if (str[pos] == '-' || (str[pos] >= '0' && str[pos] <= '9')) {
        return Json(parse_number(str, pos));
    } else {
        throw std::runtime_error(std::string("unexpected character: ") + str[pos]);
    }
}

Json Json::parse(const std::string& str) {
    size_t pos = 0;
    return parse_value(str, pos);
}

std::string Json::escape_string(const std::string& s) {
    std::string result;
    for (char c : s) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    result += buf;
                } else {
                    result += c;
                }
        }
    }
    return result;
}

void Json::dump_internal(std::ostringstream& os, int indent, int current_indent) const {
    switch (type()) {
        case Null: os << "null"; break;
        case Bool: os << (as_bool() ? "true" : "false"); break;
        case Number: {
            double v = as_number();
            if (v == std::floor(v) && std::isfinite(v) && std::abs(v) < 1e15)
                os << static_cast<int64_t>(v);
            else
                os << std::setprecision(10) << v;
            break;
        }
        case String: os << '"' << escape_string(as_string()) << '"'; break;
        case Array: {
            auto& arr = std::get<JsonArray>(data);
            if (arr.empty()) { os << "[]"; }
            else if (indent < 0) {
                os << "[";
                for (size_t i = 0; i < arr.size(); i++) {
                    if (i > 0) os << ",";
                    arr[i].dump_internal(os, indent, current_indent);
                }
                os << "]";
            } else {
                os << "[\n";
                int ni = current_indent + indent;
                for (size_t i = 0; i < arr.size(); i++) {
                    os << std::string(ni, ' ');
                    arr[i].dump_internal(os, indent, ni);
                    if (i + 1 < arr.size()) os << ",";
                    os << "\n";
                }
                os << std::string(current_indent, ' ') << "]";
            }
            break;
        }
        case Object: {
            auto& map = std::get<JsonObject>(data);
            if (map.empty()) { os << "{}"; }
            else if (indent < 0) {
                os << "{";
                bool first = true;
                for (auto& [k, v] : map) {
                    if (!first) os << ",";
                    first = false;
                    os << '"' << escape_string(k) << "\":";
                    v.dump_internal(os, indent, current_indent);
                }
                os << "}";
            } else {
                os << "{\n";
                int ni = current_indent + indent;
                bool first = true;
                for (auto& [k, v] : map) {
                    if (!first) os << ",\n";
                    first = false;
                    os << std::string(ni, ' ') << '"' << escape_string(k) << "\": ";
                    v.dump_internal(os, indent, ni);
                }
                os << "\n" << std::string(current_indent, ' ') << "}";
            }
            break;
        }
    }
}

std::string Json::dump(int indent) const {
    std::ostringstream os;
    dump_internal(os, indent, 0);
    return os.str();
}
