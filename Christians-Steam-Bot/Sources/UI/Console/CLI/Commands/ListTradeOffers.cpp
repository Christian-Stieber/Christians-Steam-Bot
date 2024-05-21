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

#include "Modules/TradeOffers.hpp"
#include "Modules/Executor.hpp"
#include "AssetData.hpp"
#include "EnumString.hpp"

/************************************************************************/

namespace
{
    class ListTradeOffersCommand : public SteamBot::UI::CommandBase
    {
    public:
        virtual bool global() const
        {
            return false;
        }

        virtual const std::string_view& command() const override
        {
            static const std::string_view string("list-tradeoffers");
            return string;
        }

        virtual const std::string_view& description() const override
        {
            static const std::string_view string("list incoming and outgoing tradeoffers");
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

    ListTradeOffersCommand::Init<ListTradeOffersCommand> init;
}

/************************************************************************/

static void printOffers(const SteamBot::TradeOffers::TradeOffers& offers)
{
    struct PrintItems
    {
        static void print(SteamBot::UI::OutputText& output, const std::vector<std::shared_ptr<SteamBot::TradeOffers::TradeOffer::Item>>& items)
        {
            for (const auto& item : items)
            {
                output << "         ";
                if (item->amount>1)
                {
                    output << item->amount << "× ";
                }
                if (auto info=SteamBot::AssetData::query(item))
                {
                    output << "\"" << info->type << "\" / \"" << info->name << "\" (" << SteamBot::enumToString(info->itemType) << ")";
                }
                else
                {
                    output << "(unidentified item)";
                }
                output << "\n";
            }
        }
    };

    const char* direction=nullptr;
    const char* partnerLabel=nullptr;

    switch(offers.direction)
    {
    case SteamBot::TradeOffers::TradeOffers::Direction::Incoming:
        direction="incoming";
        partnerLabel="from";
        break;

    case SteamBot::TradeOffers::TradeOffers::Direction::Outgoing:
        direction="outgoing";
        partnerLabel="to";
        break;

    default:
        assert(false);
    }

    if (offers.offers.size()>0)
    {
        SteamBot::UI::OutputText output;
        output << offers.offers.size() << " " << direction << " trade offers:\n";
        for (const auto& offer : offers.offers)
        {
            output << "   id " << toInteger(offer.second->tradeOfferId);
            output << " " << partnerLabel << " " << SteamBot::ClientInfo::prettyName(offer.second->partner) << ":\n";
            output << "      my items:\n";
            PrintItems::print(output, offer.second->myItems);
            output << "      for their items:\n";
            PrintItems::print(output, offer.second->theirItems);
        }
    }
    else
    {
        SteamBot::UI::OutputText() << "no " << direction << " trade offers";
    }
}

/************************************************************************/

void ListTradeOffersCommand::Execute::execute(SteamBot::ClientInfo* clientInfo) const
{
    if (auto client=clientInfo->getClient())
    {
        bool success=SteamBot::Modules::Executor::executeWithFiber(client, [](SteamBot::Client&) {
            if (auto offers=SteamBot::TradeOffers::getIncoming())
            {
                printOffers(*offers);
            }
            else
            {
                SteamBot::UI::OutputText() << "no incoming trade offers";
            }
            if (auto offers=SteamBot::TradeOffers::getOutgoing())
            {
                printOffers(*offers);
            }
            else
            {
                SteamBot::UI::OutputText() << "no outgoing trade offers";
            }
        });
        if (success)
        {
            std::cout << "requested trade offers for account " << client->getClientInfo().accountName << std::endl;
        }
    }
}
