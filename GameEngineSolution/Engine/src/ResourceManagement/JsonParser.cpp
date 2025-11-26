#include "../../include/ResourceManagement/JsonParser.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <Windows.h>

// ========================================
// JsonValue Implementation
// ========================================

JsonValue::JsonValue() : m_type(JsonType::Null), m_boolValue(false), m_numberValue(0.0) {}

JsonValue::JsonValue(bool value) : m_type(JsonType::Bool), m_boolValue(value), m_numberValue(0.0) {}

JsonValue::JsonValue(double value) : m_type(JsonType::Number), m_boolValue(false), m_numberValue(value) {}

JsonValue::JsonValue(const std::string& value) 
    : m_type(JsonType::String), m_boolValue(false), m_numberValue(0.0),
      m_stringValue(std::make_unique<std::string>(value)) {}

JsonValue::JsonValue(std::vector<JsonValue>&& value)
    : m_type(JsonType::Array), m_boolValue(false), m_numberValue(0.0),
      m_arrayValue(std::make_unique<std::vector<JsonValue>>(std::move(value))) {}

JsonValue::JsonValue(std::unordered_map<std::string, JsonValue>&& value)
    : m_type(JsonType::Object), m_boolValue(false), m_numberValue(0.0),
      m_objectValue(std::make_unique<std::unordered_map<std::string, JsonValue>>(std::move(value))) {}

bool JsonValue::AsBool() const {
    if (m_type != JsonType::Bool) throw std::runtime_error("JsonValue: expected Bool");
    return m_boolValue;
}

double JsonValue::AsNumber() const {
    if (m_type != JsonType::Number) throw std::runtime_error("JsonValue: expected Number");
    return m_numberValue;
}

const std::string& JsonValue::AsString() const {
    if (m_type != JsonType::String) throw std::runtime_error("JsonValue: expected String");
    return *m_stringValue;
}

const std::vector<JsonValue>& JsonValue::AsArray() const {
    if (m_type != JsonType::Array) throw std::runtime_error("JsonValue: expected Array");
    return *m_arrayValue;
}

bool JsonValue::HasField(const std::string& key) const {
    if (m_type != JsonType::Object) return false;
    return m_objectValue->find(key) != m_objectValue->end();
}

const JsonValue& JsonValue::GetField(const std::string& key) const {
    if (m_type != JsonType::Object) throw std::runtime_error("JsonValue: expected Object");
    auto it = m_objectValue->find(key);
    if (it == m_objectValue->end()) throw std::runtime_error("JsonValue: field not found: " + key);
    return it->second;
}

JsonValue& JsonValue::GetField(const std::string& key) {
    if (m_type != JsonType::Object) throw std::runtime_error("JsonValue: expected Object");
    auto it = m_objectValue->find(key);
    if (it == m_objectValue->end()) throw std::runtime_error("JsonValue: field not found: " + key);
    return it->second;
}

std::vector<std::string> JsonValue::GetMemberNames() const {
    if (m_type != JsonType::Object) throw std::runtime_error("JsonValue: expected Object");
    std::vector<std::string> names;
    names.reserve(m_objectValue->size());
    for (const auto& pair : *m_objectValue) {
        names.push_back(pair.first);
    }
    return names;
}

size_t JsonValue::ArraySize() const {
    if (m_type != JsonType::Array) throw std::runtime_error("JsonValue: expected Array");
    return m_arrayValue->size();
}

const JsonValue& JsonValue::operator[](size_t index) const {
    if (m_type != JsonType::Array) throw std::runtime_error("JsonValue: expected Array");
    if (index >= m_arrayValue->size()) throw std::runtime_error("JsonValue: array index out of bounds");
    return (*m_arrayValue)[index];
}

// ========================================
// JsonParser Implementation
// ========================================

JsonValue JsonParser::Parse(const std::string& jsonText) {
    JsonParser parser(jsonText);
    return parser.ParseValue();
}

JsonValue JsonParser::ParseFile(const std::wstring& filePath) {
    // Open file using Win32 API
    HANDLE hFile = CreateFileW(
        filePath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to open JSON file");
    }

    // Get file size
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        CloseHandle(hFile);
        throw std::runtime_error("Failed to get file size");
    }

    // Read file content
    std::string content;
    content.resize(static_cast<size_t>(fileSize.QuadPart));
    
    DWORD bytesRead = 0;
    if (!ReadFile(hFile, &content[0], static_cast<DWORD>(fileSize.QuadPart), &bytesRead, nullptr)) {
        CloseHandle(hFile);
        throw std::runtime_error("Failed to read JSON file");
    }

    CloseHandle(hFile);

    // Parse the content
    return Parse(content);
}

JsonParser::JsonParser(const std::string& text) : m_text(text), m_pos(0) {}

