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
#include "Modules/AddFreeLicense.hpp"

/************************************************************************/

typedef SteamBot::Modules::AddFreeLicense::Messageboard::AddLicense AddLicense;

/************************************************************************/

namespace
{
    class AddLicenseCommand : public SteamBot::UI::CommandBase
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
                positional->add("appid", -1);
                return positional;
            }();
            return positional;
        }

        virtual const std::string_view& command() const override
        {
            static const std::string_view string("add-license");
            return string;
        }

        virtual const boost::program_options::options_description* options() const override
        {
            static auto const options=[this](){
                auto options=new boost::program_options::options_description();
                options->add_options()
                    ("appid",
                     boost::program_options::value<std::vector<SteamBot::AppID>>()->value_name("app-id")->multitoken()->required(),
                     "(free) appids to add")
                    ;
                return options;
            }();
            return options;
        }

    public:
        class Execute : public ExecuteBase
        {
        private:
            std::vector<SteamBot::AppID> appIds;

        public:
            using ExecuteBase::ExecuteBase;

            virtual ~Execute() =default;

        public:
            virtual bool init(const boost::program_options::variables_map& options) override
            {
                if (options.count("appid"))
                {
                    appIds=options["appid"].as<std::vector<SteamBot::AppID>>();
                    return true;
                }
                return false;
            }

            virtual void execute(SteamBot::ClientInfo* clientInfo) const override
            {
                if (auto client=clientInfo->getClient())
                {
                    bool success=SteamBot::Modules::Executor::execute(std::move(client), [this](SteamBot::Client&) {
                        std::chrono::seconds delay(0);
                        for (const auto appId : appIds)
                        {
                            boost::this_fiber::sleep_for(delay);
                            SteamBot::UI::OutputText() << "ClI: adding license " << toInteger(appId);
                            AddLicense::add(appId);
                            delay=std::chrono::seconds(1);
                        }
                    });
                    if (success)
                    {
                        std::cout << "I've asked Steam to add ";
                        for (const auto appId : appIds)
                        {
                            std::cout << " " << toInteger(appId);
                        }
                        std::cout << " to your account" << std::endl;
                    }
                }
            }
        };

        virtual std::unique_ptr<ExecuteBase> makeExecute(SteamBot::UI::CLI& cli) const override
        {
            return std::make_unique<Execute>(cli);
        }
    };

    AddLicenseCommand::Init<AddLicenseCommand> init;
}
