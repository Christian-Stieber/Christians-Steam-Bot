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
#include "AcceptTrade.hpp"

/************************************************************************/

namespace
{
    typedef bool(*ActionFunction)(SteamBot::TradeOfferID);

    struct AcceptInfo
    {
        static constexpr std::string_view command{"accept-trade"};
        static constexpr std::string_view description{"accept an incoming trade"};
        static constexpr char offerDesc[]="tradeoffer to accept";

        static constexpr ActionFunction action=&SteamBot::acceptTrade;

        static constexpr char success[]="accepted";
        static constexpr char failure[]="accept";
    };

    struct DeclineInfo
    {
        static constexpr std::string_view command{"decline-trade"};
        static constexpr std::string_view description{"decline an incoming trade"};
        static constexpr char offerDesc[]="tradeoffer to decline";

        static constexpr ActionFunction action=&SteamBot::declineTrade;

        static constexpr char success[]="declined";
        static constexpr char failure[]="decline";
    };

    struct CancelInfo
    {
        static constexpr std::string_view command{"cancel-trade"};
        static constexpr std::string_view description{"cancel an outgoing trade"};
        static constexpr char offerDesc[]="tradeoffer to cancel";

        static constexpr ActionFunction action=&SteamBot::cancelTrade;

        static constexpr char success[]="cancelled";
        static constexpr char failure[]="cancele";
    };
}

/************************************************************************/

namespace
{
    template <typename INFO> class TradeCommand : public SteamBot::UI::CommandBase
    {
    public:
        virtual bool global() const
        {
            return false;
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

        virtual const std::string_view& command() const override
        {
            return INFO::command;
        }

        virtual const std::string_view& description() const override
        {
            return INFO::description;
        }

        virtual const boost::program_options::options_description* options() const override
        {
            static auto const options=[this](){
                auto options=new boost::program_options::options_description();
                options->add_options()
                    ("tradeoffer",
                     boost::program_options::value<SteamBot::TradeOfferID>()->value_name("tradeoffer-id")->required(),
                     INFO::offerDesc)
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
                        success=(*INFO::action)(tradeofferId);
                    });
                }
                if (success)
                {
                    std::cout << INFO::success << " trade " << toInteger(tradeofferId) << std::endl;
                }
                else
                {
                    std::cout << "failed to " << INFO::failure << " trade " << toInteger(tradeofferId) << std::endl;
                }
            }
        };

        virtual std::unique_ptr<ExecuteBase> makeExecute(SteamBot::UI::CLI& cli) const override
        {
            return std::make_unique<Execute>(cli);
        }
    };

    SteamBot::UI::CommandBase::Init<TradeCommand<AcceptInfo>> initAccept;
    SteamBot::UI::CommandBase::Init<TradeCommand<DeclineInfo>> initDecline;
    SteamBot::UI::CommandBase::Init<TradeCommand<CancelInfo>> initCancel;
}
