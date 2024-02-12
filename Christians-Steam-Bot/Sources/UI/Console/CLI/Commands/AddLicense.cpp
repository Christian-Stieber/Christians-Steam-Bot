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

#include "Modules/Executor.hpp"
#include "Modules/AddFreeLicense.hpp"

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
                positional->add("packageid", -1);
                return positional;
            }();
            return positional;
        }

        virtual const std::string_view& command() const override
        {
            static const std::string_view string("add-license");
            return string;
        }

        virtual const std::string_view& description() const override
        {
            static const std::string_view string("add (free) games to account");
            return string;
        }

        virtual const boost::program_options::options_description* options() const override
        {
            static auto const options=[this](){
                auto options=new boost::program_options::options_description();
                options->add_options()
                    ("packageid",
                     boost::program_options::value<std::vector<SteamBot::PackageID>>()->value_name("package-id")->multitoken()->required(),
                     "(free) package-ids to add")
                    ;
                return options;
            }();
            return options;
        }

    public:
        class Execute : public ExecuteBase
        {
        private:
            std::vector<SteamBot::PackageID> packageIds;

        public:
            using ExecuteBase::ExecuteBase;

            virtual ~Execute() =default;

        public:
            virtual bool init(const boost::program_options::variables_map& options) override
            {
                if (options.count("packageid"))
                {
                    packageIds=options["packageid"].as<std::vector<SteamBot::PackageID>>();
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
                        for (const auto packageId : packageIds)
                        {
                            boost::this_fiber::sleep_for(delay);
                            SteamBot::UI::OutputText() << "ClI: adding package " << toInteger(packageId);
                            SteamBot::Modules::AddFreeLicense::add(packageId);
                            delay=std::chrono::seconds(1);
                        }
                    });
                    if (success)
                    {
                        std::cout << "I've asked Steam to add";
                        for (const auto packageId : packageIds)
                        {
                            std::cout << " " << toInteger(packageId);
                        }
                        std::cout << " to your account" << std::endl;
                    }
                }
            }
        };

        virtual std::shared_ptr<ExecuteBase> makeExecute(SteamBot::UI::CLI& cli) const override
        {
            return std::make_shared<Execute>(cli);
        }
    };

    AddLicenseCommand::Init<AddLicenseCommand> init;
}
