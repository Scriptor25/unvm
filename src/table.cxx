#include <unvm/table.hxx>

#include <iomanip>
#include <iostream>

void unvm::Table::Init(std::vector<Column> columns)
{
    m_Columns = std::move(columns);
}

unvm::Table::Table(std::vector<Column> columns)
    : m_Columns(std::move(columns))
{
}

unvm::Table &unvm::Table::operator<<(std::string &&entry)
{
    const auto index = m_Entries.size() % m_Columns.size();

    m_Entries.push_back(std::move(entry));

    auto &column = m_Columns[index];
    const auto &back = m_Entries.back();

    column.Width = std::max(column.Width, back.size());
    column.Empty &= back.empty();

    return *this;
}

unvm::Table &unvm::Table::operator<<(const std::string &entry)
{
    const auto index = m_Entries.size() % m_Columns.size();

    m_Entries.push_back(entry);

    auto &column = m_Columns[index];
    const auto &back = m_Entries.back();

    column.Width = std::max(column.Width, back.size());
    column.Empty &= back.empty();

    return *this;
}

bool unvm::Table::Empty() const
{
    return m_Entries.empty();
}

std::ostream &unvm::Table::Print(std::ostream &stream) const
{
    for (auto &column : m_Columns)
    {
        if (column.Empty)
        {
            continue;
        }

        stream << std::setw(static_cast<int>(column.Width));
        if (column.Left)
        {
            stream << std::left;
        }
        else
        {
            stream << std::right;
        }
        stream << column.Label << "  ";
    }
    stream << std::endl;

    for (size_t j = 0; j < m_Entries.size(); j += m_Columns.size())
    {
        for (size_t i = 0; i < m_Columns.size(); ++i)
        {
            auto &column = m_Columns[i];
            if (column.Empty)
            {
                continue;
            }

            stream << std::setw(static_cast<int>(column.Width));
            if (column.Left)
            {
                stream << std::left;
            }
            else
            {
                stream << std::right;
            }
            stream << m_Entries[j + i] << "  ";
        }
        stream << std::endl;
    }

    return stream;
}