JsonValue JsonParser::ParseValue() {
    SkipWhitespace();
    
    char c = Peek();
    
    if (c == '{') return ParseObject();
    if (c == '[') return ParseArray();
    if (c == '"') return ParseString();
    if (c == '-' || IsDigit(c)) return ParseNumber();
    if (c == 't' || c == 'f' || c == 'n') return ParseKeyword();
    
    Error("Unexpected character");
}

JsonValue JsonParser::ParseObject() {
    Expect('{');
    SkipWhitespace();
    
    std::unordered_map<std::string, JsonValue> object;
    
    // Empty object
    if (Peek() == '}') {
        Consume();
        return JsonValue(std::move(object));
    }
    
    while (true) {
        SkipWhitespace();
        
        // Parse key (must be string)
        if (Peek() != '"') Error("Expected string key in object");
        std::string key = ParseString().AsString();
        
        SkipWhitespace();
        Expect(':');
        SkipWhitespace();
        
        // Parse value
        JsonValue value = ParseValue();
        object[key] = std::move(value);
        
        SkipWhitespace();
        
        if (Peek() == '}') {
            Consume();
            break;
        }
        
        Expect(',');
    }
    
    return JsonValue(std::move(object));
}

JsonValue JsonParser::ParseArray() {
    Expect('[');
    SkipWhitespace();
    
    std::vector<JsonValue> array;
    
    // Empty array
    if (Peek() == ']') {
        Consume();
        return JsonValue(std::move(array));
    }
    
    while (true) {
        SkipWhitespace();
        array.push_back(ParseValue());
        SkipWhitespace();
        
        if (Peek() == ']') {
            Consume();
            break;
        }
        
        Expect(',');
    }
    
    return JsonValue(std::move(array));
}

JsonValue JsonParser::ParseString() {
    Expect('"');
    
    std::string result;
    
    while (Peek() != '"') {
        char c = Consume();
        
        // Handle escape sequences
        if (c == '\\') {
            char escaped = Consume();
            switch (escaped) {
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                case '/': result += '/'; break;
                case 'b': result += '\b'; break;
                case 'f': result += '\f'; break;
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                default: Error("Invalid escape sequence");
            }
        } else {
            result += c;
        }
    }
    
    Expect('"');
    return JsonValue(result);
}

JsonValue JsonParser::ParseNumber() {
    std::string numStr;
    
    // Optional negative sign
    if (Peek() == '-') {
        numStr += Consume();
    }
    
    // Integer part
    if (Peek() == '0') {
        numStr += Consume();
    } else {
        if (!IsDigit(Peek())) Error("Invalid number");
        while (IsDigit(Peek())) {
            numStr += Consume();
        }
    }
    
    // Fractional part
    if (Peek() == '.') {
        numStr += Consume();
        if (!IsDigit(Peek())) Error("Invalid number: expected digit after decimal point");
        while (IsDigit(Peek())) {
            numStr += Consume();
        }
    }
    
    // Exponent
    if (Peek() == 'e' || Peek() == 'E') {
        numStr += Consume();
        if (Peek() == '+' || Peek() == '-') {
            numStr += Consume();
        }
        if (!IsDigit(Peek())) Error("Invalid number: expected digit in exponent");
        while (IsDigit(Peek())) {
            numStr += Consume();
        }
    }
    
    return JsonValue(std::stod(numStr));
}

JsonValue JsonParser::ParseKeyword() {
    char c = Peek();
    
    if (c == 't') {
        // true
        if (m_text.substr(m_pos, 4) == "true") {
            m_pos += 4;
            return JsonValue(true);
        }
        Error("Invalid keyword");
    }
    
    if (c == 'f') {
        // false
        if (m_text.substr(m_pos, 5) == "false") {
            m_pos += 5;
            return JsonValue(false);
        }
        Error("Invalid keyword");
    }
    
    if (c == 'n') {
        // null
        if (m_text.substr(m_pos, 4) == "null") {
            m_pos += 4;
            return JsonValue();
        }
        Error("Invalid keyword");
    }
    
    Error("Invalid keyword");
}

void JsonParser::SkipWhitespace() {
    while (m_pos < m_text.size() && IsWhitespace(m_text[m_pos])) {
        m_pos++;
    }
}

char JsonParser::Peek() const {
    if (m_pos >= m_text.size()) return '\0';
    return m_text[m_pos];
}

char JsonParser::Consume() {
    if (m_pos >= m_text.size()) Error("Unexpected end of input");
    return m_text[m_pos++];
}

bool JsonParser::Match(char expected) {
    if (Peek() == expected) {
        Consume();
        return true;
    }
    return false;
}

void JsonParser::Expect(char expected) {
    if (Peek() != expected) {
        std::string msg = "Expected '";
        msg += expected;
        msg += "'";
        Error(msg);
    }
    Consume();
}

bool JsonParser::IsDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool JsonParser::IsWhitespace(char c) const {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

void JsonParser::Error(const std::string& message) const {
    std::string error = "JSON Parse Error at position ";
    error += std::to_string(m_pos);
    error += ": ";
    error += message;
    throw std::runtime_error(error);
}
