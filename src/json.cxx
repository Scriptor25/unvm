#include <json.hxx>

#include <iostream>
#include <sstream>
#include <variant>

bool json::Node::IsNull() const
{
    return Type == NodeType::Null;
}

bool json::Node::IsBoolean() const
{
    return Type == NodeType::Boolean;
}

bool json::Node::IsNumber() const
{
    return Type == NodeType::Number;
}

bool json::Node::IsString() const
{
    return Type == NodeType::String;
}

bool json::Node::IsArray() const
{
    return Type == NodeType::Array;
}

bool json::Node::IsObject() const
{
    return Type == NodeType::Object;
}

const bool &json::Node::AsBoolean() const
{
    return *std::get_if<bool>(&Value);
}

const long double &json::Node::AsNumber() const
{
    return *std::get_if<long double>(&Value);
}

const std::string &json::Node::AsString() const
{
    return *std::get_if<std::string>(&Value);
}

const std::vector<json::Node> &json::Node::AsArray() const
{
    return *std::get_if<std::vector<Node>>(&Value);
}

std::map<std::string, json::Node> &json::Node::AsObject()
{
    return *std::get_if<std::map<std::string, Node>>(&Value);
}

const std::map<std::string, json::Node> &json::Node::AsObject() const
{
    return *std::get_if<std::map<std::string, Node>>(&Value);
}

json::Node &json::Node::Get(const std::string &key)
{
    if (IsObject())
    {
        if (auto &object = AsObject(); object.contains(key))
        {
            return object.at(key);
        }
    }
    throw std::runtime_error(std::format("missing key {} in object", key));
}

const json::Node &json::Node::Get(const std::string &key) const
{
    if (IsObject())
    {
        if (auto &object = AsObject(); object.contains(key))
        {
            return object.at(key);
        }
    }
    throw std::runtime_error(std::format("missing key {} in object", key));
}

json::Node json::Node::GetOrNull(const std::string &key) const
{
    if (IsObject())
    {
        if (auto &object = AsObject(); object.contains(key))
        {
            return object.at(key);
        }
    }
    return {};
}

std::ostream &json::Node::Print(std::ostream &stream) const
{
    switch (Type)
    {
    case NodeType::Null:
        stream << "null";
        break;

    case NodeType::Boolean:
        stream << (AsBoolean() ? "true" : "false");
        break;

    case NodeType::Number:
        stream << AsNumber();
        break;

    case NodeType::String:
    {
        stream << '"';
        for (auto &string = AsString(); auto &c : string)
        {
            switch (c)
            {
            case '\a':
                stream << "\\a";
                break;
            case '\b':
                stream << "\\b";
                break;
            case '\f':
                stream << "\\f";
                break;
            case '\n':
                stream << "\\n";
                break;
            case '\r':
                stream << "\\r";
                break;
            case '\t':
                stream << "\\t";
                break;
            case '\v':
                stream << "\\v";
                break;
            case '\\':
                stream << "\\\\";
                break;
            case '"':
                stream << "\\\"";
                break;

            default:
                if (c >= 0x20)
                {
                    stream << c;
                    break;
                }

                const auto hi = ((c >> 4) & 0xF);
                const auto lo = (c & 0xF);

                stream
                        << "\\x"
                        << (hi >= 10 ? ((hi - 10) + 'A') : (hi + '0'))
                        << (lo >= 10 ? ((lo - 10) + 'A') : (lo + '0'));
                break;
            }
        }
        stream << '"';
        break;
    }

    case NodeType::Array:
    {
        stream << '[';
        auto &values = AsArray();
        for (auto i = values.begin(); i != values.end(); ++i)
        {
            if (i != values.begin())
            {
                stream << ',';
            }
            stream << *i;
        }
        stream << ']';
        break;
    }

    case NodeType::Object:
    {
        stream << '{';
        auto &values = AsObject();
        for (auto i = values.begin(); i != values.end(); ++i)
        {
            if (i != values.begin())
            {
                stream << ',';
            }
            stream << '"' << i->first << '"' << ':' << i->second;
        }
        stream << '}';
        break;
    }
    }

    return stream;
}

json::Node json::Parser::Parse(std::istream &stream)
{
    Parser parser(stream);
    return parser.Parse();
}

json::Node json::Parser::Parse(const std::string &string)
{
    std::istringstream stream(string);
    return Parse(stream);
}

json::Node json::Parser::Parse(std::string &&string)
{
    std::istringstream stream(string);
    return Parse(stream);
}

json::Parser::Parser(std::istream &stream)
    : m_Stream(stream)
{
    Get();
    Next();
}

