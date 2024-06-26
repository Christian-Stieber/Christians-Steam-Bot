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

/************************************************************************/

#include "Startup.hpp"
#include "Client/ClientInfo.hpp"

#include <string_view>
#include <memory>
#include <istream>
#include <regex>
#include <concepts>

#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <boost/url/url.hpp>

/************************************************************************/

namespace SteamBot
{
    class ClientInfo;
}

/************************************************************************/

namespace SteamBot
{
    namespace UI
    {
        class CLI;
    }
}

/************************************************************************/

namespace SteamBot
{
    namespace UI
    {
        class CommandBase
        {
        public:
            virtual ~CommandBase() =default;

            virtual bool global() const =0;
            virtual const std::string_view& command() const =0;

            virtual const boost::program_options::options_description* options() const;
            virtual const boost::program_options::positional_options_description* positionals() const;

            virtual const std::string_view& description() const =0;			// short text: what does this command do

            bool parse(const std::vector<std::string>&, boost::program_options::variables_map&) const;

            void print(std::ostream&) const;

        public:
            // Note: execute() can be called on multiple clients concurrently!
            class ExecuteBase : public std::enable_shared_from_this<ExecuteBase>
            {
            public:
                SteamBot::UI::CLI& cli;

            public:
                ExecuteBase(SteamBot::UI::CLI& cli_)
                    : cli(cli_)
                {
                }

                virtual ~ExecuteBase() =default;

                virtual bool init(const boost::program_options::variables_map&)
                {
                    return true;
                }

                virtual void execute(SteamBot::ClientInfo*) const =0;

            public:
                template <std::derived_from<ExecuteBase> T> std::shared_ptr<const T> shared_from_this() const noexcept
                {
                    return std::dynamic_pointer_cast<const T>(std::enable_shared_from_this<ExecuteBase>::shared_from_this());
                }
            };

            virtual std::shared_ptr<ExecuteBase> makeExecute(SteamBot::UI::CLI&) const =0;

        public:
            template <typename T> using Init=SteamBot::Startup::Init<CommandBase, T>;
        };
    }
}

/************************************************************************/
/*
 * when using custom value types, boost program_options reads them
 * from istreams.
 */

/************************************************************************/

namespace SteamBot
{
    template <typename T> std::istream& operator>>(std::istream& stream, T& value) requires(std::is_enum_v<T>)
    {
        return stream >> reinterpret_cast<std::underlying_type_t<T>&>(value);
    }
}

/************************************************************************/

namespace SteamBot
{
    class OptionRegex : public std::regex
    {
    public:
        using std::regex::regex;

    public:
        OptionRegex& operator=(std::regex&& other)
        {
            std::regex::operator=(std::move(other));
            return *this;
        }
    };

    inline std::istream& operator>>(std::istream& stream, OptionRegex& value)
    {
        try
        {
            // https://stackoverflow.com/questions/3203452/how-to-read-entire-stream-into-a-stdstring
            value=std::regex(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>(), std::regex_constants::icase);
        }
        catch(const std::regex_error&)
        {
            // Note: on g++, the .what() is quite rubbish and much more confusing compared to having no detail information
            // ToDo: Visual Studio, don't know yet. See if we want to add it there.
            std::cout << "invalid regular expression" << std::endl;
            throw;
        }
        return stream;
    }
}

/************************************************************************/
/*
 * This class is for a game-matching that treats an all-numeric
 * string as an ID.
 *
 * ToDo: yes, I' aware that the appId-matching is a bit stupid since
 * we go through the entire game list to pick just one. We could
 * probably just request that one piece of data initially, since many
 * APIs have an appId-lookup already.
 * But, for now, this was a quick addition that didn't require a lot
 * lot of reorganizing in the command implementations.
 */

namespace SteamBot
{
    class OptionRegexID : public std::variant<std::monostate, std::regex, uint64_t>
    {
    public:
        bool doesMatch(const std::string&, uint64_t) const;

        template <typename T> bool doesMatch(const std::string& name, T id) const requires(std::is_enum_v<T>)
        {
            return doesMatch(name, boost::numeric_cast<uint64_t>(SteamBot::toInteger(id)));
        }

    public:
        OptionRegexID& operator=(uint64_t value)
        {
            std::variant<std::monostate, std::regex, uint64_t>::operator=(value);
            return *this;
        }

        OptionRegexID& operator=(std::regex value)
        {
            std::variant<std::monostate, std::regex, uint64_t>::operator=(std::move(value));
            return *this;
        }
    };

    std::istream& operator>>(std::istream&, OptionRegexID&);
}

/************************************************************************/

namespace SteamBot
{
    class OptionBotName
    {
    public:
        SteamBot::ClientInfo* clientInfo=nullptr;
    };

    inline std::istream& operator>>(std::istream& stream, OptionBotName& value)
    {
        std::string string{std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
        value.clientInfo=SteamBot::ClientInfo::find(string);
        if (value.clientInfo==nullptr)
        {
            std::cout << "unknown account \"" << string << "\"" << std::endl;
            throw false;
        }
        return stream;
    }
}

/************************************************************************/

namespace SteamBot
{
    class OptionURL : public boost::urls::url
    {
    public:
        using boost::urls::url::url;

    public:
        OptionURL& operator=(boost::urls::url&& other)
        {
            boost::urls::url::operator=(std::move(other));
            return *this;
        }
    };

    inline std::istream& operator>>(std::istream& stream, OptionURL& value)
    {
        try
        {
            // https://stackoverflow.com/questions/3203452/how-to-read-entire-stream-into-a-stdstring
            std::string string{std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
            value=boost::urls::url(string);
        }
        catch(const boost::system::system_error& error)
        {
            // Note: on g++, the .what() is quite rubbish and much more confusing compared to having no detail information
            // ToDo: Visual Studio, don't know yet. See if we want to add it there.
            std::cout << "invalid url: " << error.what() << std::endl;
            throw;
        }
        return stream;
    }
}
