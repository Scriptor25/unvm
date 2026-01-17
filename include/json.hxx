#pragma once

#include <format>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <variant>
#include <vector>

namespace json
{
    enum class NodeType
    {
        Null,
        Boolean,
        Number,
        String,
        Array,
        Object,
    };

    template<typename N>
    using NodeValue = std::variant<
        std::monostate,
        bool,
        long double,
        std::string,
        std::vector<N>,
        std::map<std::string, N>>;

    template<typename T>
    struct Converter;

    struct Node
    {
        template<typename T>
        static Node From(const T &value)
        {
            if (Node node; Converter<T>::From(node, value))
                return node;
            throw std::runtime_error("conversion failed");
        }

        template<typename T>
        T To() const
        {
            if (T value; Converter<T>::To(*this, value))
                return value;
            throw std::runtime_error("conversion failed");
        }

        [[nodiscard]] bool IsNull() const;
        [[nodiscard]] bool IsBoolean() const;
        [[nodiscard]] bool IsNumber() const;
        [[nodiscard]] bool IsString() const;
        [[nodiscard]] bool IsArray() const;
        [[nodiscard]] bool IsObject() const;

        [[nodiscard]] const bool &AsBoolean() const;
        [[nodiscard]] const long double &AsNumber() const;
        [[nodiscard]] const std::string &AsString() const;
        [[nodiscard]] const std::vector<Node> &AsArray() const;

        std::map<std::string, Node> &AsObject();
        [[nodiscard]] const std::map<std::string, Node> &AsObject() const;

        Node &Get(const std::string &key);
        [[nodiscard]] const Node &Get(const std::string &key) const;

        [[nodiscard]] Node GetOrNull(const std::string &key) const;

        std::ostream &Print(std::ostream &stream) const;

        NodeType Type{};
        NodeValue<Node> Value;
    };

    template<>
    struct Converter<bool>
    {
        static bool From(Node &node, bool value)
        {
            node.Type = NodeType::Boolean;
            node.Value = value;
            return true;
        }

        static bool To(const Node &node, bool &value)
        {
            if (!node.IsBoolean())
                return false;

            value = node.AsBoolean();
            return true;
        }
    };

    template<>
    struct Converter<long double>
    {
        static bool From(Node &node, long double value)
        {
            node.Type = NodeType::Number;
            node.Value = value;
            return true;
        }

        static bool To(const Node &node, long double &value)
        {
            if (!node.IsNumber())
                return false;

            value = node.AsNumber();
            return true;
        }
    };

    template<>
    struct Converter<std::string>
    {
        static bool From(Node &node, const std::string &value)
        {
            node.Type = NodeType::String;
            node.Value = value;
            return true;
        }

        static bool To(const Node &node, std::string &value)
        {
            if (!node.IsString())
                return false;

            value = node.AsString();
            return true;
        }
    };

    template<typename T>
    struct Converter<std::vector<T>>
    {
        static bool From(Node &node, const std::vector<T> &value)
        {
            std::vector<Node> array(value.size());
            for (std::size_t i = 0; i < array.size(); ++i)
                array[i] = Node::From(value.at(i));

            node.Type = NodeType::Array;
            node.Value = std::move(array);
            return true;
        }

        static bool To(const Node &node, std::vector<T> &value)
        {
            if (!node.IsArray())
                return false;

            auto &array = node.AsArray();

            value.resize(array.size());
            for (std::size_t i = 0; i < array.size(); ++i)
                value[i] = array.at(i).To<T>();

            return true;
        }
    };

    template<typename T>
    struct Converter<std::set<T>>
    {
        static bool From(Node &node, const std::set<T> &value)
        {
            std::vector<Node> array;
            for (auto &entry : value)
                array.emplace_back(Node::From(entry));

            node.Type = NodeType::Array;
            node.Value = std::move(array);
            return true;
        }

        static bool To(const Node &node, std::set<T> &value)
        {
            if (!node.IsArray())
                return false;

            auto &array = node.AsArray();

            value.clear();
            for (auto &entry : array)
                value.emplace(entry.To<T>());

            return true;
        }
    };

    template<typename T>
    struct Converter<std::optional<T>>
    {
        static bool From(Node &node, const std::optional<T> &value)
        {
            if (value.has_value())
            {
                node = Node::From(value.value());
            }
            else
            {
                node.Type = NodeType::Null;
                node.Value = {};
            }

            return true;
        }

        static bool To(const Node &node, std::optional<T> &value)
        {
            if (node.IsNull())
            {
                value = std::nullopt;
            }
            else
            {
                value = node.To<T>();
            }
            return true;
        }
    };

    enum class TokenType
    {
        EoF,
        Symbol,
        Number,
        String,
        Other,
    };

    struct Token
    {
        TokenType Type;

        std::string String;
        long double Number;
    };

    class Parser
    {
    public:
        static Node Parse(std::istream &stream);
        static Node Parse(const std::string &string);
        static Node Parse(std::string &&string);

        template<typename Iterator>
        static Node Parse(Iterator begin, Iterator end)
        {
            return Parse({ begin, end });
        }

        explicit Parser(std::istream &stream);

        Node Parse();

    private:
        void Get();
        void Next();

        [[nodiscard]] bool At(TokenType type) const;
        [[nodiscard]] bool At(TokenType type, const std::string &value) const;

        Token Skip();

        Token Expect(TokenType type);
        Token Expect(TokenType type, const std::string &value);

    private:
        std::istream &m_Stream;
        int m_Buffer = -1;
        Token m_Token;
    };
}

inline std::ostream &operator<<(std::ostream &stream, const json::NodeType type)
{
    static const std::map<json::NodeType, const char *> map = {
        { json::NodeType::Null, "Null" },
        { json::NodeType::Boolean, "Boolean" },
        { json::NodeType::Number, "Number" },
        { json::NodeType::String, "String" },
        { json::NodeType::Array, "Array" },
        { json::NodeType::Object, "Object" },
    };
    return stream << map.at(type);
}

inline std::ostream &operator<<(std::ostream &stream, const json::TokenType type)
{
    static const std::map<json::TokenType, const char *> map = {
        { json::TokenType::EoF, "EoF" },
        { json::TokenType::Symbol, "Symbol" },
        { json::TokenType::Number, "Number" },
        { json::TokenType::String, "String" },
        { json::TokenType::Other, "Other" },
    };
    return stream << map.at(type);
}

template<typename C>
struct std::formatter<json::TokenType, C>
{
    constexpr auto parse(std::basic_format_parse_context<C> &ctx)
    {
        return ctx.begin();
    }

    template<typename Context>
    auto format(json::TokenType type, Context &ctx) const
    {
        static const std::map<json::TokenType, const char *> map = {
            { json::TokenType::EoF, "EoF" },
            { json::TokenType::Symbol, "Symbol" },
            { json::TokenType::Number, "Number" },
            { json::TokenType::String, "String" },
            { json::TokenType::Other, "Other" },
        };

        return std::format_to(ctx.out(), "{}", map.at(type));
    }
};

inline std::ostream &operator<<(std::ostream &stream, const json::Node &node)
{
    return node.Print(stream);
}

inline std::istream &operator>>(std::istream &stream, json::Node &node)
{
    node = json::Parser::Parse(stream);
    return stream;
}
