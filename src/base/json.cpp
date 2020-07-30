#include "json.hpp"
#include "assert.hpp"

#include <climits>
#define __STDC_FORMAT_MACROS
#include <cinttypes>

std::array<signed char, 128> _hexval = {
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,  1,  2,  3,
      4,  5,  6,  7,  8,  9,  -1, -1, -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }
};

namespace base
{
namespace json
{

bool details::g_keep_json_object_unsorted = false;

void keep_object_element_order(bool keep_order)
{
    json::details::g_keep_json_object_unsorted = keep_order;
}

namespace details
{

template<typename Token>
__attribute__((noreturn)) void CreateException(const Token& tk, const std::string& message)
{
    std::string str("* Line ");
    str += std::to_string(tk.start.m_line);
    str += ", Column ";
    str += std::to_string(tk.start.m_column);
    str += " Syntax error: ";
    str += message;
    throw json::json_exception(std::move(str));
}

template<typename Token>
void SetErrorCode(Token& tk, json_error jsonErrorCode)
{
    tk.m_error = std::error_code(jsonErrorCode, json_error_category());
}

template<typename CharType>
class JSON_Parser
{
  public:
    JSON_Parser()
      : m_currentLine(1)
      , m_currentColumn(1)
      , m_currentParsingDepth(0)
    {}

    struct Location
    {
        size_t m_line;
        size_t m_column;
    };

    struct Token
    {
        enum Kind
        {
            TKN_EOF,

            TKN_OpenBrace,
            TKN_CloseBrace,
            TKN_OpenBracket,
            TKN_CloseBracket,
            TKN_Comma,
            TKN_Colon,
            TKN_StringLiteral,
            TKN_NumberLiteral,
            TKN_IntegerLiteral,
            TKN_BooleanLiteral,
            TKN_NullLiteral,
            TKN_Comment
        };

        Token()
          : kind(TKN_EOF)
        {}

        Kind kind;
        std::basic_string<CharType> string_val;

        typename JSON_Parser<CharType>::Location start;

        union
        {
            double double_val;
            int64_t int64_val;
            uint64_t uint64_val;
            bool boolean_val;
            bool has_unescape_symbol;
        };

        bool signed_number;

        std::error_code m_error;
    };

    void GetNextToken(Token&);

    json::Value ParseValue(typename JSON_Parser<CharType>::Token& first) { return json::Value(_ParseValue(first)); }

  protected:
    typedef typename std::char_traits<CharType>::int_type int_type;
    virtual int_type NextCharacter() = 0;
    virtual int_type PeekCharacter() = 0;

    virtual bool CompleteComment(Token& token);
    virtual bool CompleteStringLiteral(Token& token);
    int convert_unicode_to_code_point();
    bool handle_unescape_char(Token& token);

  private:
    bool CompleteNumberLiteral(CharType first, Token& token);
    bool ParseInt64(CharType first, uint64_t& value);
    bool CompleteKeywordTrue(Token& token);
    bool CompleteKeywordFalse(Token& token);
    bool CompleteKeywordNull(Token& token);
    std::unique_ptr<json::details::_Value> _ParseValue(typename JSON_Parser<CharType>::Token& first);
    std::unique_ptr<json::details::_Value> _ParseObject(typename JSON_Parser<CharType>::Token& tkn);
    std::unique_ptr<json::details::_Value> _ParseArray(typename JSON_Parser<CharType>::Token& tkn);

    JSON_Parser& operator=(const JSON_Parser&);

    int_type EatWhitespace();

    void CreateToken(typename JSON_Parser<CharType>::Token& tk, typename Token::Kind kind, Location& start)
    {
        tk.kind = kind;
        tk.start = start;
        tk.string_val.clear();
    }

    void CreateToken(typename JSON_Parser<CharType>::Token& tk, typename Token::Kind kind)
    {
        tk.kind = kind;
        tk.start.m_line = m_currentLine;
        tk.start.m_column = m_currentColumn;
        tk.string_val.clear();
    }

  protected:
    size_t m_currentLine;
    size_t m_currentColumn;
    size_t m_currentParsingDepth;


    static const size_t maxParsingDepth = 128;
};
// Replace with template alias once VS 2012 support is removed.
template<typename CharType>
typename std::char_traits<CharType>::int_type eof()
{
    return std::char_traits<CharType>::eof();
}

template<typename CharType>
class JSON_StreamParser : public JSON_Parser<CharType>
{
  public:
    JSON_StreamParser(std::basic_istream<CharType>& stream)
      : m_streambuf(stream.rdbuf())
    {}

  protected:
    virtual typename JSON_Parser<CharType>::int_type NextCharacter();
    virtual typename JSON_Parser<CharType>::int_type PeekCharacter();

  private:
    typename std::basic_streambuf<CharType, std::char_traits<CharType>>* m_streambuf;
};

template<typename CharType>
class JSON_StringParser : public JSON_Parser<CharType>
{
  public:
    JSON_StringParser(const std::basic_string<CharType>& string)
      : m_position(&string[0])
    {
        m_startpos = m_position;
        m_endpos = m_position + string.size();
    }

  protected:
    virtual typename JSON_Parser<CharType>::int_type NextCharacter();
    virtual typename JSON_Parser<CharType>::int_type PeekCharacter();

    virtual bool CompleteComment(typename JSON_Parser<CharType>::Token& token);
    virtual bool CompleteStringLiteral(typename JSON_Parser<CharType>::Token& token);

