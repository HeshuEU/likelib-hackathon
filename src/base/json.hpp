#pragma once

#include "types.hpp"

#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace base
{
namespace json
{

// Various forward declarations.
namespace details
{
class _Value;
class _Number;
class _Null;
class _Boolean;
class _String;
class _Object;
class _Array;
template<typename CharType>
class JSON_Parser;

extern bool g_keep_json_object_unsorted;
}

class Number;
class Array;
class Object;

class Value
{
  public:
    enum ValueType
    {
        Number,
        Boolean,
        String,
        Object,
        Array,
        Null
    };

    Value();

    Value(int32_t value);

    Value(uint32_t value);

    Value(int64_t value);

    Value(uint64_t value);

    Value(double value);

    explicit Value(bool value);

    explicit Value(std::string value);

    explicit Value(std::string value, bool has_escape_chars);

    explicit Value(const char* value);

    explicit Value(const char* value, bool has_escape_chars);

    Value(const Value&);

    Value(Value&&) noexcept;

    Value& operator=(const Value&);

    Value& operator=(Value&&) noexcept;

    static Value null();

    static Value number(double value);

    static Value number(int32_t value);

    static Value number(uint32_t value);

    static Value number(int64_t value);

    static Value number(uint64_t value);

    static Value boolean(bool value);

    static Value string(std::string value);

    static Value string(std::string value, bool has_escape_chars);

    static Value object(bool keep_order = false);

    static Value object(std::vector<std::pair<std::string, Value>> fields, bool keep_order = false);

    static Value array();

    static Value array(size_t size);

    static Value array(std::vector<Value> elements);

    Value::ValueType type() const;

    bool is_null() const;

    bool is_number() const;

    bool is_integer() const;

    bool is_double() const;

    bool is_boolean() const;

    bool is_string() const;

    bool is_array() const;

    bool is_object() const;

    size_t size() const;

    static Value parse(const std::string& value);

    static Value parse(const std::string& value, std::error_code& errorCode);

    std::string serialize() const;

    static Value parse(std::istream& input);

    static Value parse(std::istream& input, std::error_code& errorCode);

    void serialize(std::ostream& stream) const;

    double as_double() const;

    int as_integer() const;

    const json::Number& as_number() const;

    bool as_bool() const;

    json::Array& as_array();

    const json::Array& as_array() const;

    json::Object& as_object();

    const json::Object& as_object() const;

    const std::string& as_string() const;

    bool operator==(const Value& other) const;

    bool operator!=(const Value& other) const;

    bool has_field(const std::string& key) const;

    bool has_number_field(const std::string& key) const;

    bool has_integer_field(const std::string& key) const;

    bool has_double_field(const std::string& key) const;

    bool has_boolean_field(const std::string& key) const;

    bool has_string_field(const std::string& key) const;

    bool has_array_field(const std::string& key) const;

    bool has_object_field(const std::string& key) const;

    void erase(size_t index);

    void erase(const std::string& key);

    Value& at(size_t index);

    const Value& at(size_t index) const;

    Value& at(const std::string& key);

    const Value& at(const std::string& key) const;

    Value& operator[](const std::string& key);

    Value& operator[](size_t index);

  private:
    friend class details::_Object;
    friend class details::_Array;
    template<typename CharType>
    friend class details::JSON_Parser;

    void format(std::basic_string<char>& string) const;

    explicit Value(std::unique_ptr<details::_Value> v);

    std::unique_ptr<details::_Value> _value;
};

class json_exception : public std::exception
{
  private:
    std::string _message;

  public:
    json_exception(const char* const message);

    json_exception(std::string&& message);

    // Must be narrow string because it derives from std::exception
    const char* what() const noexcept;
};

namespace details
{
enum json_error
{
    left_over_character_in_stream = 1,
    malformed_array_literal,
    malformed_comment,
    malformed_literal,
    malformed_object_literal,
    malformed_numeric_literal,
    malformed_string_literal,
    malformed_token,
    mismatched_brances,
    nesting,
    unexpected_token
};

class json_error_category_impl : public std::error_category
{
  public:
    virtual const char* name() const noexcept override;

    virtual std::string message(int ev) const override;
};

const json_error_category_impl& json_error_category();

} // namespace details


class Array
{
    using storage_type = std::vector<Value>;

  public:
    using iterator = storage_type::iterator;
    using const_iterator = storage_type::const_iterator;
    using reverse_iterator = storage_type::reverse_iterator;
    using const_reverse_iterator = storage_type::const_reverse_iterator;
    using size_type = storage_type::size_type;

  private:
    Array() = default;

    Array(size_type size);

    Array(storage_type elements);

  public:
    iterator begin();

    const_iterator begin() const;

    iterator end();

    const_iterator end() const;

    reverse_iterator rbegin();

    const_reverse_iterator rbegin() const;

    reverse_iterator rend();

    const_reverse_iterator rend() const;

    const_iterator cbegin() const;

    const_iterator cend() const;

    const_reverse_iterator crbegin() const;

    const_reverse_iterator crend() const;

    iterator erase(iterator position);

    void erase(size_type index);

    Value& at(size_type index);

    const Value& at(size_type index) const;

    Value& operator[](size_type index);

    size_type size() const;

  private:
    storage_type _elements;

    friend class details::_Array;

    template<typename CharType>
    friend class details::JSON_Parser;
};

class Object
{
    using storage_type = std::vector<std::pair<std::string, Value>>;

  public:
    using iterator = storage_type::iterator;
    using const_iterator = storage_type::const_iterator;
    using reverse_iterator = storage_type::reverse_iterator;
    using const_reverse_iterator = storage_type::const_reverse_iterator;
    using size_type = storage_type::size_type;

  private:
    Object(bool keep_order = false);

    Object(storage_type elements, bool keep_order = false);

  public:
    iterator begin();

    const_iterator begin() const;

    iterator end();

    const_iterator end() const;

    reverse_iterator rbegin();

    const_reverse_iterator rbegin() const;

    reverse_iterator rend();

    const_reverse_iterator rend() const;

    const_iterator cbegin() const;

    const_iterator cend() const;

    const_reverse_iterator crbegin() const;

    const_reverse_iterator crend() const;

    iterator erase(iterator position);

    void erase(const std::string& key);

    Value& at(const std::string& key);

    const Value& at(const std::string& key) const;

    Value& operator[](const std::string& key);

    const_iterator find(const std::string& key) const;

    std::size_t size() const;

    bool empty() const;

  private:
    static bool compare_pairs(const std::pair<std::string, Value>& p1, const std::pair<std::string, Value>& p2);

    static bool compare_with_key(const std::pair<std::string, Value>& p1, const std::string& key);

    storage_type::iterator find_insert_location(const std::string& key);

    storage_type::const_iterator find_by_key(const std::string& key) const;

    storage_type::iterator find_by_key(const std::string& key);

    storage_type _elements;
    bool _keep_order;
    friend class details::_Object;

    template<typename CharType>
    friend class details::JSON_Parser;
};


class Number
{

    Number(double val);

    Number(std::int32_t val);

    Number(std::uint32_t val);

    Number(std::int64_t val);

    Number(std::uint64_t val);

  public:
    bool is_int32() const;

    bool is_uint32() const;

    bool is_int64() const;

    bool is_uint64() const;

    double to_double() const;

    std::int32_t to_int32() const;

    std::uint32_t to_uint32() const;

    std::int64_t to_int64() const;

    std::uint64_t to_uint64() const;

    bool is_integral() const;

    bool operator==(const Number& other) const;

  private:
    union
    {
        std::int64_t _intval;
        std::uint64_t _uintval;
        double _value;
    };

    enum type
    {
        signed_type = 0,
        unsigned_type,
        double_type
    } _type;

    friend class details::_Number;
};

namespace details
{
class _Value
{
  public:
    virtual std::unique_ptr<_Value> _copy_value() = 0;

    virtual bool has_field(const std::string&) const;
    virtual Value get_field(const std::string&) const;
    virtual Value get_element(Array::size_type) const;

    virtual Value& index(const std::string&);
    virtual Value& index(Array::size_type);

    virtual const Value& cnst_index(const std::string&) const;
    virtual const Value& cnst_index(Array::size_type) const;

    // Common function used for serialization to strings and streams.
    virtual void serialize_impl(std::string& str) const;

    virtual std::string to_string() const;

    virtual Value::ValueType type() const;

    virtual bool is_integer() const;
    virtual bool is_double() const;

    virtual const Number& as_number();
    virtual double as_double() const;
    virtual int as_integer() const;
    virtual bool as_bool() const;
    virtual Array& as_array();
    virtual const Array& as_array() const;
    virtual Object& as_object();
    virtual const Object& as_object() const;
    virtual const std::string& as_string() const;

    virtual std::size_t size() const;

    virtual ~_Value() = default;

    _Value() = default;

    virtual void format(std::basic_string<char>& stream) const;

  private:
    friend class value;
};

class _Null : public _Value
{
  public:
    virtual std::unique_ptr<_Value> _copy_value();
    virtual Value::ValueType type() const;
};

class _Number : public _Value
{
  public:
    _Number(double value);

    _Number(std::int32_t value);

    _Number(std::uint32_t value);

    _Number(std::int64_t value);

    _Number(std::uint64_t value);

    virtual std::unique_ptr<_Value> _copy_value();

    virtual Value::ValueType type() const;

    virtual bool is_integer() const;
    virtual bool is_double() const;

    virtual double as_double() const;

    virtual int as_integer() const;

    virtual const Number& as_number();

  protected:
    virtual void format(std::basic_string<char>& stream) const;

  private:
    template<typename CharType>
    friend class details::JSON_Parser;

    Number _number;
};


class _Boolean : public _Value
{
  public:
    _Boolean(bool value);

    virtual std::unique_ptr<_Value> _copy_value();

    virtual Value::ValueType type() const;

    virtual bool as_bool() const;

  protected:
    virtual void format(std::basic_string<char>& stream) const;

  private:
    template<typename CharType>
    friend class details::JSON_Parser;
    bool _value;
};

class _String : public _Value
{
  public:
    _String(std::string value);

    _String(std::string value, bool escaped_chars);

    virtual std::unique_ptr<_Value> _copy_value();

    virtual Value::ValueType type() const;

    virtual const std::string& as_string() const;

    virtual void serialize_impl(std::string& str) const;

  protected:
    virtual void format(std::basic_string<char>& str) const;

  private:
    friend class _Object;
    friend class _Array;

    size_t get_reserve_size() const;

    template<typename CharType>
    void serialize_impl_char_type(std::basic_string<CharType>& str) const
    {
        // To avoid repeated allocations reserve some space all up front.
        // size of string + 2 for quotes
        str.reserve(get_reserve_size());
        format(str);
    }

    std::string _string;

    // There are significant performance gains that can be made by knowing whether
    // or not a character that requires escaping is present.
    bool _has_escape_char;
    static bool has_escape_chars(const _String& str);
};

template<typename CharType>
void append_escape_string(std::basic_string<CharType>& str, const std::basic_string<CharType>& escaped)
{
    for (const auto& ch : escaped) {
        switch (ch) {
            case '\"':
                str += '\\';
                str += '\"';
                break;
            case '\\':
                str += '\\';
                str += '\\';
                break;
            case '\b':
                str += '\\';
                str += 'b';
                break;
            case '\f':
                str += '\\';
                str += 'f';
                break;
            case '\r':
                str += '\\';
                str += 'r';
                break;
            case '\n':
                str += '\\';
                str += 'n';
                break;
            case '\t':
                str += '\\';
                str += 't';
                break;
            default:

                // If a control character then must unicode escaped.
                if (ch >= 0 && ch <= 0x1F) {
                    static const std::array<CharType, 16> intToHex = {
                        { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' }
                    };
                    str += '\\';
                    str += 'u';
                    str += '0';
                    str += '0';
                    str += intToHex[(ch & 0xF0) >> 4];
                    str += intToHex[ch & 0x0F];
                }
                else {
                    str += ch;
                }
        }
    }
}

void format_string(const std::string& key, std::string& str);


class _Object : public _Value
{
  public:
    _Object(bool keep_order);

    _Object(Object::storage_type fields, bool keep_order);

    virtual std::unique_ptr<_Value> _copy_value();

    virtual Object& as_object();

    virtual const Object& as_object() const;

    virtual Value::ValueType type() const;

    virtual bool has_field(const std::string&) const;

    virtual Value& index(const std::string& key);

    bool is_equal(const _Object* other) const;

    virtual void serialize_impl(std::string& str) const;

    std::size_t size() const;

  protected:
    virtual void format(std::basic_string<char>& str) const;

  private:
    Object _object;

    template<typename CharType>
    friend class details::JSON_Parser;

    template<typename CharType>
    void format_impl(std::basic_string<CharType>& str) const
    {
        str.push_back('{');
        if (!_object.empty()) {
            auto lastElement = _object.end() - 1;
            for (auto iter = _object.begin(); iter != lastElement; ++iter) {
                format_string(iter->first, str);
                str.push_back(':');
                iter->second.format(str);
                str.push_back(',');
            }
            format_string(lastElement->first, str);
            str.push_back(':');
            lastElement->second.format(str);
        }
        str.push_back('}');
    }

    std::size_t get_reserve_size() const;
};

class _Array : public _Value
{
  public:
    _Array() = default;

    _Array(Array::size_type size);

    _Array(Array::storage_type elements);

    virtual std::unique_ptr<_Value> _copy_value();

    virtual Value::ValueType type() const;

    virtual Array& as_array();
    virtual const Array& as_array() const;

    virtual Value& index(Array::size_type index);

    bool is_equal(const _Array* other) const;

    virtual void serialize_impl(std::string& str) const;

    std::size_t size() const;

  protected:
    virtual void format(std::basic_string<char>& str) const;

  private:
    Array _array;

    template<typename CharType>
    friend class details::JSON_Parser;

    template<typename CharType>
    void format_impl(std::basic_string<CharType>& str) const
    {
        str.push_back('[');
        if (!_array._elements.empty()) {
            auto lastElement = _array._elements.end() - 1;
            for (auto iter = _array._elements.begin(); iter != lastElement; ++iter) {
                iter->format(str);
                str.push_back(',');
            }
            lastElement->format(str);
        }
        str.push_back(']');
    }

    std::size_t get_reserve_size() const;
};
} // namespace details

} // namespace json

std::ostream& operator<<(std::ostream& os, const json::Value& val);

std::istream& operator>>(std::istream& is, json::Value& val);

} // namespace base
