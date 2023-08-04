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

#include "Client/Module.hpp"
#include "Modules/PersonaState.hpp"
#include "Modules/CardFarmer.hpp"

#include "Settings.hpp"

#include "Modules/AutoAccept.hpp"
#include "Modules/AutoLoadTradeoffers.hpp"

/************************************************************************/

static const char autoAcceptBotGiftsSetting[]="auto-accept-bot-gifts";

/************************************************************************/

static void settings()
{
    // ToDo: we need "enums" for settings...
    SteamBot::ClientSettings::get().use(autoAcceptBotGiftsSetting, SteamBot::ClientSettings::Type::Bool);
}

/************************************************************************/

#include "Client/Sleep.hpp"

namespace
{
    class MyModule : public SteamBot::Client::Module
    {
    private:
        SteamBot::Messageboard::WaiterType<SteamBot::ClientSettings::Changed> settingsChanged;

    public:
        MyModule() =default;
        virtual ~MyModule() =default;

        virtual void init(SteamBot::Client& client) override
        {
            settingsChanged=client.messageboard.createWaiter<SteamBot::ClientSettings::Changed>(*waiter);
        }

        void applySettings()
        {
            const auto& settings=SteamBot::ClientSettings::get();

            if (settings.getBool(autoAcceptBotGiftsSetting).value_or(false))
            {
                SteamBot::AutoAccept::enableBots(SteamBot::AutoAccept::Items::Gifts);
            }
            else
            {
                SteamBot::AutoAccept::enableBots(SteamBot::AutoAccept::Items::None);
            }
        }

        void handle(std::shared_ptr<const SteamBot::ClientSettings::Changed> message)
        {
            SteamBot::UI::OutputText() << "changed setting: " << message->name;
            applySettings();
        }

        virtual void run(SteamBot::Client&) override
        {
            applySettings();
            while (true)
            {
                waiter->wait();
                settingsChanged->handle(this);
            }
        }
    };

    MyModule::Init<MyModule> init;
}

/************************************************************************/

std::unique_ptr<SteamBot::UI::Base> SteamBot::UI::create()
{
#if 0
    useStatusCommand();
    useSelectCommand();
    useCreateCommand();
    useQuitCommand();
#endif

#if 0
    SteamBot::UI::CLI::useCommonCommands();
    SteamBot::UI::CLI::usePlayStopGameCommands();
    SteamBot::UI::CLI::useAddLicenseCommand();
    SteamBot::UI::CLI::useClearQueueCommand();
    SteamBot::UI::CLI::useSaleEventCommand();
    SteamBot::UI::CLI::useListInventoryCommand();
    SteamBot::UI::CLI::useSendInventoryCommand();
    SteamBot::UI::CLI::useDeclineTradeCommand();
    SteamBot::UI::CLI::useCancelTradeCommand();
    SteamBot::UI::CLI::useListTradeOffersCommand();
    SteamBot::UI::CLI::useCreateAddRemoveGroupCommands();
    SteamBot::UI::CLI::useListGroupsCommand();

    SteamBot::UI::CLI::useSettingsCommand();
#endif

    return createConsole();
}

/************************************************************************/

void application()
{
    SteamBot::Modules::PersonaState::use();
    // SteamBot::Modules::CardFarmer::use();

    settings();

    SteamBot::UI::Thread::outputText("Welcome to Christian's work-in-progress SteamBot");
    SteamBot::UI::Thread::outputText("Note: use the TAB or RETURN key to enter command mode");

    SteamBot::UI::Thread::wait();
}
