#include <semver.hxx>

#include <iostream>
#include <map>
#include <sstream>

semver::Parser::Parser(std::istream &stream)
    : m_Stream(stream)
{
    m_Buffer = m_Stream.get();
    m_Token = Next();
}

semver::RangeSet semver::Parser::Parse()
{
    RangeSet set;

    do
    {
        Skip(" ");

        auto begin = ParsePrimitive();

        Skip(" ");

        if (begin.Type == PrimitiveType::Equal && Skip("-"))
        {
            Skip(" ");

            auto end = ParsePartial();

            set.emplace_back(
                Hyphen{
                    .Begin = std::move(begin.Value),
                    .End = std::move(end),
                });
        }
        else
        {
            PrimitiveSet primitives;
            primitives.push_back(std::move(begin));

            while (!At("||") && !At(""))
            {
                primitives.push_back(ParsePrimitive());
                Skip(" ");
            }

            set.emplace_back(std::move(primitives));
        }

        Skip(" ");
    }
    while (Skip("||"));

    return set;
}

semver::Primitive semver::Parser::ParsePrimitive()
{
    static const std::map<std::string_view, PrimitiveType> map = {
        { "=", PrimitiveType::Equal },
        { "<", PrimitiveType::LessThan },
        { "<=", PrimitiveType::LessThanOrEqual },
        { ">", PrimitiveType::GreaterThan },
        { ">=", PrimitiveType::GreaterThanOrEqual },
        { "~", PrimitiveType::Tilde },
        { "^", PrimitiveType::Caret },
    };

    PrimitiveType type{};
    if (At("=", "<", "<=", ">", ">=", "~", "^"))
    {
        auto op = m_Token;
        m_Token = Next();

        type = map.at(op);
    }

    auto value = ParsePartial();

    return {
        .Type = type,
        .Value = std::move(value),
    };
}

semver::Partial semver::Parser::ParsePartial()
{
    Partial partial;

    if (ParsePossibleWildcard(partial.Value.Major))
    {
        partial.Mod |= 0b001;
    }

    partial.Mask = 0;
    if (!Skip("."))
    {
        return partial;
    }

    if (ParsePossibleWildcard(partial.Value.Minor))
    {
        partial.Mod |= 0b010;
    }

    partial.Mask = 1;
    if (!Skip("."))
    {
        return partial;
    }

    if (ParsePossibleWildcard(partial.Value.Patch))
    {
        partial.Mod |= 0b100;
    }

    partial.Mask = 2;
    if (!At("-", "+"))
    {
        return partial;
    }

    if (Skip("-"))
    {
        std::vector<std::string> value;
        do
        {
            value.push_back(std::move(m_Token));
            m_Token = Next();
        }
        while (Skip("."));

        partial.Value.PreRelease = std::move(value);
    }

    if (Skip("+"))
    {
        std::vector<std::string> value;
        do
        {
            value.push_back(std::move(m_Token));
            m_Token = Next();
        }
        while (Skip("."));

        partial.Value.Build = std::move(value);
    }

    return partial;
}

semver::Version semver::Parser::ParseVersion()
{
    Version version;

    ParseVersionPart(version.Major);
    Expect(".");

    ParseVersionPart(version.Minor);
    Expect(".");

    ParseVersionPart(version.Patch);
    if (!At("-", "+"))
    {
        return version;
    }

    if (Skip("-"))
    {
        std::vector<std::string> value;
        do
        {
            value.push_back(std::move(m_Token));
            m_Token = Next();
        }
        while (Skip("."));

        version.PreRelease = std::move(value);
    }

    if (Skip("+"))
    {
        std::vector<std::string> value;
        do
        {
            value.push_back(std::move(m_Token));
            m_Token = Next();
        }
        while (Skip("."));

        version.Build = std::move(value);
    }

    return version;
}

bool semver::Parser::ParsePossibleWildcard(std::uint32_t &value)
{
    if (Skip("x", "X", "*"))
    {
        return true;
    }

    ParseVersionPart(value);
    return false;
}

void semver::Parser::ParseVersionPart(std::uint32_t &value)
{
    auto token = std::move(m_Token);
    m_Token = Next();

    if (token.front() == 'v')
    {
        token = token.substr(1);
    }

    value = std::stoul(token);
}

bool semver::Parser::At(const std::set<std::string_view> &set) const
{
    for (auto &e : set)
    {
        if (m_Token == e)
        {
            return true;
        }
    }
    return false;
}

bool semver::Parser::Skip(const std::set<std::string_view> &set)
{
    if (At(set))
    {
        m_Token = Next();
        return true;
    }

    return false;
}

std::string semver::Parser::Expect(const std::set<std::string_view> &set)
{
    if (At(set))
    {
        auto token = std::move(m_Token);
        m_Token = Next();
        return token;
    }

    throw std::runtime_error("expect");
}