json::Node json::Parser::Parse()
{
    if (At(TokenType::Other, "{"))
    {
        Skip();

        std::map<std::string, Node> values;
        while (!At(TokenType::Other, "}"))
        {
            auto key = Expect(TokenType::String).String;
            Expect(TokenType::Other, ":");
            values[key] = Parse();

            if (!At(TokenType::Other, "}"))
            {
                Expect(TokenType::Other, ",");
            }
        }
        Expect(TokenType::Other, "}");

        return {
            .Type = NodeType::Object,
            .Value = std::move(values),
        };
    }

    if (At(TokenType::Other, "["))
    {
        Skip();

        std::vector<Node> values;
        while (!At(TokenType::Other, "]"))
        {
            values.push_back(Parse());

            if (!At(TokenType::Other, "]"))
            {
                Expect(TokenType::Other, ",");
            }
        }
        Expect(TokenType::Other, "]");

        return {
            .Type = NodeType::Array,
            .Value = std::move(values),
        };
    }

    if (At(TokenType::String))
    {
        auto value = Skip().String;

        return {
            .Type = NodeType::String,
            .Value = std::move(value),
        };
    }

    if (At(TokenType::Number))
    {
        auto value = Skip().Number;

        return {
            .Type = NodeType::Number,
            .Value = value,
        };
    }

    if (At(TokenType::Symbol, "true") || At(TokenType::Symbol, "false"))
    {
        const auto value = Skip().String;

        return {
            .Type = NodeType::Boolean,
            .Value = value == "true",
        };
    }

    if (At(TokenType::Symbol, "null"))
    {
        Skip();

        return {
            .Type = NodeType::Null,
        };
    }

    throw std::runtime_error(std::format("parse {} '{}'", m_Token.Type, m_Token.String));
}

void json::Parser::Get()
{
    m_Buffer = m_Stream.get();
}

void json::Parser::Next()
{
    enum class State
    {
        None,
        Symbol,
        Number,
        String,
    } state = State::None;

    bool floating = false;
    std::string value;

    while (m_Buffer >= 0)
    {
        switch (state)
        {
        case State::None:
            switch (m_Buffer)
            {
            case '"':
                Get();
                state = State::String;
                break;

            default:
                if (std::isspace(m_Buffer))
                {
                    Get();
                    break;
                }

                if (std::isdigit(m_Buffer))
                {
                    state = State::Number;
                    break;
                }

                if (std::isalpha(m_Buffer))
                {
                    state = State::Symbol;
                    break;
                }

                value += static_cast<char>(m_Buffer);
                Get();

                m_Token = {
                    TokenType::Other,
                    std::move(value),
                    0.0L,
                };
                return;
            }
            break;

        case State::Symbol:
            if (std::isalnum(m_Buffer))
            {
                value += static_cast<char>(m_Buffer);
                Get();
                break;
            }

            m_Token = {
                TokenType::Symbol,
                std::move(value),
                0.0L,
            };
            return;

        case State::Number:
            if (!floating && m_Buffer == '.')
            {
                floating = true;

                value += static_cast<char>(m_Buffer);
                Get();
                break;
            }

            if (std::isdigit(m_Buffer))
            {
                value += static_cast<char>(m_Buffer);
                Get();
                break;
            }
            {
                const auto number_value = std::stold(value);
                m_Token = {
                    TokenType::Number,
                    std::move(value),
                    number_value,
                };
            }
            return;

        case State::String:
            if (m_Buffer != '"')
            {
                if (m_Buffer == '\\')
                {
                    Get();
                    switch (m_Buffer)
                    {
                    case 'a':
                        m_Buffer = '\a';
                        break;
                    case 'b':
                        m_Buffer = '\b';
                        break;
                    case 'f':
                        m_Buffer = '\f';
                        break;
                    case 'n':
                        m_Buffer = '\n';
                        break;
                    case 'r':
                        m_Buffer = '\r';
                        break;
                    case 't':
                        m_Buffer = '\t';
                        break;
                    case 'v':
                        m_Buffer = '\v';
                        break;

                    case 'u':
                    {
                        char buffer[5];
                        for (unsigned i = 0; i < 4; ++i)
                        {
                            Get();
                            buffer[i] = static_cast<char>(m_Buffer);
                        }
                        buffer[4] = 0;
                        m_Buffer = std::stoi(buffer, nullptr, 0x10);
                        break;
                    }

                    case 'x':
                    {
                        char buffer[3];
                        for (unsigned i = 0; i < 2; ++i)
                        {
                            Get();
                            buffer[i] = static_cast<char>(m_Buffer);
                        }
                        buffer[2] = 0;
                        m_Buffer = std::stoi(buffer, nullptr, 0x10);
                        break;
                    }

                    default:
                        break;
                    }
                }

                value += static_cast<char>(m_Buffer);
                Get();
                break;
            }

            Get();
            m_Token = {
                TokenType::String,
                std::move(value),
                0.0L,
            };
            return;
        }
    }

    m_Token = {
        TokenType::EoF,
        std::move(value),
        0.0L,
    };
}

bool json::Parser::At(const TokenType type) const
{
    return m_Token.Type == type;
}

bool json::Parser::At(const TokenType type, const std::string &value) const
{
    return m_Token.Type == type && m_Token.String == value;
}

json::Token json::Parser::Skip()
{
    auto token = m_Token;
    Next();
    return token;
}

json::Token json::Parser::Expect(TokenType type)
{
    if (m_Token.Type == type)
    {
        return Skip();
    }
    throw std::runtime_error(
        std::format(
            "expect {}, got {}",
            type,
            m_Token.Type));
}

json::Token json::Parser::Expect(TokenType type, const std::string &value)
{
    if (m_Token.Type == type && m_Token.String == value)
    {
        return Skip();
    }
    throw std::runtime_error(
        std::format(
            "expect {} '{}', got {} '{}'",
            type,
            value,
            m_Token.Type,
            m_Token.String));
}
