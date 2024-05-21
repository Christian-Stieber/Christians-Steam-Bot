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
#include "UI/Table.hpp"

#include "Client/Client.hpp"
#include "Modules/Executor.hpp"
#include "Modules/PlayGames.hpp"
#include "Modules/Login.hpp"
#include "Modules/OwnedGames.hpp"

/************************************************************************/

namespace
{
    class StatusCommand : public SteamBot::UI::CommandBase
    {
    public:
        virtual bool global() const
        {
            return true;
        }

        virtual const std::string_view& command() const override
        {
            static const std::string_view string("status");
            return string;
        }

        virtual const std::string_view& description() const override
        {
            static const std::string_view string("show status of bot accounts");
            return string;
        }

    public:
        class Execute : public ExecuteBase
        {
        public:
            using ExecuteBase::ExecuteBase;

            virtual ~Execute() =default;

        public:
            virtual void execute(SteamBot::ClientInfo*) const;
        };

        virtual std::shared_ptr<ExecuteBase> makeExecute(SteamBot::UI::CLI& cli) const override
        {
            return std::make_shared<Execute>(cli);
        }
    };

    StatusCommand::Init<StatusCommand> init;
}

/************************************************************************/

void StatusCommand::Execute::execute(SteamBot::ClientInfo*) const
{
    enum class Columns : unsigned int { Account, Status, Max };
    SteamBot::UI::Table<Columns> table;

    for (auto clientInfo: SteamBot::ClientInfo::getClients())
    {
        decltype(table)::Line line;
        line[Columns::Account] << clientInfo->accountName;

        auto& status=line[Columns::Status];
        if (auto client_=clientInfo->getClient())
        {
            SteamBot::Modules::Executor::execute(std::move(client_), [&status](SteamBot::Client& client) mutable {
                typedef SteamBot::Modules::Login::Whiteboard::LoginStatus LoginStatus;
                typedef SteamBot::Modules::PlayGames::Whiteboard::PlayingGames PlayingGames;
                typedef SteamBot::Modules::OwnedGames::Whiteboard::OwnedGames OwnedGames;
                switch (client.whiteboard.get<LoginStatus>(LoginStatus::LoggedOut))
                {
                case LoginStatus::LoggedOut:
                    break;

                case LoginStatus::LoggingIn:
                    status << "logging in";
                    break;

                case LoginStatus::LoggedIn:
                    if (auto playing=client.whiteboard.has<PlayingGames>())
                    {
                        assert(!playing->empty());

                        auto ownedGames=client.whiteboard.has<OwnedGames::Ptr>();
                        const char* separator="playing ";
                        for (SteamBot::AppID appId : *playing)
                        {
                            status << separator << static_cast<std::underlying_type_t<decltype(appId)>>(appId);
                            if (ownedGames)
                            {
                                if (auto info=(*ownedGames)->getInfo(appId))
                                {
                                    status << " (" << info->name << ")";
                                }
                            }
                            separator=", ";
                        }
                    }
                    else
                    {
                        status << "logged in";
                    }
                    break;

                default:
                    assert(false);
                }
            });
        }

        table.add(line);
    }

    table.sort(Columns::Account);
    while (table.startLine())
    {
        std::cout << table.getContent(Columns::Account);
        if (table.hasContent(Columns::Status))
        {
            std::cout << table.getFiller(Columns::Account) << " -> " << table.getContent(Columns::Status);
        }
        std::cout << '\n';
    }
    std::cout << std::flush;
}