  private:
    bool finish_parsing_string_with_unescape_char(typename JSON_Parser<CharType>::Token& token);
    const CharType* m_position;
    const CharType* m_startpos;
    const CharType* m_endpos;
};

template<typename CharType>
typename JSON_Parser<CharType>::int_type JSON_StreamParser<CharType>::NextCharacter()
{
    auto ch = m_streambuf->sbumpc();

    if (ch == '\n') {
        this->m_currentLine += 1;
        this->m_currentColumn = 0;
    }
    else {
        this->m_currentColumn += 1;
    }

    return ch;
}

template<typename CharType>
typename JSON_Parser<CharType>::int_type JSON_StreamParser<CharType>::PeekCharacter()
{
    return m_streambuf->sgetc();
}

template<typename CharType>
typename JSON_Parser<CharType>::int_type JSON_StringParser<CharType>::NextCharacter()
{
    if (m_position == m_endpos)
        return eof<CharType>();

    CharType ch = *m_position;
    m_position += 1;

    if (ch == '\n') {
        this->m_currentLine += 1;
        this->m_currentColumn = 0;
    }
    else {
        this->m_currentColumn += 1;
    }

    return ch;
}

template<typename CharType>
typename JSON_Parser<CharType>::int_type JSON_StringParser<CharType>::PeekCharacter()
{
    if (m_position == m_endpos)
        return eof<CharType>();

    return *m_position;
}

//
// Consume whitespace characters and return the first non-space character or EOF
//
template<typename CharType>
typename JSON_Parser<CharType>::int_type JSON_Parser<CharType>::EatWhitespace()
{
    auto ch = NextCharacter();

    while (ch != eof<CharType>() && iswspace(static_cast<wint_t>(ch))) {
        ch = NextCharacter();
    }

    return ch;
}

template<typename CharType>
bool JSON_Parser<CharType>::CompleteKeywordTrue(Token& token)
{
    if (NextCharacter() != 'r')
        return false;
    if (NextCharacter() != 'u')
        return false;
    if (NextCharacter() != 'e')
        return false;
    token.kind = Token::TKN_BooleanLiteral;
    token.boolean_val = true;
    return true;
}

template<typename CharType>
bool JSON_Parser<CharType>::CompleteKeywordFalse(Token& token)
{
    if (NextCharacter() != 'a')
        return false;
    if (NextCharacter() != 'l')
        return false;
    if (NextCharacter() != 's')
        return false;
    if (NextCharacter() != 'e')
        return false;
    token.kind = Token::TKN_BooleanLiteral;
    token.boolean_val = false;
    return true;
}

template<typename CharType>
bool JSON_Parser<CharType>::CompleteKeywordNull(Token& token)
{
    if (NextCharacter() != 'u')
        return false;
    if (NextCharacter() != 'l')
        return false;
    if (NextCharacter() != 'l')
        return false;
    token.kind = Token::TKN_NullLiteral;
    return true;
}

// Returns false only on overflow
template<typename CharType>
inline bool JSON_Parser<CharType>::ParseInt64(CharType first, uint64_t& value)
{
    value = first - '0';
    auto ch = PeekCharacter();
    while (ch >= '0' && ch <= '9') {
        unsigned int next_digit = (unsigned int)(ch - '0');
        if (value > (ULLONG_MAX / 10) || (value == ULLONG_MAX / 10 && next_digit > ULLONG_MAX % 10))
            return false;

        NextCharacter();

        value *= 10;
        value += next_digit;
        ch = PeekCharacter();
    }
    return true;
}

// This namespace hides the x-plat helper functions
namespace
{
static int __attribute__((__unused__)) print_llu(char* ptr, size_t n, unsigned long long val64)
{
    return snprintf(ptr, n, "%llu", val64);
}
static int __attribute__((__unused__)) print_llu(char* ptr, size_t n, unsigned long val64)
{
    return snprintf(ptr, n, "%lu", val64);
}
static double __attribute__((__unused__)) anystod(const char* str)
{
    return strtod(str, nullptr);
}
static double __attribute__((__unused__)) anystod(const wchar_t* str)
{
    return wcstod(str, nullptr);
}
} // namespace

template<typename CharType>
bool JSON_Parser<CharType>::CompleteNumberLiteral(CharType first, Token& token)
{
    bool minus_sign;

    if (first == '-') {
        minus_sign = true;

        // Safe to cast because the check after this if/else statement will cover EOF.
        first = static_cast<CharType>(NextCharacter());
    }
    else {
        minus_sign = false;
    }

    if (first < '0' || first > '9')
        return false;

    auto ch = PeekCharacter();

    // Check for two (or more) zeros at the beginning
    if (first == '0' && ch == '0')
        return false;

    // Parse the number assuming its integer
    uint64_t val64;
    bool complete = ParseInt64(first, val64);

    ch = PeekCharacter();
    if (complete && ch != '.' && ch != 'E' && ch != 'e') {
        if (minus_sign) {
            if (val64 > static_cast<uint64_t>(1) << 63) {
                // It is negative and cannot be represented in int64, so we resort to double
                token.double_val = 0 - static_cast<double>(val64);
                token.signed_number = true;
                token.kind = JSON_Parser<CharType>::Token::TKN_NumberLiteral;
                return true;
            }

            // It is negative, but fits into int64
            token.int64_val = 0 - static_cast<int64_t>(val64);
            token.kind = JSON_Parser<CharType>::Token::TKN_IntegerLiteral;
            token.signed_number = true;
            return true;
        }

        // It is positive so we use unsigned int64
        token.uint64_val = val64;
        token.kind = JSON_Parser<CharType>::Token::TKN_IntegerLiteral;
        token.signed_number = false;
        return true;
    }

    // Magic number 5 leaves room for decimal point, null terminator, etc (in most cases)
    ::std::vector<CharType> buf(::std::numeric_limits<uint64_t>::digits10 + 5);
    int count = print_llu(buf.data(), buf.size(), val64);
    ASSERT(count >= 0);
    ASSERT((size_t)count < buf.size());
    // Resize to cut off the null terminator
    buf.resize(count);

    bool decimal = false;

    while (ch != eof<CharType>()) {
        // Digit encountered?
        if (ch >= '0' && ch <= '9') {
            buf.push_back(static_cast<CharType>(ch));
            NextCharacter();
            ch = PeekCharacter();
        }

        // Decimal dot?
        else if (ch == '.') {
            if (decimal)
                return false;

            decimal = true;
            buf.push_back(static_cast<CharType>(ch));

            NextCharacter();
            ch = PeekCharacter();

            // Check that the following char is a digit
            if (ch < '0' || ch > '9')
                return false;

            buf.push_back(static_cast<CharType>(ch));
            NextCharacter();
            ch = PeekCharacter();
        }

        // Exponent?
        else if (ch == 'E' || ch == 'e') {
            buf.push_back(static_cast<CharType>(ch));
            NextCharacter();
            ch = PeekCharacter();

            // Check for the exponent sign
            if (ch == '+') {
                buf.push_back(static_cast<CharType>(ch));
                NextCharacter();
                ch = PeekCharacter();
            }
            else if (ch == '-') {
                buf.push_back(static_cast<CharType>(ch));
                NextCharacter();
                ch = PeekCharacter();
            }

            // First number of the exponent
            if (ch >= '0' && ch <= '9') {
                buf.push_back(static_cast<CharType>(ch));
                NextCharacter();
                ch = PeekCharacter();
            }
            else
                return false;

            // The rest of the exponent
            while (ch >= '0' && ch <= '9') {
                buf.push_back(static_cast<CharType>(ch));
                NextCharacter();
                ch = PeekCharacter();
            }

            // The peeked character is not a number, so we can break from the loop and construct the number
            break;
        }
        else {
            // Not expected number character?
            break;
        }
    };

    buf.push_back('\0');
    token.double_val = anystod(buf.data());
    if (minus_sign) {
        token.double_val = -token.double_val;
    }
    token.kind = (JSON_Parser<CharType>::Token::TKN_NumberLiteral);

    return true;
}

template<typename CharType>
bool JSON_Parser<CharType>::CompleteComment(Token& token)
{
    // We already found a '/' character as the first of a token -- what kind of comment is it?

    auto ch = NextCharacter();

    if (ch == eof<CharType>() || (ch != '/' && ch != '*'))
        return false;

    if (ch == '/') {
        // Line comment -- look for a newline or EOF to terminate.

        ch = NextCharacter();

        while (ch != eof<CharType>() && ch != '\n') {
            ch = NextCharacter();
        }
    }
    else {
        // Block comment -- look for a terminating "*/" sequence.

        ch = NextCharacter();

        while (true) {
            if (ch == eof<CharType>())
                return false;

            if (ch == '*') {
                auto ch1 = PeekCharacter();

                if (ch1 == eof<CharType>())
                    return false;

                if (ch1 == '/') {
                    // Consume the character
                    NextCharacter();
                    break;
                }

                ch = ch1;
            }

            ch = NextCharacter();
        }
    }

    token.kind = Token::TKN_Comment;

    return true;
}

template<typename CharType>
bool JSON_StringParser<CharType>::CompleteComment(typename JSON_Parser<CharType>::Token& token)
{
    // This function is specialized for the string parser, since we can be slightly more
    // efficient in copying data from the input to the token: do a memcpy() rather than
    // one character at a time.

    auto ch = JSON_StringParser<CharType>::NextCharacter();

    if (ch == eof<CharType>() || (ch != '/' && ch != '*'))
        return false;

    if (ch == '/') {
        // Line comment -- look for a newline or EOF to terminate.

        ch = JSON_StringParser<CharType>::NextCharacter();

        while (ch != eof<CharType>() && ch != '\n') {
            ch = JSON_StringParser<CharType>::NextCharacter();
        }
    }
    else {
        // Block comment -- look for a terminating "*/" sequence.

        ch = JSON_StringParser<CharType>::NextCharacter();

        while (true) {
            if (ch == eof<CharType>())
                return false;

            if (ch == '*') {
                ch = JSON_StringParser<CharType>::PeekCharacter();

                if (ch == eof<CharType>())
                    return false;

                if (ch == '/') {
                    // Consume the character
                    JSON_StringParser<CharType>::NextCharacter();
                    break;
                }
            }

            ch = JSON_StringParser<CharType>::NextCharacter();
        }
    }

    token.kind = JSON_Parser<CharType>::Token::TKN_Comment;

    return true;
}

template<typename CharType>
int JSON_Parser<CharType>::convert_unicode_to_code_point()
{
    // A four-hexdigit Unicode character.
    // Transform into a 16 bit code point.
    int decoded = 0;
    for (int i = 0; i < 4; ++i) {
        auto ch = NextCharacter();
        int ch_int = static_cast<int>(ch);
        if (ch_int < 0 || ch_int > 127)
            return -1;
        const int isxdigitResult = isxdigit(ch_int);
        if (!isxdigitResult)
            return -1;

        int val = _hexval[static_cast<size_t>(ch_int)];

        ASSERT(val != -1);

        // Add the input char to the decoded number
        decoded |= (val << (4 * (3 - i)));
    }
    return decoded;
}

template<typename CharType>
inline bool JSON_Parser<CharType>::handle_unescape_char(Token& token)
{
    token.has_unescape_symbol = true;

    // This function converts unescaped character pairs (e.g. "\t") into their ASCII or Unicode representations (e.g.
    // tab sign) Also it handles \u + 4 hexadecimal digits
    auto ch = NextCharacter();
    switch (ch) {
        case '\"':
            token.string_val.push_back('\"');
            return true;
        case '\\':
            token.string_val.push_back('\\');
            return true;
        case '/':
            token.string_val.push_back('/');
            return true;
        case 'b':
            token.string_val.push_back('\b');
            return true;
        case 'f':
            token.string_val.push_back('\f');
            return true;
        case 'r':
            token.string_val.push_back('\r');
            return true;
        case 'n':
            token.string_val.push_back('\n');
            return true;
        case 't':
            token.string_val.push_back('\t');
            return true;
        case 'u': {
            int decoded = convert_unicode_to_code_point();
            if (decoded == -1) {
                return false;
            }

            ASSERT(false); // utf16 is not implimented

            return true;
        }
        default:
            return false;
    }
}

template<typename CharType>
bool JSON_Parser<CharType>::CompleteStringLiteral(Token& token)
{
    token.has_unescape_symbol = false;
    auto ch = NextCharacter();
    while (ch != '"') {
        if (ch == '\\') {
            handle_unescape_char(token);
        }
        else if (ch >= CharType(0x0) && ch < CharType(0x20)) {
            return false;
        }
        else {
            if (ch == eof<CharType>())
                return false;

            token.string_val.push_back(static_cast<CharType>(ch));
        }
        ch = NextCharacter();
    }

    if (ch == '"') {
        token.kind = Token::TKN_StringLiteral;
    }
    else {
        return false;
    }

    return true;
}

template<typename CharType>
bool JSON_StringParser<CharType>::CompleteStringLiteral(typename JSON_Parser<CharType>::Token& token)
{
    // This function is specialized for the string parser, since we can be slightly more
    // efficient in copying data from the input to the token: do a memcpy() rather than
    // one character at a time.

    auto start = m_position;
    token.has_unescape_symbol = false;

    auto ch = JSON_StringParser<CharType>::NextCharacter();

    while (ch != '"') {
        if (ch == eof<CharType>())
            return false;

        if (ch == '\\') {
            const size_t numChars = m_position - start - 1;
            const size_t prevSize = token.string_val.size();
            token.string_val.resize(prevSize + numChars);
            memcpy(const_cast<CharType*>(token.string_val.c_str() + prevSize), start, numChars * sizeof(CharType));

            if (!JSON_StringParser<CharType>::handle_unescape_char(token)) {
                return false;
            }

            // Reset start position and continue.
            start = m_position;
        }
        else if (ch >= CharType(0x0) && ch < CharType(0x20)) {
            return false;
        }

        ch = JSON_StringParser<CharType>::NextCharacter();
    }

    const size_t numChars = m_position - start - 1;
    const size_t prevSize = token.string_val.size();
    token.string_val.resize(prevSize + numChars);
    memcpy(const_cast<CharType*>(token.string_val.c_str() + prevSize), start, numChars * sizeof(CharType));

    token.kind = JSON_Parser<CharType>::Token::TKN_StringLiteral;

    return true;
}

template<typename CharType>
void JSON_Parser<CharType>::GetNextToken(typename JSON_Parser<CharType>::Token& result)
{
try_again:
    auto ch = EatWhitespace();

    CreateToken(result, Token::TKN_EOF);

    if (ch == eof<CharType>())
        return;

    switch (ch) {
        case '{':
        case '[': {
            if (++m_currentParsingDepth > JSON_Parser<CharType>::maxParsingDepth) {
                SetErrorCode(result, json_error::nesting);
                break;
            }

            typename JSON_Parser<CharType>::Token::Kind tk = ch == '{' ? Token::TKN_OpenBrace : Token::TKN_OpenBracket;
            CreateToken(result, tk, result.start);
            break;
        }
        case '}':
        case ']': {
            if ((signed int)(--m_currentParsingDepth) < 0) {
                SetErrorCode(result, json_error::mismatched_brances);
                break;
            }

            typename JSON_Parser<CharType>::Token::Kind tk =
              ch == '}' ? Token::TKN_CloseBrace : Token::TKN_CloseBracket;
            CreateToken(result, tk, result.start);
            break;
        }
        case ',':
            CreateToken(result, Token::TKN_Comma, result.start);
            break;

        case ':':
            CreateToken(result, Token::TKN_Colon, result.start);
            break;

        case 't':
            if (!CompleteKeywordTrue(result)) {
                SetErrorCode(result, json_error::malformed_literal);
            }
            break;
        case 'f':
            if (!CompleteKeywordFalse(result)) {
                SetErrorCode(result, json_error::malformed_literal);
            }
            break;
        case 'n':
            if (!CompleteKeywordNull(result)) {
                SetErrorCode(result, json_error::malformed_literal);
            }
            break;
        case '/':
            if (!CompleteComment(result)) {
                SetErrorCode(result, json_error::malformed_comment);
                break;
            }
            // For now, we're ignoring comments.
            goto try_again;
        case '"':
            if (!CompleteStringLiteral(result)) {
                SetErrorCode(result, json_error::malformed_string_literal);
            }
            break;

        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            if (!CompleteNumberLiteral(static_cast<CharType>(ch), result)) {
                SetErrorCode(result, json_error::malformed_numeric_literal);
            }
            break;
        default:
            SetErrorCode(result, json_error::malformed_token);
            break;
    }
}

template<typename CharType>
std::unique_ptr<json::details::_Value> JSON_Parser<CharType>::_ParseObject(typename JSON_Parser<CharType>::Token& tkn)
{
    auto obj = std::make_unique<json::details::_Object>(g_keep_json_object_unsorted);
    auto& elems = obj->_object._elements;

    GetNextToken(tkn);
    if (tkn.m_error)
        goto error;

    if (tkn.kind != JSON_Parser<CharType>::Token::TKN_CloseBrace) {
        while (true) {
            // State 1: New field or end of object, looking for field name or closing brace
            std::basic_string<CharType> fieldName;
            switch (tkn.kind) {
                case JSON_Parser<CharType>::Token::TKN_StringLiteral:
                    fieldName = std::move(tkn.string_val);
                    break;
                default:
                    goto error;
            }

            GetNextToken(tkn);
            if (tkn.m_error)
                goto error;

            // State 2: Looking for a colon.
            if (tkn.kind != JSON_Parser<CharType>::Token::TKN_Colon)
                goto error;

            GetNextToken(tkn);
            if (tkn.m_error)
                goto error;

            elems.emplace_back(std::move(fieldName), json::Value(_ParseValue(tkn)));

            if (tkn.m_error)
                goto error;

            // State 4: Looking for a comma or a closing brace
            switch (tkn.kind) {
                case JSON_Parser<CharType>::Token::TKN_Comma:
                    GetNextToken(tkn);
                    if (tkn.m_error)
                        goto error;
                    break;
                case JSON_Parser<CharType>::Token::TKN_CloseBrace:
                    goto done;
                default:
                    goto error;
            }
        }
    }

done:
    GetNextToken(tkn);
    if (tkn.m_error)
        return std::make_unique<json::details::_Null>();

    if (!g_keep_json_object_unsorted) {
        ::std::sort(elems.begin(), elems.end(), json::Object::compare_pairs);
    }

    return std::unique_ptr<json::details::_Value>(obj.release());

error:
    if (!tkn.m_error) {
        SetErrorCode(tkn, json_error::malformed_object_literal);
    }
    return std::make_unique<json::details::_Null>();
}

template<typename CharType>
std::unique_ptr<json::details::_Value> JSON_Parser<CharType>::_ParseArray(typename JSON_Parser<CharType>::Token& tkn)
{
    GetNextToken(tkn);
    if (tkn.m_error)
        return std::make_unique<json::details::_Null>();

    auto result = std::make_unique<json::details::_Array>();

    if (tkn.kind != JSON_Parser<CharType>::Token::TKN_CloseBracket) {
        while (true) {
            // State 1: Looking for an expression.
            result->_array._elements.emplace_back(ParseValue(tkn));
            if (tkn.m_error)
                return std::make_unique<json::details::_Null>();

            // State 4: Looking for a comma or a closing bracket
            switch (tkn.kind) {
                case JSON_Parser<CharType>::Token::TKN_Comma:
                    GetNextToken(tkn);
                    if (tkn.m_error)
                        return std::make_unique<json::details::_Null>();
                    break;
                case JSON_Parser<CharType>::Token::TKN_CloseBracket:
                    GetNextToken(tkn);
                    if (tkn.m_error)
                        return std::make_unique<json::details::_Null>();
                    return std::unique_ptr<json::details::_Value>(result.release());
                default:
                    SetErrorCode(tkn, json_error::malformed_array_literal);
                    return std::make_unique<json::details::_Null>();
            }
        }
    }

    GetNextToken(tkn);
    if (tkn.m_error)
        return std::make_unique<json::details::_Null>();

    return std::unique_ptr<json::details::_Value>(result.release());
}

template<typename CharType>
std::unique_ptr<json::details::_Value> JSON_Parser<CharType>::_ParseValue(typename JSON_Parser<CharType>::Token& tkn)
{
    typedef std::unique_ptr<json::details::_Value> Vptr;
    switch (tkn.kind) {
        case JSON_Parser<CharType>::Token::TKN_OpenBrace: {
            return _ParseObject(tkn);
        }
        case JSON_Parser<CharType>::Token::TKN_OpenBracket: {
            return _ParseArray(tkn);
        }
        case JSON_Parser<CharType>::Token::TKN_StringLiteral: {
            Vptr value = std::make_unique<json::details::_String>(std::move(tkn.string_val), tkn.has_unescape_symbol);
            GetNextToken(tkn);
            if (tkn.m_error)
                return std::make_unique<json::details::_Null>();
            return value;
        }
        case JSON_Parser<CharType>::Token::TKN_IntegerLiteral: {
            Vptr value;
            if (tkn.signed_number)
                value = std::make_unique<json::details::_Number>(tkn.int64_val);
            else
                value = std::make_unique<json::details::_Number>(tkn.uint64_val);

            GetNextToken(tkn);
            if (tkn.m_error)
                return std::make_unique<json::details::_Null>();
            return value;
        }
        case JSON_Parser<CharType>::Token::TKN_NumberLiteral: {
            Vptr value = std::make_unique<json::details::_Number>(tkn.double_val);
            GetNextToken(tkn);
            if (tkn.m_error)
                return std::make_unique<json::details::_Null>();
            return value;
        }
        case JSON_Parser<CharType>::Token::TKN_BooleanLiteral: {
            Vptr value = std::make_unique<json::details::_Boolean>(tkn.boolean_val);
            GetNextToken(tkn);
            if (tkn.m_error)
                return std::make_unique<json::details::_Null>();
            return value;
        }
        case JSON_Parser<CharType>::Token::TKN_NullLiteral: {
            GetNextToken(tkn);
            // Returning a null value whether or not an error occurred.
            return std::make_unique<json::details::_Null>();
        }
        default: {
            SetErrorCode(tkn, json_error::malformed_token);
            return std::make_unique<json::details::_Null>();
        }
    }
}

} // namespace details

template<typename CharType>
static base::json::Value _parse_stream(std::basic_istream<CharType>& stream)
{
    base::json::details::JSON_StreamParser<CharType> parser(stream);
    typename base::json::details::JSON_Parser<CharType>::Token tkn;

    parser.GetNextToken(tkn);
    if (tkn.m_error) {
        base::json::details::CreateException(tkn, tkn.m_error.message());
    }

    auto value = parser.ParseValue(tkn);
    if (tkn.m_error) {
        base::json::details::CreateException(tkn, tkn.m_error.message());
    }
    else if (tkn.kind != base::json::details::JSON_Parser<CharType>::Token::TKN_EOF) {
        base::json::details::CreateException(tkn, "Left-over characters in stream after parsing a JSON value");
    }
    return value;
}

template<typename CharType>
static base::json::Value _parse_stream(std::basic_istream<CharType>& stream, std::error_code& error)
{
    base::json::details::JSON_StreamParser<CharType> parser(stream);
    typename base::json::details::JSON_Parser<CharType>::Token tkn;

    parser.GetNextToken(tkn);
    if (tkn.m_error) {
        error = std::move(tkn.m_error);
        return base::json::Value();
    }

    auto returnObject = parser.ParseValue(tkn);
    if (tkn.kind != base::json::details::JSON_Parser<CharType>::Token::TKN_EOF) {
        base::json::details::SetErrorCode(tkn, base::json::details::json_error::left_over_character_in_stream);
    }

    error = std::move(tkn.m_error);
    return returnObject;
}

template<typename CharType>
static base::json::Value _parse_string(const std::basic_string<CharType>& str)
{
    base::json::details::JSON_StringParser<CharType> parser(str);
    typename base::json::details::JSON_Parser<CharType>::Token tkn;

    parser.GetNextToken(tkn);
    if (tkn.m_error) {
        base::json::details::CreateException(tkn, tkn.m_error.message());
    }

    auto value = parser.ParseValue(tkn);
    if (tkn.m_error) {
        base::json::details::CreateException(tkn, tkn.m_error.message());
    }
    else if (tkn.kind != base::json::details::JSON_Parser<CharType>::Token::TKN_EOF) {
        base::json::details::CreateException(tkn, "Left-over characters in stream after parsing a JSON value");
    }
    return value;
}

template<typename CharType>
static base::json::Value _parse_string(const std::basic_string<CharType>& str, std::error_code& error)
{
    base::json::details::JSON_StringParser<CharType> parser(str);
    typename base::json::details::JSON_Parser<CharType>::Token tkn;

    parser.GetNextToken(tkn);
    if (tkn.m_error) {
        error = std::move(tkn.m_error);
        return base::json::Value();
    }

    auto returnObject = parser.ParseValue(tkn);
    if (tkn.kind != base::json::details::JSON_Parser<CharType>::Token::TKN_EOF) {
        returnObject = base::json::Value();
        base::json::details::SetErrorCode(tkn, base::json::details::json_error::left_over_character_in_stream);
    }

    error = std::move(tkn.m_error);
    return returnObject;
}

Value::Value()
  : _value(std::make_unique<details::_Null>())
{}

Value::Value(int32_t value)
  : _value(std::make_unique<details::_Number>(value))
{}

Value::Value(uint32_t value)
  : _value(std::make_unique<details::_Number>(value))
{}

Value::Value(int64_t value)
  : _value(std::make_unique<details::_Number>(value))
{}

Value::Value(uint64_t value)
  : _value(std::make_unique<details::_Number>(value))
{}

Value::Value(double value)
  : _value(std::make_unique<details::_Number>(value))
{}

Value::Value(bool value)
  : _value(std::make_unique<details::_Boolean>(value))
{}

Value::Value(std::string value)
  : _value(std::make_unique<details::_String>(std::move(value)))
{}

Value::Value(std::string value, bool has_escape_chars)
  : _value(std::make_unique<details::_String>(std::move(value), has_escape_chars))
{}

Value::Value(const char* value)
  : _value(std::make_unique<details::_String>(value))
{}

Value::Value(const char* value, bool has_escape_chars)
  : _value(std::make_unique<details::_String>(std::string(value), has_escape_chars))
{}

Value::Value(const Value& other)
  : _value(other._value->_copy_value())
{}

Value& Value::operator=(const Value& other)
{
    if (this != &other) {
        _value = std::unique_ptr<details::_Value>(other._value->_copy_value());
    }
    return *this;
}

Value::Value(Value&& other) noexcept
  : _value(std::move(other._value))
{}

Value& Value::operator=(Value&& other) noexcept
{
    if (this != &other) {
        _value.swap(other._value);
    }
    return *this;
}

Value Value::null()
{
    return Value();
}

Value Value::number(double val)
{
    return Value(val);
}

Value Value::number(int32_t val)
{
    return Value(val);
}

Value Value::number(uint32_t val)
{
    return Value(val);
}

Value Value::number(int64_t val)
{
    return Value(val);
}

Value Value::number(uint64_t val)
{
    return Value(val);
}

Value Value::boolean(bool val)
{
    return Value(val);
}

Value Value::string(std::string val)
{
    std::unique_ptr<details::_Value> ptr = std::make_unique<details::_String>(std::move(val));
    return Value(std::move(ptr));
}

Value Value::string(std::string val, bool has_escape_chars)
{
    std::unique_ptr<details::_Value> ptr = std::make_unique<details::_String>(std::move(val), has_escape_chars);
    return Value(std::move(ptr));
}

Value Value::object(bool keep_order)
{
    std::unique_ptr<details::_Value> ptr = std::make_unique<details::_Object>(keep_order);
    return Value(std::move(ptr));
}

Value Value::object(std::vector<std::pair<std::string, Value>> fields, bool keep_order)
{
    std::unique_ptr<details::_Value> ptr = std::make_unique<details::_Object>(std::move(fields), keep_order);
    return Value(std::move(ptr));
}

Value Value::array()
{
    std::unique_ptr<details::_Value> ptr = std::make_unique<details::_Array>();
    return Value(std::move(ptr));
}

Value Value::array(size_t size)
{
    std::unique_ptr<details::_Value> ptr = std::make_unique<details::_Array>(size);
    return Value(std::move(ptr));
}

Value Value::array(std::vector<Value> elements)
{
    std::unique_ptr<details::_Value> ptr = std::make_unique<details::_Array>(std::move(elements));
    return Value(std::move(ptr));
}

Value::ValueType Value::type() const
{
    return _value->type();
}

bool Value::is_null() const
{
    return type() == Null;
}

bool Value::is_number() const
{
    return type() == Number;
}

bool Value::is_integer() const
{
    if (!is_number()) {
        return false;
    }
    return _value->is_integer();
}

bool Value::is_double() const
{
    if (!is_number()) {
        return false;
    }
    return _value->is_double();
}

bool Value::is_boolean() const
{
    return type() == Boolean;
}

bool Value::is_string() const
{
    return type() == String;
}

bool Value::is_array() const
{
    return type() == Array;
}

bool Value::is_object() const
{
    return type() == Object;
}

std::size_t Value::size() const
{
    return _value->size();
}

base::json::Value base::json::Value::parse(const std::string& str)
{
    return _parse_string(str);
}

base::json::Value base::json::Value::parse(const std::string& str, std::error_code& error)
{
    return _parse_string(str, error);
}

std::string json::Value::serialize() const
{
    return _value->to_string();
}

base::json::Value base::json::Value::parse(std::istream& stream)
{
    return _parse_stream(stream);
}

base::json::Value base::json::Value::parse(std::istream& stream, std::error_code& error)
{
    return _parse_stream(stream, error);
}

void Value::serialize(std::ostream& stream) const
{
    // This has better performance than writing directly to stream.
    std::string str;
    _value->serialize_impl(str);
    stream << str;
}

const Number& Value::as_number() const
{
    return _value->as_number();
}

double Value::as_double() const
{
    return _value->as_double();
}

int Value::as_integer() const
{
    return _value->as_integer();
}

bool Value::as_bool() const
{
    return _value->as_bool();
}

Array& Value::as_array()
{
    return _value->as_array();
}

const Array& Value::as_array() const
{
    return _value->as_array();
}

Object& Value::as_object()
{
    return _value->as_object();
}

const Object& Value::as_object() const
{
    return _value->as_object();
}

const std::string& Value::as_string() const
{
    return _value->as_string();
}

bool Value::operator==(const Value& other) const
{
    if (this->_value.get() == other._value.get())
        return true;
    if (this->type() != other.type())
        return false;

    switch (this->type()) {
        case Null:
            return true;
        case Number:
            return this->as_number() == other.as_number();
        case Boolean:
            return this->as_bool() == other.as_bool();
        case String:
            return this->as_string() == other.as_string();
        case Object:
            return static_cast<const details::_Object*>(this->_value.get())
              ->is_equal(static_cast<const details::_Object*>(other._value.get()));
        case Array:
            return static_cast<const details::_Array*>(this->_value.get())
              ->is_equal(static_cast<const details::_Array*>(other._value.get()));
    }
    ASSERT(false);
}

bool Value::operator!=(const Value& other) const
{
    return !((*this) == other);
}

bool Value::has_field(const std::string& key) const
{
    return _value->has_field(key);
}

bool Value::has_number_field(const std::string& key) const
{
    return has_field(key) && at(key).is_number();
}

bool Value::has_integer_field(const std::string& key) const
{
    return has_field(key) && at(key).is_integer();
}

bool Value::has_double_field(const std::string& key) const
{
    return has_field(key) && at(key).is_double();
}

bool Value::has_boolean_field(const std::string& key) const
{
    return has_field(key) && at(key).is_boolean();
}

bool Value::has_string_field(const std::string& key) const
{
    return has_field(key) && at(key).is_string();
}

bool Value::has_array_field(const std::string& key) const
{
    return has_field(key) && at(key).is_array();
}

bool Value::has_object_field(const std::string& key) const
{
    return has_field(key) && at(key).is_object();
}

void Value::erase(size_t index)
{
    return this->as_array().erase(index);
}

void Value::erase(const std::string& key)
{
    return this->as_object().erase(key);
}

Value& Value::at(size_t index)
{
    return this->as_array().at(index);
}

const Value& Value::at(size_t index) const
{
    return this->as_array().at(index);
}

Value& Value::at(const std::string& key)
{
    return this->as_object().at(key);
}

const Value& Value::at(const std::string& key) const
{
    return this->as_object().at(key);
}

Value& Value::operator[](const std::string& key)
{
    if (this->is_null()) {
        _value.reset(new details::_Object(details::g_keep_json_object_unsorted));
    }
    return _value->index(key);
}

Value& Value::operator[](size_t index)
{
    if (this->is_null()) {
        _value.reset(new details::_Array());
    }
    return _value->index(index);
}

void Value::format(std::basic_string<char>& string) const
{
    _value->format(string);
}

Value::Value(std::unique_ptr<details::_Value> v)
  : _value(std::move(v))
{}

json_exception::json_exception(const char* const message)
  : _message(message)
{}

json_exception::json_exception(std::string&& message)
  : _message(std::move(message))
{}

const char* json_exception::what() const noexcept
{
    return _message.c_str();
}

namespace details
{

const char* json_error_category_impl::name() const noexcept
{
    return "json";
}

std::string json_error_category_impl::message(int ev) const
{
    switch (ev) {
        case json_error::left_over_character_in_stream:
            return "Left-over characters in stream after parsing a JSON value";
        case json_error::malformed_array_literal:
            return "Malformed array literal";
        case json_error::malformed_comment:
            return "Malformed comment";
        case json_error::malformed_literal:
            return "Malformed literal";
        case json_error::malformed_object_literal:
            return "Malformed object literal";
        case json_error::malformed_numeric_literal:
            return "Malformed numeric literal";
        case json_error::malformed_string_literal:
            return "Malformed string literal";
        case json_error::malformed_token:
            return "Malformed token";
        case json_error::mismatched_brances:
            return "Mismatched braces";
        case json_error::nesting:
            return "Nesting too deep";
        case json_error::unexpected_token:
            return "Unexpected token";
        default:
            return "Unknown json error";
    }
}

const json_error_category_impl& json_error_category()
{
    static details::json_error_category_impl instance;
    return instance;
}

} // namespace details

Array::Array(std::size_t size)
  : _elements(size)
{}

Array::Array(storage_type elements)
  : _elements(std::move(elements))
{}

Array::iterator Array::begin()
{
    return _elements.begin();
}

Array::const_iterator Array::begin() const
{
    return _elements.cbegin();
}

Array::iterator Array::end()
{
    return _elements.end();
}

Array::const_iterator Array::end() const
{
    return _elements.cend();
}

Array::reverse_iterator Array::rbegin()
{
    return _elements.rbegin();
}

Array::const_reverse_iterator Array::rbegin() const
{
    return _elements.rbegin();
}

Array::reverse_iterator Array::rend()
{
    return _elements.rend();
}

Array::const_reverse_iterator Array::rend() const
{
    return _elements.crend();
}

Array::const_iterator Array::cbegin() const
{
    return _elements.cbegin();
}

Array::const_iterator Array::cend() const
{
    return _elements.cend();
}

Array::const_reverse_iterator Array::crbegin() const
{
    return _elements.crbegin();
}

Array::const_reverse_iterator Array::crend() const
{
    return _elements.crend();
}

Array::iterator Array::erase(iterator position)
{
    return _elements.erase(position);
}

void Array::erase(std::size_t index)
{
    if (index >= _elements.size()) {
        throw json_exception("index out of bounds");
    }
    _elements.erase(_elements.begin() + index);
}

Value& Array::at(std::size_t index)
{
    if (index >= _elements.size())
        throw json_exception("index out of bounds");

    return _elements[index];
}

const Value& Array::at(std::size_t index) const
{
    if (index >= _elements.size())
        throw json_exception("index out of bounds");

    return _elements[index];
}

Value& Array::operator[](std::size_t index)
{
    std::size_t nMinSize(index);
    nMinSize += 1;
    std::size_t nlastSize(_elements.size());
    if (nlastSize < nMinSize)
        _elements.resize((std::size_t)nMinSize);

    return _elements[index];
}

std::size_t Array::size() const
{
    return _elements.size();
}

Object::Object(bool keep_order)
  : _keep_order(keep_order)
{}

Object::Object(storage_type elements, bool keep_order)
  : _elements(std::move(elements))
  , _keep_order(keep_order)
{
    if (!keep_order) {
        sort(_elements.begin(), _elements.end(), compare_pairs);
    }
}

Object::iterator Object::begin()
{
    return _elements.begin();
}

Object::const_iterator Object::begin() const
{
    return _elements.cbegin();
}

Object::iterator Object::end()
{
    return _elements.end();
}

Object::const_iterator Object::end() const
{
    return _elements.cend();
}

Object::reverse_iterator Object::rbegin()
{
    return _elements.rbegin();
}

Object::const_reverse_iterator Object::rbegin() const
{
    return _elements.rbegin();
}

Object::reverse_iterator Object::rend()
{
    return _elements.rend();
}

Object::const_reverse_iterator Object::rend() const
{
    return _elements.crend();
}

Object::const_iterator Object::cbegin() const
{
    return _elements.cbegin();
}

Object::const_iterator Object::cend() const
{
    return _elements.cend();
}

Object::const_reverse_iterator Object::crbegin() const
{
    return _elements.crbegin();
}

Object::const_reverse_iterator Object::crend() const
{
    return _elements.crend();
}

Object::iterator Object::erase(iterator position)
{
    return _elements.erase(position);
}

void Object::erase(const std::string& key)
{
    auto iter = find_by_key(key);
    if (iter == _elements.end()) {
        throw json_exception("Key not found");
    }

    _elements.erase(iter);
}

Value& Object::at(const std::string& key)
{
    auto iter = find_by_key(key);
    if (iter == _elements.end()) {
        throw json_exception("Key not found");
    }

    return iter->second;
}

const Value& Object::at(const std::string& key) const
{
    auto iter = find_by_key(key);
    if (iter == _elements.end()) {
        throw json_exception("Key not found");
    }

    return iter->second;
}

Value& Object::operator[](const std::string& key)
{
    auto iter = find_insert_location(key);

    if (iter == _elements.end() || key != iter->first) {
        return _elements.insert(iter, std::pair<std::string, Value>(key, Value()))->second;
    }

    return iter->second;
}

Object::const_iterator Object::find(const std::string& key) const
{
    return find_by_key(key);
}

std::size_t Object::size() const
{
    return _elements.size();
}

bool Object::empty() const
{
    return _elements.empty();
}

bool Object::compare_pairs(const std::pair<std::string, Value>& p1, const std::pair<std::string, Value>& p2)
{
    return p1.first < p2.first;
}

bool Object::compare_with_key(const std::pair<std::string, Value>& p1, const std::string& key)
{
    return p1.first < key;
}

Object::storage_type::iterator Object::find_insert_location(const std::string& key)
{
    if (_keep_order) {
        return std::find_if(_elements.begin(), _elements.end(), [&key](const std::pair<std::string, Value>& p) {
            return p.first == key;
        });
    }
    else {
        return std::lower_bound(_elements.begin(), _elements.end(), key, compare_with_key);
    }
}

Object::storage_type::const_iterator Object::find_by_key(const std::string& key) const
{
    if (_keep_order) {
        return std::find_if(_elements.begin(), _elements.end(), [&key](const std::pair<std::string, Value>& p) {
            return p.first == key;
        });
    }
    else {
        auto iter = std::lower_bound(_elements.begin(), _elements.end(), key, compare_with_key);
        if (iter != _elements.end() && key != iter->first) {
            return _elements.end();
        }
        return iter;
    }
}

Object::storage_type::iterator Object::find_by_key(const std::string& key)
{
    auto iter = find_insert_location(key);
    if (iter != _elements.end() && key != iter->first) {
        return _elements.end();
    }
    return iter;
}

Number::Number(double val)
  : _value(val)
  , _type(double_type)
{}

Number::Number(std::int32_t val)
  : _intval(val)
  , _type(val < 0 ? signed_type : unsigned_type)
{}

Number::Number(std::uint32_t val)
  : _intval(val)
  , _type(unsigned_type)
{}

Number::Number(std::int64_t val)
  : _intval(val)
  , _type(val < 0 ? signed_type : unsigned_type)
{}

Number::Number(std::uint64_t val)
  : _uintval(val)
  , _type(unsigned_type)
{}

bool Number::is_int32() const
{
    switch (_type) {
        case signed_type:
            return _intval >= (std::numeric_limits<std::int32_t>::min)() &&
                   _intval <= (std::numeric_limits<std::int32_t>::max)();
        case unsigned_type:
            return _uintval <= (std::uint32_t)(std::numeric_limits<std::int32_t>::max)();
        case double_type:
        default:
            return false;
    }
}

bool Number::is_uint32() const
{
    switch (_type) {
        case signed_type:
            return _intval >= 0 && _intval <= (std::numeric_limits<std::uint32_t>::max)();
        case unsigned_type:
            return _uintval <= (std::numeric_limits<std::uint32_t>::max)();
        case double_type:
        default:
            return false;
    }
}

bool Number::is_int64() const
{
    switch (_type) {
        case signed_type:
            return true;
        case unsigned_type:
            return _uintval <= static_cast<uint64_t>((std::numeric_limits<int64_t>::max)());
        case double_type:
        default:
            return false;
    }
}

bool Number::is_uint64() const
{
    switch (_type) {
        case signed_type:
            return _intval >= 0;
        case unsigned_type:
            return true;
        case double_type:
        default:
            return false;
    }
}

double Number::to_double() const
{
    switch (_type) {
        case double_type:
            return _value;
        case signed_type:
            return static_cast<double>(_intval);
        case unsigned_type:
            return static_cast<double>(_uintval);
        default:
            return false;
    }
}

std::int32_t Number::to_int32() const
{
    if (_type == double_type)
        return static_cast<std::int32_t>(_value);
    else
        return static_cast<std::int32_t>(_intval);
}

std::uint32_t Number::to_uint32() const
{
    if (_type == double_type)
        return static_cast<std::uint32_t>(_value);
    else
        return static_cast<std::uint32_t>(_intval);
}

std::int64_t Number::to_int64() const
{
    if (_type == double_type)
        return static_cast<std::int64_t>(_value);
    else
        return static_cast<std::int64_t>(_intval);
}

std::uint64_t Number::to_uint64() const
{
    if (_type == double_type)
        return static_cast<std::uint64_t>(_value);
    else
        return static_cast<std::uint64_t>(_intval);
}

bool Number::is_integral() const
{
    return _type != double_type;
}

bool Number::operator==(const Number& other) const
{
    if (_type != other._type)
        return false;

    switch (_type) {
        case Number::type::signed_type:
            return _intval == other._intval;
        case Number::type::unsigned_type:
            return _uintval == other._uintval;
        case Number::type::double_type:
            return _value == other._value;
    }
    ASSERT(false);
}

namespace details
{

bool _Value::has_field(const std::string&) const
{
    return false;
}

Value _Value::get_field(const std::string&) const
{
    throw json_exception("not an object");
}

Value _Value::get_element(Array::size_type) const
{
    throw json_exception("not an array");
}

Value& _Value::index(const std::string&)
{
    throw json_exception("not an object");
}

Value& _Value::index(Array::size_type)
{
    throw json_exception("not an array");
}

const Value& _Value::cnst_index(const std::string&) const
{
    throw json_exception("not an object");
}

const Value& _Value::cnst_index(Array::size_type) const
{
    throw json_exception("not an array");
}

void _Value::serialize_impl(std::string& str) const
{
    format(str);
}

std::string _Value::to_string() const
{
    std::string str;
    serialize_impl(str);
    return str;
}

Value::ValueType _Value::type() const
{
    return Value::Null;
}

bool _Value::is_integer() const
{
    throw json_exception("not a number");
}

bool _Value::is_double() const
{
    throw json_exception("not a number");
}

const Number& _Value::as_number()
{
    throw json_exception("not a number");
}

double _Value::as_double() const
{
    throw json_exception("not a number");
}

int _Value::as_integer() const
{
    throw json_exception("not a number");
}

bool _Value::as_bool() const
{
    throw json_exception("not a boolean");
}

Array& _Value::as_array()
{
    throw json_exception("not an array");
}

const Array& _Value::as_array() const
{
    throw json_exception("not an array");
}

Object& _Value::as_object()
{
    throw json_exception("not an object");
}

const Object& _Value::as_object() const
{
    throw json_exception("not an object");
}

const std::string& _Value::as_string() const
{
    throw json_exception("not a string");
}

std::size_t _Value::size() const
{
    return 0;
}

void _Value::format(std::basic_string<char>& stream) const
{
    stream.append("null");
}

std::unique_ptr<_Value> _Null::_copy_value()
{
    return std::make_unique<_Null>();
}

Value::ValueType _Null::type() const
{
    return Value::Null;
}

_Number::_Number(double value)
  : _number(value)
{}

_Number::_Number(std::int32_t value)
  : _number(value)
{}

_Number::_Number(std::uint32_t value)
  : _number(value)
{}

_Number::_Number(std::int64_t value)
  : _number(value)
{}

_Number::_Number(std::uint64_t value)
  : _number(value)
{}

std::unique_ptr<_Value> _Number::_copy_value()
{
    return std::make_unique<_Number>(*this);
}

Value::ValueType _Number::type() const
{
    return Value::Number;
}

bool _Number::is_integer() const
{
    return _number.is_integral();
}

bool _Number::is_double() const
{
    return !_number.is_integral();
}

double _Number::as_double() const
{
    return _number.to_double();
}

int _Number::as_integer() const
{
    return _number.to_int32();
}

const Number& _Number::as_number()
{
    return _number;
}

void _Number::format(std::basic_string<char>& stream) const
{
    if (_number._type != Number::type::double_type) {
        // #digits + 1 to avoid loss + 1 for the sign + 1 for null terminator.
        const size_t tempSize = std::numeric_limits<uint64_t>::digits10 + 3;
        char tempBuffer[tempSize];

        int numChars;
        if (_number._type == Number::type::signed_type)
            numChars = snprintf(tempBuffer, tempSize, "%" PRId64, _number._intval);
        else
            numChars = snprintf(tempBuffer, tempSize, "%" PRIu64, _number._uintval);

        stream.append(tempBuffer, numChars);
    }
    else {
        // #digits + 2 to avoid loss + 1 for the sign + 1 for decimal point + 5 for exponent (e+xxx) + 1 for null
        // terminator
        const size_t tempSize = std::numeric_limits<double>::digits10 + 10;
        char tempBuffer[tempSize];

        const auto numChars =
          snprintf(tempBuffer, tempSize, "%.*g", std::numeric_limits<double>::digits10 + 2, _number._value);
        stream.append(tempBuffer, numChars);
    }
}

_Boolean::_Boolean(bool value)
  : _value(value)
{}

std::unique_ptr<_Value> _Boolean::_copy_value()
{
    return std::make_unique<_Boolean>(*this);
}

Value::ValueType _Boolean::type() const
{
    return Value::Boolean;
}

bool _Boolean::as_bool() const
{
    return _value;
}

void _Boolean::format(std::basic_string<char>& stream) const
{
    stream.append(_value ? "true" : "false");
}

_String::_String(std::string value)
  : _string(std::move(value))
{
    _has_escape_char = has_escape_chars(*this);
}

_String::_String(std::string value, bool escaped_chars)
  : _string(std::move(value))
  , _has_escape_char(escaped_chars)
{}

std::unique_ptr<_Value> _String::_copy_value()
{
    return std::make_unique<_String>(*this);
}

Value::ValueType _String::type() const
{
    return Value::String;
}

const std::string& _String::as_string() const
{
    return _string;
}

void _String::serialize_impl(std::string& str) const
{
    serialize_impl_char_type(str);
}

void _String::format(std::basic_string<char>& str) const
{
    str.push_back('"');

    if (_has_escape_char) {
        append_escape_string(str, _string);
    }
    else {
        str.append(_string);
    }

    str.push_back('"');
}

size_t _String::get_reserve_size() const
{
    return _string.size() + 2;
}

bool _String::has_escape_chars(const _String& str)
{
    return std::any_of(std::begin(str._string), std::end(str._string), [](std::string::value_type const x) {
        if (x <= 31) {
            return true;
        }
        if (x == '"') {
            return true;
        }
        if (x == '\\') {
            return true;
        }
        return false;
    });
}

void format_string(const std::string& key, std::string& str)
{
    str.push_back('"');
    append_escape_string(str, key);
    str.push_back('"');
}

_Object::_Object(bool keep_order)
  : _object(keep_order)
{}

_Object::_Object(Object::storage_type fields, bool keep_order)
  : _object(std::move(fields), keep_order)
{}

std::unique_ptr<_Value> _Object::_copy_value()
{
    return std::make_unique<_Object>(*this);
}

Object& _Object::as_object()
{
    return _object;
}

const Object& _Object::as_object() const
{
    return _object;
}

Value::ValueType _Object::type() const
{
    return Value::Object;
}

bool _Object::has_field(const std::string& key) const
{
    return _object.find(key) != _object.end();
}

Value& _Object::index(const std::string& key)
{
    return _object[key];
}

bool _Object::is_equal(const _Object* other) const
{
    if (_object.size() != other->_object.size())
        return false;

    return std::equal(std::begin(_object), std::end(_object), std::begin(other->_object));
}

void _Object::serialize_impl(std::string& str) const
{
    // To avoid repeated allocations reserve some space all up front.
    str.reserve(get_reserve_size());
    format(str);
}

size_t _Object::size() const
{
    return _object.size();
}

void _Object::format(std::basic_string<char>& str) const
{
    format_impl(str);
}

std::size_t _Object::get_reserve_size() const
{
    // This is a heuristic we can tune more in the future:
    // Basically size of string plus
    // sum size of value if an object, array, or string.
    size_t reserveSize = 2; // For brackets {}
    for (auto iter = _object.begin(); iter != _object.end(); ++iter) {
        reserveSize += iter->first.length() + 2;     // 2 for quotes
        size_t valueSize = iter->second.size() * 20; // Multiply by each object/array element
        if (valueSize == 0) {
            if (iter->second.type() == Value::String) {
                valueSize = static_cast<_String*>(iter->second._value.get())->get_reserve_size();
            }
            else {
                valueSize = 5; // true, false, or null
            }
        }
        reserveSize += valueSize;
    }
    return reserveSize;
}

_Array::_Array(Array::size_type size)
  : _array(size)
{}

_Array::_Array(Array::storage_type elements)
  : _array(std::move(elements))
{}

std::unique_ptr<_Value> _Array::_copy_value()
{
    return std::make_unique<_Array>(*this);
}

Value::ValueType _Array::type() const
{
    return Value::Array;
}

Array& _Array::as_array()
{
    return _array;
}
const Array& _Array::as_array() const
{
    return _array;
}

Value& _Array::index(Array::size_type index)
{
    return _array[index];
}

bool _Array::is_equal(const _Array* other) const
{
    if (_array.size() != other->_array.size())
        return false;

    auto iterT = _array.cbegin();
    auto iterO = other->_array.cbegin();
    auto iterTe = _array.cend();
    auto iterOe = other->_array.cend();

    for (; iterT != iterTe && iterO != iterOe; ++iterT, ++iterO) {
        if (*iterT != *iterO)
            return false;
    }

    return true;
}

void _Array::serialize_impl(std::string& str) const
{
    // To avoid repeated allocations reserve some space all up front.
    str.reserve(get_reserve_size());
    format(str);
}

std::size_t _Array::size() const
{
    return _array.size();
}

void _Array::format(std::basic_string<char>& str) const
{
    format_impl(str);
}

std::size_t _Array::get_reserve_size() const
{
    // This is a heuristic we can tune more in the future:
    // Basically sum size of each value if an object, array, or string by a multiplier.
    size_t reserveSize = 2; // For brackets []
    for (auto iter = _array.cbegin(); iter != _array.cend(); ++iter) {
        size_t valueSize = iter->size() * 20; // Per each nested array/object

        if (valueSize == 0)
            valueSize = 5; // true, false, or null

        reserveSize += valueSize;
    }
    return reserveSize;
}

} // namespace detail

} // namespace json

std::ostream& operator<<(std::ostream& os, const json::Value& val)
{
    val.serialize(os);
    return os;
}

std::istream& operator>>(std::istream& is, json::Value& val)
{
    val = json::Value::parse(is);
    return is;
}

} // namespace base