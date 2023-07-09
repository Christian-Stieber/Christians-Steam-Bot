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

#include "UI/UI.hpp"
#include "UI/CLI.hpp"

#include "Modules/PersonaState.hpp"
#include "Modules/CardFarmer.hpp"
#include "Modules/TradeOffers.hpp"

#include "Settings.hpp"

#include "Client/Module.hpp"

#include "Modules/ClientNotification.hpp"

/************************************************************************/

std::unique_ptr<SteamBot::UI::Base> SteamBot::UI::create()
{
    SteamBot::UI::CLI::useCommonCommands();
    SteamBot::UI::CLI::useListGamesCommand();
    SteamBot::UI::CLI::usePlayStopGameCommands();
    SteamBot::UI::CLI::useAddLicenseCommand();
    SteamBot::UI::CLI::useClearQueueCommand();
    SteamBot::UI::CLI::useSaleEventCommand();
    SteamBot::UI::CLI::useListInventoryCommand();
    SteamBot::UI::CLI::useSendInventoryCommand();

    SteamBot::UI::CLI::useSettingsCommand();

    SteamBot::ClientSettings::get().use("test-item", SteamBot::ClientSettings::Type::Bool);

    return createConsole();
}

/************************************************************************/

#if 0
#include "Modules/UnifiedMessageServer.hpp"
#include "Steam/ProtoBuf/steammessages_player.steamclient.hpp"

namespace
{
    class TestModule : public SteamBot::Client::Module
    {
    public:
        TestModule() =default;
        virtual ~TestModule() =default;

        virtual void run(SteamBot::Client& client) override
        {
            SteamBot::Modules::UnifiedMessageServer::registerNotification<Steam::CPlayerLastPlayedTimesNotificationMessageType>("PlayerClient.NotifyLastPlayedTimes#1");

            auto notification=client.messageboard.createWaiter<Steam::CPlayerLastPlayedTimesNotificationMessageType>(*waiter);
            while (true)
            {
                waiter->wait();
                if (auto message=notification->fetch())
                {
                    SteamBot::UI::OutputText output;
                    output << "received a PlayerClient.NotifyLastPlayedTimes#1 notification for";
                    const char* separator=" ";
                    for (size_t i=0; i<message->content.games_size(); i++)
                    {
                        const auto& game=message->content.games(i);
                        if (game.has_appid())
                        {
                            output << separator << game.appid();
                            separator=", ";
                        }
                    }
                }
            }
        }
    };

    TestModule::Init<TestModule> init;
}
#endif

/************************************************************************/

void application()
{
    SteamBot::Modules::PersonaState::use();
    // SteamBot::Modules::CardFarmer::use();

    SteamBot::Modules::TradeOffers::use();
    SteamBot::Modules::ClientNotification::use();

    SteamBot::UI::Thread::outputText("Welcome to Christian's work-in-progress SteamBot");
    SteamBot::UI::Thread::outputText("Note: use the TAB or RETURN key to enter command mode");

    SteamBot::UI::Thread::wait();
}
