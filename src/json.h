#pragma once
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <cstdint>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

class Json {
public:
    enum Type { Null, Bool, Number, String, Array, Object };

private:
    struct JsonArray : std::vector<Json> { using std::vector<Json>::vector; };
    struct JsonObject : std::map<std::string, Json> { using std::map<std::string, Json>::map; };

    std::variant<std::monostate, bool, double, std::string, JsonArray, JsonObject> data;

public:
    Json() : data(std::monostate{}) {}
    Json(bool b) : data(b) {}
    Json(int v) : data(static_cast<double>(v)) {}
    Json(long v) : data(static_cast<double>(v)) {}
    Json(long long v) : data(static_cast<double>(v)) {}
    Json(unsigned int v) : data(static_cast<double>(v)) {}
    Json(unsigned long v) : data(static_cast<double>(v)) {}
    Json(unsigned long long v) : data(static_cast<double>(v)) {}
    Json(float v) : data(static_cast<double>(v)) {}
    Json(double v) : data(v) {}
    Json(const char* s) : data(std::string(s)) {}
    Json(const std::string& s) : data(s) {}

    Type type() const { return static_cast<Type>(data.index()); }
    bool is_null() const { return data.index() == 0; }
    bool is_bool() const { return data.index() == 1; }
    bool is_number() const { return data.index() == 2; }
    bool is_string() const { return data.index() == 3; }
    bool is_array() const { return data.index() == 4; }
    bool is_object() const { return data.index() == 5; }

    bool as_bool() const {
        if (!is_bool()) throw std::runtime_error("not a bool");
        return std::get<bool>(data);
    }

    double as_number() const {
        if (!is_number()) throw std::runtime_error("not a number");
        return std::get<double>(data);
    }

    int64_t as_int() const { return static_cast<int64_t>(as_number()); }

    const std::string& as_string() const {
        if (!is_string()) throw std::runtime_error("not a string");
        return std::get<std::string>(data);
    }

    Json& operator[](const std::string& key) {
        if (is_null()) data = JsonObject();
        if (!is_object()) throw std::runtime_error("not an object");
        return std::get<JsonObject>(data)[key];
    }

    const Json& operator[](const std::string& key) const {
        if (!is_object()) throw std::runtime_error("not an object");
        auto& obj = std::get<JsonObject>(data);
        auto it = obj.find(key);
        if (it == obj.end()) throw std::runtime_error("key not found: " + key);
        return it->second;
    }

    Json& operator[](size_t idx) {
        if (is_null()) data = JsonArray();
        if (!is_array()) throw std::runtime_error("not an array");
        auto& arr = std::get<JsonArray>(data);
        if (idx >= arr.size()) arr.resize(idx + 1);
        return arr[idx];
    }

    const Json& operator[](size_t idx) const {
        if (!is_array()) throw std::runtime_error("not an array");
        return std::get<JsonArray>(data).at(idx);
    }

    size_t size() const {
        if (is_array()) return std::get<JsonArray>(data).size();
        if (is_object()) return std::get<JsonObject>(data).size();
        return 0;
    }

    void push_back(const Json& val) {
        if (is_null()) data = JsonArray();
        if (!is_array()) throw std::runtime_error("not an array");
        std::get<JsonArray>(data).push_back(val);
    }

    bool has(const std::string& key) const {
        if (!is_object()) return false;
        auto& obj = std::get<JsonObject>(data);
        return obj.find(key) != obj.end();
    }

    using iterator = std::vector<Json>::iterator;
    using const_iterator = std::vector<Json>::const_iterator;

    iterator begin() {
        if (!is_array()) throw std::runtime_error("not an array");
        return std::get<JsonArray>(data).begin();
    }
    iterator end() {
        if (!is_array()) throw std::runtime_error("not an array");
        return std::get<JsonArray>(data).end();
    }
    const_iterator begin() const {
        if (!is_array()) throw std::runtime_error("not an array");
        return std::get<JsonArray>(data).begin();
    }
    const_iterator end() const {
        if (!is_array()) throw std::runtime_error("not an array");
        return std::get<JsonArray>(data).end();
    }

    using o_iterator = std::map<std::string, Json>::iterator;
    using co_iterator = std::map<std::string, Json>::const_iterator;

    o_iterator obj_begin() {
        if (!is_object()) throw std::runtime_error("not an object");
        return std::get<JsonObject>(data).begin();
    }
    o_iterator obj_end() {
        if (!is_object()) throw std::runtime_error("not an object");
        return std::get<JsonObject>(data).end();
    }
    co_iterator obj_begin() const {
        if (!is_object()) throw std::runtime_error("not an object");
        return std::get<JsonObject>(data).begin();
    }
    co_iterator obj_end() const {
        if (!is_object()) throw std::runtime_error("not an object");
        return std::get<JsonObject>(data).end();
    }

    std::string dump(int indent = 0) const;
    static Json parse(const std::string& str);

private:
    static Json parse_value(const std::string& str, size_t& pos);
    static std::string parse_string(const std::string& str, size_t& pos);
    static double parse_number(const std::string& str, size_t& pos);
    static void skip_ws(const std::string& str, size_t& pos);
    static std::string escape_string(const std::string& s);
    void dump_internal(std::ostringstream& os, int indent, int current_indent) const;
};
