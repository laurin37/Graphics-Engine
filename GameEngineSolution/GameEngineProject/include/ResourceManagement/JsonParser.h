#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <stdexcept>

// ========================================
// Lightweight JSON Parser
// No external dependencies, custom implementation
// ========================================

enum class JsonType {
    Null,
    Bool,
    Number,
    String,
    Array,
    Object
};

class JsonValue {
public:
    // Constructors
    JsonValue(); // Null
    explicit JsonValue(bool value);
    explicit JsonValue(double value);
    explicit JsonValue(const std::string& value);
    explicit JsonValue(std::vector<JsonValue>&& value);
    explicit JsonValue(std::unordered_map<std::string, JsonValue>&& value);

    // Type queries
    JsonType GetType() const { return m_type; }
    bool IsNull() const { return m_type == JsonType::Null; }
    bool IsBool() const { return m_type == JsonType::Bool; }
    bool IsNumber() const { return m_type == JsonType::Number; }
    bool IsString() const { return m_type == JsonType::String; }
    bool IsArray() const { return m_type == JsonType::Array; }
    bool IsObject() const { return m_type == JsonType::Object; }

    // Getters (throw std::runtime_error on type mismatch)
    bool AsBool() const;
    double AsNumber() const;
    const std::string& AsString() const;
    const std::vector<JsonValue>& AsArray() const;

    // Object field access
    bool HasField(const std::string& key) const;
    const JsonValue& GetField(const std::string& key) const;
    JsonValue& GetField(const std::string& key);

    // Array element access
    size_t ArraySize() const;
    const JsonValue& operator[](size_t index) const;

private:
    JsonType m_type;
    
    // Storage (union-like approach using pointers to avoid complex destructors)
    bool m_boolValue;
    double m_numberValue;
    std::unique_ptr<std::string> m_stringValue;
    std::unique_ptr<std::vector<JsonValue>> m_arrayValue;
    std::unique_ptr<std::unordered_map<std::string, JsonValue>> m_objectValue;
};

class JsonParser {
public:
    // Parse JSON from string
    static JsonValue Parse(const std::string& jsonText);
    
    // Parse JSON from file (Win32 API, UTF-8 encoding)
    static JsonValue ParseFile(const std::wstring& filePath);

private:
    JsonParser(const std::string& text);
    
    // Main parsing entry point
    JsonValue ParseValue();
    
    // Parse specific types
    JsonValue ParseObject();
    JsonValue ParseArray();
    JsonValue ParseString();
    JsonValue ParseNumber();
    JsonValue ParseKeyword(); // true, false, null
    
    // Tokenizer helpers
    void SkipWhitespace();
    char Peek() const;
    char Consume();
    bool Match(char expected);
    void Expect(char expected);
    bool IsDigit(char c) const;
    bool IsWhitespace(char c) const;
    
    // Error handling
    [[noreturn]] void Error(const std::string& message) const;
    
    const std::string& m_text;
    size_t m_pos;
};
