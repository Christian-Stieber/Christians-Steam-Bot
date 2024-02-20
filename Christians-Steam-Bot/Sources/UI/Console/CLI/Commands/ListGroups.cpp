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

#include "UI/CLI.hpp"
#include "UI/Command.hpp"

#include "Helpers/JSON.hpp"

/************************************************************************/

namespace
{
    class ListGroupsCommand : public SteamBot::UI::CommandBase
    {
    public:
        virtual bool global() const
        {
            return true;
        }

        virtual const std::string_view& command() const override
        {
            static const std::string_view string("list-groups");
            return string;
        }

        virtual const std::string_view& description() const override
        {
            static const std::string_view string("list groups");
            return string;
        }

    public:
        class Execute : public ExecuteBase
        {
        public:
            using ExecuteBase::ExecuteBase;

            virtual ~Execute() =default;

        public:
            virtual void execute(SteamBot::ClientInfo*) const override;
        };

        virtual std::shared_ptr<ExecuteBase> makeExecute(SteamBot::UI::CLI& cli) const override
        {
            return std::make_shared<Execute>(cli);
        }
    };

    ListGroupsCommand::Init<ListGroupsCommand> init;
}

/************************************************************************/

void ListGroupsCommand::Execute::execute(SteamBot::ClientInfo*) const
{
    std::unordered_map<std::string, std::vector<const SteamBot::ClientInfo*>> groups;

    auto clients=SteamBot::ClientInfo::getClients();
    for (const SteamBot::ClientInfo* info : clients)
    {
        auto& dataFile=SteamBot::DataFile::get(info->accountName, SteamBot::DataFile::FileType::Account);
        dataFile.examine([info, &groups](const boost::json::value& json) {
            if (auto array=SteamBot::JSON::getItem(json, "Groups"))
            {
                for (const auto& group : array->as_array())
                {
                    groups[group.as_string().subview()].push_back(info);
                }
            }
        });
    }

    for (const auto& group : groups)
    {
        std::cout << "@" << group.first << ":";
        for (const SteamBot::ClientInfo* info : group.second)
        {
            std::cout << " " << info->accountName;
        }
        std::cout << std::endl;
    }
}
