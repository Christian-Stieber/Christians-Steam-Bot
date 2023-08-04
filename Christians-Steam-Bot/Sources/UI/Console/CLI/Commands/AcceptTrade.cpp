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
#include "../Helpers.hpp"

#include "Modules/Executor.hpp"
#include "Helpers/ParseNumber.hpp"
#include "AcceptTrade.hpp"

/************************************************************************/

namespace
{
    class AcceptTradeCommand : public SteamBot::UI::CommandBase
    {
    public:
        virtual bool global() const
        {
            return false;
        }

        virtual const std::string_view& command() const override
        {
            static const std::string_view string("accept-trade");
            return string;
        }

        virtual const boost::program_options::positional_options_description* positionals() const override
        {
            static auto const positional=[](){
                auto positional=new boost::program_options::positional_options_description();
                positional->add("tradeoffer", -1);
                return positional;
            }();
            return positional;
        }

        virtual const boost::program_options::options_description* options() const override
        {
            static auto const options=[](){
                auto options=new boost::program_options::options_description();
                options->add_options()
                    ("tradeoffer",
                     boost::program_options::value<SteamBot::TradeOfferID>()->value_name("tradeoffer-id")->required(),
                     "tradeoffer to accept")
                    ;
                return options;
            }();
            return options;
        }

    public:
        class Execute : public ExecuteBase
        {
        private:
            SteamBot::TradeOfferID tradeofferId=SteamBot::TradeOfferID::None;

        public:
            using ExecuteBase::ExecuteBase;

            virtual ~Execute() =default;

        public:
            virtual bool init(const boost::program_options::variables_map& options) override
            {
                tradeofferId=options["tradeoffer"].as<SteamBot::TradeOfferID>();
                return true;
            }

            virtual void execute(SteamBot::ClientInfo* clientInfo) const override
            {
                bool success=false;
                if (auto client=clientInfo->getClient())
                {
                    SteamBot::Modules::Executor::execute(client, [this, &success](SteamBot::Client&) {
                        success=SteamBot::acceptTrade(tradeofferId);
                    });
                }
                if (success)
                {
                    std::cout << "accepted trade " << toInteger(tradeofferId) << std::endl;
                }
                else
                {
                    std::cout << "failed to accept trade " << toInteger(tradeofferId) << std::endl;
                }
            }
        };

        virtual std::unique_ptr<ExecuteBase> makeExecute(SteamBot::UI::CLI& cli) const override
        {
            return std::make_unique<Execute>(cli);
        }
    };

    AcceptTradeCommand::Init<AcceptTradeCommand> init;
}
