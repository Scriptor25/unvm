#include <table.hxx>

#include <iomanip>
#include <iostream>

Table &Table::operator<<(std::string &&entry)
{
    auto index = m_Entries.size() % m_Columns.size();

    m_Entries.push_back(entry);

    auto &width = m_Columns[index].Width;
    width = std::max(width, m_Entries.back().length());

    return *this;
}

Table &Table::operator<<(const std::string &entry)
{
    auto index = m_Entries.size() % m_Columns.size();

    m_Entries.push_back(entry);

    auto &width = m_Columns[index].Width;
    width = std::max(width, m_Entries.back().length());

    return *this;
}

bool Table::Empty() const
{
    return m_Entries.empty();
}

std::ostream &Table::Print(std::ostream &stream) const
{
    for (auto &column : m_Columns)
    {
        stream << std::setw(column.Width);
        if (column.Left)
            stream << std::left;
        else
            stream << std::right;
        stream << column.Label << ' ';
    }
    stream << std::endl;

    for (std::size_t j = 0; j < m_Entries.size(); j += m_Columns.size())
    {
        for (std::size_t i = 0; i < m_Columns.size(); ++i)
        {
            auto &column = m_Columns.at(i);
            stream << std::setw(column.Width);
            if (column.Left)
                stream << std::left;
            else
                stream << std::right;
            stream << m_Entries.at(j + i) << ' ';
        }
        stream << std::endl;
    }

    return stream;
}
