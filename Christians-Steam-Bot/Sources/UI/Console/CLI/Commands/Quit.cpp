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

#include "Modules/Executor.hpp"

/************************************************************************/

namespace
{
    class QuitCommand : public SteamBot::UI::CommandBase
    {
    public:
        virtual bool global() const
        {
            return false;
        }

        virtual const std::string_view& command() const override
        {
            static const std::string_view string("quit");
            return string;
        }

        virtual const std::string_view& description() const override
        {
            static const std::string_view string("quit the client");
            return string;
        }

    public:
        class Execute : public ExecuteBase
        {
        public:
            using ExecuteBase::ExecuteBase;

            virtual ~Execute() =default;

        public:
            virtual void execute(SteamBot::ClientInfo* clientInfo) const
            {
                if (auto client=clientInfo->getClient())
                {
                    bool success=SteamBot::Modules::Executor::execute(client, [](SteamBot::Client& client_) {
                        client_.quit(false);
                    });
                    if (success)
                    {
                        std::cout << "requested client \"" << client->getClientInfo().accountName << "\" to terminate" << std::endl;
                    }
                }
                if (cli.currentAccount==clientInfo)
                {
                    cli.currentAccount=nullptr;
                }
            }
        };

        virtual std::shared_ptr<ExecuteBase> makeExecute(SteamBot::UI::CLI& cli) const override
        {
            return std::make_shared<Execute>(cli);
        }
    };

    QuitCommand::Init<QuitCommand> init;
}
