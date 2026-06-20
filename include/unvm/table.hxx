#pragma once

#include <ostream>
#include <string>
#include <vector>

namespace unvm
{
    class Table final
    {
    public:
        struct Column
        {
            Column(std::string label, const bool left)
                : Label(std::move(label)),
                  Width(Label.length()),
                  Left(left)
            {
            }

            std::string Label;
            size_t Width;
            bool Left;
        };

        Table() = default;

        void Init(std::vector<Column> columns);

        explicit Table(std::vector<Column> columns);

        Table &operator<<(std::string &&entry);
        Table &operator<<(const std::string &entry);

        [[nodiscard]] bool Empty() const;

        std::ostream &Print(std::ostream &stream) const;

    private:
        std::vector<Column> m_Columns;
        std::vector<std::string> m_Entries;
    };
}

inline std::ostream &operator<<(std::ostream &stream, const unvm::Table &table)
{
    return table.Print(stream);
}
