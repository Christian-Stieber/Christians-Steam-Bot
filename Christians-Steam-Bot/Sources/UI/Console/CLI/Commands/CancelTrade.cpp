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
#include "../Helpers.hpp"

#include "Modules/Executor.hpp"
#include "Helpers/ParseNumber.hpp"
#include "AcceptTrade.hpp"

/************************************************************************/

namespace
{
    class CancelTradeCommand : public CLI::CLICommandBase
    {
    public:
        CancelTradeCommand(CLI& cli_)
            : CLICommandBase(cli_, "cancel-trade", "<tradeofferid>", "cancel a trade", true)
        {
        }

        virtual ~CancelTradeCommand() =default;

    public:
        virtual bool execute(SteamBot::ClientInfo*, std::vector<std::string>&) override;
    };

    CancelTradeCommand::InitClass<CancelTradeCommand> init;
}

/************************************************************************/

bool CancelTradeCommand::execute(SteamBot::ClientInfo* clientInfo, std::vector<std::string>& words)
{
    if (words.size()!=2)
    {
        return false;
    }

    SteamBot::TradeOfferID tradeOfferId=SteamBot::TradeOfferID::None;
    if (SteamBot::parseNumber(words[1], tradeOfferId))
    {
        bool success=false;
        if (auto client=clientInfo->getClient())
        {
            SteamBot::Modules::Executor::execute(client, [tradeOfferId, &success](SteamBot::Client&) {
                success=SteamBot::cancelTrade(tradeOfferId);
            });
        }
        if (success)
        {
            std::cout << "canceled trade" << std::endl;
        }
        else
        {
            std::cout << "failed to cancel trade" << std::endl;
        }
    }
    else
    {
        std::cout << "invalid trade offer id \"" << words[1] << "\"" << std::endl;
    }

    return true;
}

/************************************************************************/

void SteamBot::UI::CLI::useCancelTradeCommand()
{
}
