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

#pragma once

#include <vector>
#include <sstream>

/************************************************************************/

namespace SteamBot
{
    namespace UI
    {
        class TableBase
        {
        public:
            class LineBase;

        private:
            const size_t columnCount;
            std::vector<size_t> widths{columnCount, 0};
            std::vector<std::string> fields;

        private:
            size_t outputLine=(size_t)(-1);
            mutable std::string filler;

        protected:
            TableBase(size_t);
            ~TableBase();

        protected:
            void add(LineBase&);

        protected:
            // checks whether any column starting at the indicated one has content
            bool hasContent(size_t) const;

            // returns the content of the column
            const std::string& getContent(size_t) const;

            // returns spaces to fill the column
            std::string_view getFiller(size_t) const;

        public:
            bool startLine();
        };
    }
}

/************************************************************************/

class SteamBot::UI::TableBase::LineBase
{
private:
    std::vector<std::ostringstream> columns;

protected:
    LineBase(size_t);
    ~LineBase();

protected:
    std::ostringstream& operator[](size_t);

public:
    // Doing it like this makes the columns empty, which will cause problems
    // if we try to use this Line again.
    decltype(columns) getColumns()
    {
        return std::move(columns);
    }
};

/************************************************************************/
/*
 * This is the main Table class. You'll have to provide an enum with
 * column names, followed by a "Max" value.
 */

namespace SteamBot
{
    namespace UI
    {
        template <typename T> concept TableColumns=requires(T) {
            static_cast<std::underlying_type_t<T>>(T::Max);
        };

        template <TableColumns T> class Table : public TableBase {
        public:
            class Line : public LineBase
            {
            public:
                Line()
                    : LineBase(static_cast<std::underlying_type_t<T>>(T::Max))
                {
                }

                ~Line() =default;

            public:
                std::ostringstream& operator[](T column)
                {
                    return this->LineBase::operator[](static_cast<std::underlying_type_t<T>>(column));
                }
            };

        public:
            Table()
                : TableBase(static_cast<std::underlying_type_t<T>>(T::Max))
            {
            }

            ~Table() =default;

        public:
            void add(Line& line)
            {
                TableBase::add(line);
            }

        public:
            bool hasContent(T column) const
            {
                return TableBase::hasContent(static_cast<std::underlying_type_t<T>>(column));
            }

            const std::string& getContent(T column) const
            {
                return TableBase::getContent(static_cast<std::underlying_type_t<T>>(column));
            }

            std::string_view getFiller(T column) const
            {
                return TableBase::getFiller(static_cast<std::underlying_type_t<T>>(column));
            }
        };
    }
}
