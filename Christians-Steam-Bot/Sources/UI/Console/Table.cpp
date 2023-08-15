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

#include <cassert>

/************************************************************************/

typedef SteamBot::UI::TableBase TableBase;

/************************************************************************/

TableBase::LineBase::LineBase(size_t columnCount)
    : columns(columnCount)
{
}

/************************************************************************/

TableBase::TableBase(size_t columns)
    : columnCount(columns)
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

void TableBase::add(TableBase::LineBase& line)
{
    auto columns=line.getColumns();
    assert(columns.size()==columnCount);

    for (size_t i=0; i<columnCount; i++)
    {
        std::string string=std::move(columns[i].str());
        // ToDo: maybe add UTF-8 support
        if (widths[i]<string.size())
        {
            widths[i]=string.size();
        }
        fields.emplace_back(std::move(string));
    }

    assert(fields.size()%columnCount==0);
}

/************************************************************************/

bool TableBase::startLine()
{
    outputLine++;
    return (outputLine+1)*columnCount<=fields.size();
}

/************************************************************************/

bool TableBase::hasContent(size_t column) const
{
    for (size_t i=column; i<columnCount; i++)
    {
        if (!fields[outputLine*columnCount+i].empty())
        {
            return true;
        }
    }
    return false;
}

/************************************************************************/

const std::string& TableBase::getContent(size_t column) const
{
    assert(column<columnCount);
    return fields[outputLine*columnCount+column];
}

/************************************************************************/

std::string_view TableBase::getFiller(size_t column) const
{
    assert(column<columnCount);

    const size_t width=widths[column]-fields[outputLine*columnCount+column].size();
    assert((ssize_t)width>=0);
    if (width>filler.size())
    {
        filler.resize(width, ' ');
    }
    return std::string_view(filler.data(), width);
}