std::string semver::Parser::Next()
{
    enum class State
    {
        None,
        Space,
        Number,
        String,
        Operator,
    };

    State state{};
    std::string value;

    while (m_Buffer >= 0)
    {
        switch (state)
        {
        case State::None:
            switch (m_Buffer)
            {
            case '=':
            case '<':
            case '>':
            case '|':
                state = State::Operator;
                break;

            case '~':
            case '^':
            case '+':
            case '-':
            case '*':
            case '.':
                value += static_cast<char>(m_Buffer);
                m_Buffer = m_Stream.get();
                return value;

            default:
                if (std::isspace(m_Buffer))
                {
                    state = State::Space;
                    break;
                }

                if (std::isdigit(m_Buffer))
                {
                    state = State::Number;
                    break;
                }

                if (std::isalpha(m_Buffer))
                {
                    state = State::String;
                    break;
                }

                std::cerr << "invalid character '" << m_Buffer << "' in stream." << std::endl;
                throw std::runtime_error("who da hell is drivin dis bus - next");
            }
            break;

        case State::Space:
            if (std::isspace(m_Buffer))
            {
                m_Buffer = m_Stream.get();
                break;
            }
            return " ";

        case State::Number:
            if (std::isdigit(m_Buffer))
            {
                value += static_cast<char>(m_Buffer);
                m_Buffer = m_Stream.get();
                break;
            }
            return value;

        case State::String:
            if (std::isalnum(m_Buffer) || m_Buffer == '_')
            {
                value += static_cast<char>(m_Buffer);
                m_Buffer = m_Stream.get();
                break;
            }
            return value;

        case State::Operator:
            if (m_Buffer == '=' || m_Buffer == '<' || m_Buffer == '>' || m_Buffer == '|')
            {
                value += static_cast<char>(m_Buffer);
                m_Buffer = m_Stream.get();
                break;
            }
            return value;
        }
    }

    return value;
}

semver::RangeSet semver::ParseRangeSet(std::istream &stream)
{
    Parser parser(stream);
    return parser.Parse();
}

semver::RangeSet semver::ParseRangeSet(const std::string_view string)
{
    const std::string s(string);
    std::istringstream stream(s);
    return ParseRangeSet(stream);
}

bool semver::IsInRange(const RangeSet &set, const std::string_view version)
{
    const std::string s(version);
    std::istringstream stream(s);
    Parser parser(stream);
    const auto parsed = parser.ParseVersion();
    return IsInRange(set, parsed);
}

static semver::Partial normalize_partial(const semver::Partial &partial)
{
    return {
        .Value = {
            .Major = (partial.Mod & 0b001) ? 0u : partial.Value.Major,
            .Minor = partial.Mask >= 1 && (partial.Mod & 0b010) ? 0u : partial.Value.Minor,
            .Patch = partial.Mask >= 2 && (partial.Mod & 0b100) ? 0u : partial.Value.Patch,
        },
        .Mask = 2,
        .Mod = 0,
    };
}

bool semver::IsInRange(const RangeSet &set, const Version &version)
{
    Partial partial;
    partial.Value = version;
    partial.Mask = 2;
    partial.Mod = 0;

    for (auto &range : set)
    {
        if (const auto hyphen = std::get_if<Hyphen>(&range))
        {
            auto &begin = hyphen->Begin;
            auto &end = hyphen->End;

            if (begin <= partial && partial <= end)
            {
                return true;
            }

            continue;
        }

        if (const auto primitive_set = std::get_if<PrimitiveSet>(&range))
        {
            auto match = true;
            for (auto &primitive : *primitive_set)
            {
                switch (primitive.Type)
                {
                case PrimitiveType::Equal:
                    match &= (partial == primitive.Value);
                    break;

                case PrimitiveType::LessThan:
                    match &= (partial < primitive.Value);
                    break;

                case PrimitiveType::LessThanOrEqual:
                    match &= (partial <= primitive.Value);
                    break;

                case PrimitiveType::GreaterThan:
                    match &= (partial > primitive.Value);
                    break;

                case PrimitiveType::GreaterThanOrEqual:
                    match &= (partial >= primitive.Value);
                    break;

                case PrimitiveType::Tilde:
                {
                    auto &lower = primitive.Value;

                    if (partial < lower)
                    {
                        match = false;
                        break;
                    }

                    Partial upper{};
                    upper.Mask = 2;

                    if (lower.Mask == 0)
                    {
                        upper.Value.Major = lower.Value.Major + 1;
                    }
                    else
                    {
                        upper.Value.Major = lower.Value.Major;
                        upper.Value.Minor = lower.Value.Minor + 1;
                    }

                    if (partial >= upper)
                    {
                        match = false;
                        break;
                    }

                    break;
                }

                case PrimitiveType::Caret:
                {
                    auto &lower = primitive.Value;

                    if (partial < lower)
                    {
                        match = false;
                        break;
                    }

                    Partial upper{};
                    upper.Mask = 2;

                    if (lower.Value.Major > 0)
                    {
                        upper.Value.Major = lower.Value.Major + 1;
                    }
                    else if (lower.Value.Minor > 0)
                    {
                        upper.Value.Major = 0;
                        upper.Value.Minor = lower.Value.Minor + 1;
                    }
                    else
                    {
                        upper.Value.Major = 0;
                        upper.Value.Minor = 0;
                        upper.Value.Patch = lower.Value.Patch + 1;
                    }

                    if (partial >= upper)
                    {
                        match = false;
                        break;
                    }

                    break;
                }

                default:
                    throw std::runtime_error("who da hell is drivin dis bus - is in range - invalid primitive type");
                }

                if (!match)
                {
                    break;
                }
            }

            if (match)
            {
                return true;
            }

            continue;
        }

        throw std::runtime_error("who da hell is drivin dis bus - is in range - invalid range type");
    }

    return false;
}

