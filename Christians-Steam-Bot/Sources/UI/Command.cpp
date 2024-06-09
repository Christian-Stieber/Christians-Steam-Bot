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

/************************************************************************/

#include "UI/Command.hpp"

#include "Helpers/ParseNumber.hpp"

/************************************************************************/

bool SteamBot::OptionRegexID::doesMatch(const std::string& name, uint64_t const ID) const
{
    if (const uint64_t* const valueId=std::get_if<uint64_t>(this))
    {
        return *valueId==ID;
    }
    else if (const std::regex* const valueRegex=std::get_if<std::regex>(this))
    {
        return std::regex_search(name, *valueRegex);
    }
    return false;
}

/************************************************************************/

std::istream& SteamBot::operator>>(std::istream& stream, SteamBot::OptionRegexID& value)
{
    // https://stackoverflow.com/questions/3203452/how-to-read-entire-stream-into-a-stdstring
    std::string string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>{});

    uint64_t valueId;
    if (SteamBot::parseNumber(string, valueId))
    {
        value=valueId;
    }
    else
    {
        try
        {
            value=std::regex(string, std::regex_constants::icase);
        }
        catch(const std::regex_error&)
        {
            // Note: on g++, the .what() is quite rubbish and much more confusing compared to having no detail information
            // ToDo: Visual Studio, don't know yet. See if we want to add it there.
            std::cout << "invalid regular expression" << std::endl;
            throw;
        }
    }
    return stream;
}
