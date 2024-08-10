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

#include "Modules/PackageData.hpp"
#include "Modules/Executor.hpp"
#include "Client/ClientInfo.hpp"
#include "Helpers/Time.hpp"
#include "AppInfo.hpp"
#include "EnumString.hpp"
#include "Steam/AppType.hpp"

/************************************************************************/

namespace
{
    class ShowLicenseCommand : public SteamBot::UI::CommandBase
    {
    public:
        virtual bool global() const
        {
            return false;
        }

        virtual const std::string_view& command() const override
        {
            static const std::string_view string("show-license");
            return string;
        }

        virtual const std::string_view& description() const override
        {
            static const std::string_view string("show licenses for a game");
            return string;
        }

        virtual const boost::program_options::positional_options_description* positionals() const override
        {
            static auto const positional=[](){
                auto positional_=new boost::program_options::positional_options_description();
                positional_->add("appId", -1);
                return positional_;
            }();
            return positional;
        }

        virtual const boost::program_options::options_description* options() const override
        {
            static auto const options=[](){
                auto options_=new boost::program_options::options_description();
                options_->add_options()
                    ("appId",
                     boost::program_options::value<uint32_t>()->value_name("appId"),
                     "game to list")
                    ;
                return options_;
            }();
            return options;
        }

        class Execute : public ExecuteBase
        {
        private:
            SteamBot::AppID appId;

        public:
            using ExecuteBase::ExecuteBase;

            virtual ~Execute() =default;

        public:
            virtual bool init(const boost::program_options::variables_map& options) override
            {
                if (options.count("appId"))
                {
                    try
                    {
                        appId=static_cast<SteamBot::AppID>(boost::numeric_cast<std::underlying_type_t<SteamBot::AppID>>(options["appId"].as<uint32_t>()));
                        return true;
                    }
                    catch(const boost::numeric::bad_numeric_cast&)
                    {
                    }
                }
                return false;
            }

            virtual void execute(SteamBot::ClientInfo*) const override;
        };

        virtual std::shared_ptr<ExecuteBase> makeExecute(SteamBot::UI::CLI& cli) const override
        {
            return std::make_shared<Execute>(cli);
        }
    };

    ShowLicenseCommand::Init<ShowLicenseCommand> init;
}

/************************************************************************/
/*
 * This is initialized on the client thread, but is meant to be output
 * on the UI thread.
 */

namespace
{
    class Info
    {
    private:
        typedef SteamBot::Modules::LicenseList::Whiteboard::Licenses::LicenseInfo LicenseInfo;

    private:
        SteamBot::AppID game=SteamBot::AppID::None;

        std::unordered_map<SteamBot::PackageID, std::shared_ptr<const LicenseInfo>> licenses;

    private:
        void addLicenses(SteamBot::AppID);
        void printLicense(SteamBot::PackageID);

    public:
        Info(SteamBot::AppID game_)
            : game(game_)
        {
        }

        // runs on the client thread
        void init();

        // runs on the UI thread
        void print();
    };
}

/************************************************************************/
/*
 * This adds all licenses providing he appId into our table
 */

void Info::addLicenses(SteamBot::AppID appId)
{
    typedef SteamBot::Modules::PackageData::PackageInfo PackageInfo;

    std::vector<std::shared_ptr<const PackageInfo>> packages=SteamBot::Modules::PackageData::getPackageInfo(appId);
    for (const auto& package: packages)
    {
        const SteamBot::PackageID packageId=package->packageId;

        std::shared_ptr<const LicenseInfo> license=SteamBot::Modules::LicenseList::getLicenseInfo(packageId);
        if (license)
        {
            auto& entry=licenses[packageId];
            if (entry)
            {
                assert(entry==license);
            }
            else
            {
                entry=std::move(license);
            }
        }
    }
}

/************************************************************************/
/*
 * Output the information for the license
 *
 * This will also "clear" the entry in the license table, since we
 * want to list each license just once.
 */

void Info::printLicense(SteamBot::PackageID packageId)
{
    auto iterator=licenses.find(packageId);
    if (iterator!=licenses.end())
    {
        if (iterator->second)
        {
            const LicenseInfo& license=(*iterator->second);

            std::cout << "   package " << license.packageId
                      << " purchased " << SteamBot::Time::toString(license.timeCreated, false);
            if (license.paymentMethod!=SteamBot::PaymentMethod::None)
            {
                std::cout << " (" << SteamBot::enumToStringAlways(license.paymentMethod) << ')';
            }
            std::cout << '\n';

            if (auto package=SteamBot::Modules::PackageData::getPackageInfo(license))
            {
                for (const SteamBot::AppID appId: package->appIds)
                {
                    std::cout << "      "
                              << SteamBot::enumToStringAlways(SteamBot::AppInfo::getAppType(appId))
                              << " " << appId << '\n';
                }
            }

            iterator->second=nullptr;
        }
    }
}

/************************************************************************/

void Info::print()
{
    std::cout << "You have " << licenses.size() << " licenses related to " << game << ": \n";

    // lets first print the licenses providing the game itself
    {
        auto packages=SteamBot::Modules::PackageData::getPackageInfo(game);
        for (const auto& package: packages)
        {
            printLicense(package->packageId);
        }
    }

    // followed by the remaining licenses in no particular order
    for (const auto& entry: licenses)
    {
        if (entry.second)
        {
            printLicense(entry.second->packageId);
        }
    }
}

/************************************************************************/

void Info::init()
{
    addLicenses(game);
    for (auto appId: SteamBot::AppInfo::getDLCs(game))
    {
        addLicenses(appId);
    }
}

/************************************************************************/

void ShowLicenseCommand::Execute::execute(SteamBot::ClientInfo* clientInfo) const
{
    if (auto client=clientInfo->getClient())
    {
        Info info(appId);
        SteamBot::Modules::Executor::execute(std::move(client), [&info](SteamBot::Client&) {
            info.init();
        });

        info.print();
    }
}
