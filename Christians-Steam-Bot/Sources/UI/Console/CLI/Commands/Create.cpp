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

/************************************************************************/

namespace
{
    class CreateCommand : public SteamBot::UI::CommandBase
    {
    public:
        virtual bool global() const
        {
            return true;
        }

        virtual const std::string_view& command() const override
        {
            static const std::string_view string("create");
            return string;
        }

        virtual const std::string_view& description() const override
        {
            static const std::string_view string("create a new bot");
            return string;
        }

        virtual const boost::program_options::positional_options_description* positionals() const override
        {
            static auto const positional=[](){
                auto positional=new boost::program_options::positional_options_description();
                positional->add("account", 1);
                return positional;
            }();
            return positional;
        }

        virtual const boost::program_options::options_description* options() const override
        {
            static auto const options=[](){
                auto options=new boost::program_options::options_description();
                options->add_options()
                    ("account",
                     boost::program_options::value<std::string>()->value_name("accountname")->required(),
                     "bot account")
                    ;
                return options;
            }();
            return options;
        }

    public:
        class Execute : public ExecuteBase
        {
        private:
            std::string account;

        public:
            using ExecuteBase::ExecuteBase;

            virtual ~Execute() =default;

        public:
            virtual bool init(const boost::program_options::variables_map& options) override
            {
                assert(options.count("account")==1);
                account=options["account"].as<std::string>();
                return true;
            }

            virtual void execute(SteamBot::ClientInfo*) const
            {
                const auto clientInfo=SteamBot::ClientInfo::create(account);
                if (clientInfo==nullptr)
                {
                    std::cout << "account \"" << account << "\" already exists" << std::endl;
                }
                else
                {
                    SteamBot::Client::launch(*clientInfo);
                    std::cout << "launched new client \"" << account << "\"" << std::endl;
                    std::cout << "NOTE: leave command mode to be able to see password/SteamGuard prompts!" << std::endl;

                    cli.currentAccount=clientInfo;
                    std::cout << "your current account is now \"" << cli.currentAccount->accountName << "\"" << std::endl;
                }
            }
        };

        virtual std::shared_ptr<ExecuteBase> makeExecute(SteamBot::UI::CLI& cli) const override
        {
            return std::make_shared<Execute>(cli);
        }
    };

    CreateCommand::Init<CreateCommand> init;
}
