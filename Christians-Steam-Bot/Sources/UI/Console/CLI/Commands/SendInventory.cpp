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
#include "SendInventory.hpp"

/************************************************************************/

namespace
{
    class SendInventoryCommand : public SteamBot::UI::CommandBase
    {
    public:
        virtual bool global() const
        {
            return false;
        }

        virtual const std::string_view& command() const override
        {
            static const std::string_view string("send-inventory");
            return string;
        }

        virtual const std::string_view& description() const override
        {
            static const std::string_view string("send all tradable Steam items to another bot account");
            return string;
        }

        virtual const boost::program_options::positional_options_description* positionals() const override
        {
            static auto const positional=[](){
                auto positional_=new boost::program_options::positional_options_description();
                positional_->add("recipient", 1);
                return positional_;
            }();
            return positional;
        }

        virtual const boost::program_options::options_description* options() const override
        {
            static auto const options=[](){
                auto options_=new boost::program_options::options_description();
                options_->add_options()
                    ("recipient",
                     boost::program_options::value<SteamBot::OptionBotName>()->value_name("accountname")->required(),
                     "recipient account")
                    ;
                return options_;
            }();
            return options;
        }

    public:
        class Execute : public ExecuteBase
        {
        private:
            SteamBot::ClientInfo* clientInfo=nullptr;

        public:
            using ExecuteBase::ExecuteBase;

            virtual ~Execute() =default;

        public:
            virtual bool init(const boost::program_options::variables_map& options) override
            {
                assert(options.count("recipient"));
                clientInfo=options["recipient"].as<SteamBot::OptionBotName>().clientInfo;
                return true;
            }

            virtual void execute(SteamBot::ClientInfo* clientInfo_) const override
            {
                bool success=false;
                if (auto client=clientInfo_->getClient())
                {
                    SteamBot::Modules::Executor::execute(std::move(client), [this, &success](SteamBot::Client&) {
                        success=SteamBot::sendInventory(this->clientInfo);
                    });
                }
                if (!success)
                {
                    std::cout << "failed to send inventory" << std::endl;
                }
            }
        };

        virtual std::shared_ptr<ExecuteBase> makeExecute(SteamBot::UI::CLI& cli) const override
        {
            return std::make_shared<Execute>(cli);
        }
    };

    SendInventoryCommand::Init<SendInventoryCommand> init;
}
