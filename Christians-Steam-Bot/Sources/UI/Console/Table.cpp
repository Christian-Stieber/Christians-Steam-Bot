/*
 * This file is part of "Christians-Steam-Framework"
 * Copyright (C) 2023- Christian Stieber
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file LICENSE.  If not see
 * <http://www.gnu.org/licenses/>.
 */

#include "UI/Table.hpp"

#include "Helpers/StringCompare.hpp"

#include <cassert>
#include <algorithm>

#ifdef _WIN32
typedef std::make_signed_t<size_t> ssize_t;
#endif

/************************************************************************/

typedef SteamBot::UI::TableBase TableBase;

/************************************************************************/

TableBase::LineBase::LineBase(size_t columnCount)
    : columns(columnCount)
{
}

/************************************************************************/

TableBase::TableBase(size_t columnCount)
    : widths(columnCount, 0)
{
}

/************************************************************************/

TableBase::LineBase::~LineBase() =default;
TableBase::~TableBase() =default;

/************************************************************************/

std::ostringstream& TableBase::LineBase::operator[](size_t column)
{
    assert(column<columns.size());
    return columns[column];
}

/************************************************************************/

std::vector<std::string> TableBase::LineBase::getColumns()
{
    std::vector<std::string> result;
    result.reserve(columns.size());
    for (auto& column : columns)
    {
        result.emplace_back(std::move(column.str()));
    }
    return result;
}

/************************************************************************/

void TableBase::add(TableBase::LineBase& line)
{
    fields.emplace_back(line.getColumns());

    auto& columns=fields.back();
    assert(columns.size()==widths.size());

    for (size_t i=0; i<columns.size(); i++)
    {
        // ToDo: support UTF-8
        const auto length=columns[i].size();
        if (widths[i]<length) widths[i]=length;
    }
}

/************************************************************************/

bool TableBase::startLine()
{
    outputLine++;
    return outputLine<fields.size();
}

/************************************************************************/

const std::string& TableBase::getField(size_t line, size_t column) const
{
    assert(line<fields.size() && column<widths.size());
    return fields[line][column];
}

/************************************************************************/

bool TableBase::hasContent(size_t column) const
{
    const auto& line=fields[outputLine];
    for (; column<widths.size(); column++)
    {
        if (!line[column].empty())
        {
            return true;
        }
    }
    return false;
}

/************************************************************************/

const std::string& TableBase::getContent(size_t column) const
{
    assert(column<widths.size());
    return fields[outputLine][column];
}

/************************************************************************/

std::string_view TableBase::getFiller(size_t column) const
{
    assert(column<widths.size());

    const size_t width=widths[column]-fields[outputLine][column].size();
    assert((ssize_t)width>=0);
    if (width>filler.size())
    {
        filler.resize(width, ' ');
    }
    return std::string_view(filler.data(), width);
}

/************************************************************************/

void TableBase::sort(size_t column)
{
    assert(column<widths.size());
    std::sort(fields.begin(), fields.end(), [column](const std::vector<std::string>& left, const std::vector<std::string>& right) {
        return SteamBot::caseInsensitiveStringCompare_less(left[column], right[column]);
    });
}
