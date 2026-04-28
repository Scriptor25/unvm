#pragma once

#include <toolkit/result.hxx>

#include <cstdint>
#include <istream>
#include <set>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

template<typename T>
concept string_type = std::is_convertible_v<T, std::string_view>;

namespace unvm::semver
{
    // range-set  ::= range ( logical-or range ) *
    // logical-or ::= ( ' ' ) * '||' ( ' ' ) *
    // range      ::= hyphen | simple ( ' ' simple ) * | ''
    // hyphen     ::= partial ' - ' partial
    // simple     ::= primitive | partial
    // primitive  ::= ( '<' | '>' | '>=' | '<=' | '=' | '~' | '^' ) partial
    // partial    ::= xr ( '.' xr ( '.' xr qualifier ? )? )?
    // xr         ::= 'x' | 'X' | '*' | nr
    // nr         ::= '0' | ['1'-'9'] ( ['0'-'9'] ) *
    // qualifier  ::= ( '-' pre )? ( '+' build )?
    // pre        ::= parts
    // build      ::= parts
    // parts      ::= part ( '.' part ) *
    // part       ::= nr | [-0-9A-Za-z]+

    struct Version
    {
        std::uint32_t Major{};
        std::uint32_t Minor{};
        std::uint32_t Patch{};

        std::vector<std::string> PreRelease;
        std::vector<std::string> Build;
    };

    struct Partial
    {
        Version Value;
        unsigned Mask{}; // 0 - <major>, 1 - <major>.<minor>, 2 - <major>.<minor>.<patch>[-<pre-release>][+<build>]
        unsigned Mod{};  // if set: wildcard; bit 0 - <major>, bit 1 - <minor>, bit 2 - <patch>
    };

    enum class PrimitiveType
    {
        Equal,

        LessThan,
        LessThanOrEqual,

        GreaterThan,
        GreaterThanOrEqual,

        Tilde,
        Caret,
    };

    struct Primitive
    {
        PrimitiveType Type{};
        Partial Value;
    };

    using PrimitiveSet = std::vector<Primitive>;

    struct Hyphen
    {
        Partial Begin;
        Partial End;
    };

    using Range = std::variant<Hyphen, PrimitiveSet>;
    using RangeSet = std::vector<Range>;

    class Parser final
    {
    public:
        explicit Parser(std::istream &stream);

        toolkit::result<RangeSet> Parse();

        toolkit::result<Primitive> ParsePrimitive();
        toolkit::result<Partial> ParsePartial();
        toolkit::result<Version> ParseVersion();

        toolkit::result<bool> ParsePossibleWildcard(std::uint32_t &value);
        toolkit::result<> ParseVersionPart(std::uint32_t &value);

        [[nodiscard]] bool At(const std::set<std::string_view> &set) const;

        template<string_type... S>
        bool At(S... s) const
        {
            return At(std::set<std::string_view>{ s... });
        }

        bool Skip(const std::set<std::string_view> &set);

        template<string_type... S>
        bool Skip(S... s)
        {
            return Skip(std::set<std::string_view>{ s... });
        }

        toolkit::result<std::string> Expect(const std::set<std::string_view> &set);

        template<string_type... S>
        toolkit::result<std::string> Expect(S... s)
        {
            return Expect(std::set<std::string_view>{ s... });
        }

        std::string Next();

    private:
        std::istream &m_Stream;
        int m_Buffer;
        
        std::string m_Token;
    };

    toolkit::result<RangeSet> ParseRangeSet(std::istream &stream);
    toolkit::result<RangeSet> ParseRangeSet(std::string_view str);
    toolkit::result<RangeSet> ParseRangeSet(const std::string &str);

    toolkit::result<bool> IsInRange(const RangeSet &set, std::string_view version);
    toolkit::result<bool> IsInRange(const RangeSet &set, const std::string &version);
    bool IsInRange(const RangeSet &set, const Version &version);

    bool operator==(const Partial &a, const Partial &b);
    bool operator<(const Partial &a, const Partial &b);
    bool operator<=(const Partial &a, const Partial &b);
    bool operator>(const Partial &a, const Partial &b);
    bool operator>=(const Partial &a, const Partial &b);
}