bool semver::operator==(const Partial &a, const Partial &b)
{
    auto precision = std::min(a.Mask, b.Mask);

    if (!(a.Mod & 0b001) && !(b.Mod & 0b001) && a.Value.Major != b.Value.Major)
    {
        return false;
    }

    if (precision >= 1 && !(a.Mod & 0b010) && !(b.Mod & 0b010) && a.Value.Minor != b.Value.Minor)
    {
        return false;
    }

    if (precision >= 2 && !(a.Mod & 0b100) && !(b.Mod & 0b100) && a.Value.Patch != b.Value.Patch)
    {
        return false;
    }

    return true;
}

static bool is_numeric(const std::string &str)
{
    if (str.empty())
    {
        return false;
    }

    for (auto &c : str)
    {
        if (!std::isdigit(static_cast<unsigned char>(c)))
        {
            return false;
        }
    }

    return true;
}

static int compare_pre_release(const std::vector<std::string> &a, const std::vector<std::string> &b)
{
    if (a == b)
    {
        return 0;
    }

    const auto count = std::min(a.size(), b.size());

    for (std::size_t i = 0; i < count; ++i)
    {
        auto &a_entry = a.at(i);
        auto &b_entry = b.at(i);

        const auto a_numeric = is_numeric(a_entry);
        const auto b_numeric = is_numeric(b_entry);

        if (a_numeric && b_numeric)
        {
            const auto a_value = std::stoul(a_entry);
            const auto b_value = std::stoul(b_entry);

            if (a_value < b_value)
            {
                return -1;
            }

            if (a_value > b_value)
            {
                return 1;
            }
        }
        else if (a_numeric)
        {
            return -1;
        }
        else if (b_numeric)
        {
            return 1;
        }
        else
        {
            if (a_entry < b_entry)
            {
                return -1;
            }

            if (a_entry > b_entry)
            {
                return 1;
            }
        }
    }

    if (a.size() < b.size())
    {
        return -1;
    }

    if (a.size() > b.size())
    {
        return 1;
    }

    return 0;
}

bool semver::operator<(const Partial &a, const Partial &b)
{
    const auto precision = std::min(a.Mask, b.Mask);

    const auto a_major = (a.Mod & 0b001) ? 0u : a.Value.Major;
    const auto b_major = (b.Mod & 0b001) ? 0u : b.Value.Major;

    if (a_major != b_major)
    {
        return a_major < b_major;
    }

    if (precision >= 1)
    {
        const auto a_minor = (a.Mod & 0b010) ? 0u : a.Value.Minor;
        const auto b_minor = (b.Mod & 0b010) ? 0u : b.Value.Minor;

        if (a_minor != b_minor)
        {
            return a_minor < b_minor;
        }
    }

    if (precision >= 2)
    {
        const auto a_patch = (a.Mod & 0b100) ? 0u : a.Value.Patch;
        const auto b_patch = (b.Mod & 0b100) ? 0u : b.Value.Patch;

        if (a_patch != b_patch)
        {
            return a_patch < b_patch;
        }

        const auto wildcard = (a.Mod & 0b100) || (b.Mod & 0b100);

        if (!wildcard)
        {
            auto &a_pre = a.Value.PreRelease;
            auto &b_pre = b.Value.PreRelease;

            const auto a_empty = a_pre.empty();
            const auto b_empty = b_pre.empty();

            if (a_empty && !b_empty)
            {
                return false;
            }

            if (!a_empty && b_empty)
            {
                return true;
            }

            if (!a_empty && !b_empty)
            {
                const auto cmp = compare_pre_release(a_pre, b_pre);
                if (cmp != 0)
                {
                    return cmp < 0;
                }
            }
        }
    }

    return false;
}

bool semver::operator<=(const Partial &a, const Partial &b)
{
    return !(b < a);
}

bool semver::operator>(const Partial &a, const Partial &b)
{
    return b < a;
}

bool semver::operator>=(const Partial &a, const Partial &b)
{
    return !(a < b);
}
