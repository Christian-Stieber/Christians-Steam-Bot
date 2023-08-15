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
    for (auto clientInfo: SteamBot::ClientInfo::getClients())
    {
        std::ostringstream output;
        output << "   " << clientInfo->accountName;
        if (auto client=clientInfo->getClient())
        {
            SteamBot::Modules::Executor::execute(std::move(client), [&output](SteamBot::Client& client) mutable {
                typedef SteamBot::Modules::Login::Whiteboard::LoginStatus LoginStatus;
                typedef SteamBot::Modules::PlayGames::Whiteboard::PlayingGames PlayingGames;
                typedef SteamBot::Modules::OwnedGames::Whiteboard::OwnedGames OwnedGames;
                switch (client.whiteboard.get<LoginStatus>(LoginStatus::LoggedOut))
                {
                case LoginStatus::LoggedOut:
                    break;

                case LoginStatus::LoggingIn:
                    output << "; logging in";
                    break;

                case LoginStatus::LoggedIn:
                    if (auto playing=client.whiteboard.has<PlayingGames>())
                    {
                        assert(!playing->empty());

                        auto ownedGames=client.whiteboard.has<OwnedGames::Ptr>();
                        const char* separator="; playing ";
                        for (SteamBot::AppID appId : *playing)
                        {
                            output << separator << static_cast<std::underlying_type_t<decltype(appId)>>(appId);
                            if (ownedGames)
                            {
                                if (auto info=(*ownedGames)->getInfo(appId))
                                {
                                    output << " (" << info->name << ")";
                                }
                            }
                            separator=", ";
                        }
                    }
                    else
                    {
                        output << "; logged in";
                    }
                    break;
                }
            });
        }
        std::cout << output.view() << std::endl;
    }
}
