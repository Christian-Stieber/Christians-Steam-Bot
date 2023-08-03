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

#include <string_view>
#include <memory>
#include <vector>

/************************************************************************/

namespace SteamBot
{
    class ClientInfo;

    namespace UI
    {
        class ConsoleUI;
    }
}

/************************************************************************/

namespace SteamBot
{
    namespace UI
    {
        class CLI
        {
        public:
            class Helpers;

        private:
            ConsoleUI& ui;

        public:
            std::unique_ptr<Helpers> helpers;
            SteamBot::ClientInfo* currentAccount=nullptr;
            bool quit=false;

        public:
            CLI(ConsoleUI&);
            ~CLI();

        public:
            SteamBot::ClientInfo* getAccount() const;
            SteamBot::ClientInfo* getAccount(std::string_view) const;
            void printHelp(const std::string*);
            void command(const std::string&);

            static std::vector<std::string> getWords(std::string_view);

        public:
            void run();

        public:
            static void useHelpCommand();
            static void useExitCommand();
            static void useStatusCommand();
            static void useLaunchCommand();
            static void useCreateCommand();
            static void useSelectCommand();
            static void useQuitCommand();

            static void useListGamesCommand();
            static void usePlayStopGameCommands();
            static void useAddLicenseCommand();
            static void useClearQueueCommand();
            static void useSettingsCommand();
            static void useListInventoryCommand();
            static void useSendInventoryCommand();
            static void useAcceptTradeCommand();
            static void useDeclineTradeCommand();
            static void useCancelTradeCommand();
            static void useListTradeOffersCommand();
            static void useCreateAddRemoveGroupCommands();
            static void useListGroupsCommand();

            static void useSaleEventCommand();
            static void useSaleStickerCommand();
            static void useSaleQueueCommand();

        };
    }
}
