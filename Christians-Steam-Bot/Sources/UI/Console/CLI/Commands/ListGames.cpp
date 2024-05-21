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

#include "Client/Client.hpp"
#include "Helpers/StringCompare.hpp"
#include "Helpers/Time.hpp"
#include "Helpers/JSON.hpp"
#include "Modules/PackageData.hpp"
#include "Modules/PackageInfo.hpp"
#include "EnumString.hpp"
#include "AppInfo.hpp"

#include "steamdatabase/protobufs/steam/enums_productinfo.pb.h"

#include <iomanip>

/************************************************************************/

typedef CLI::Helpers::OwnedGames OwnedGames;

/************************************************************************/

namespace
{
    class ListGamesCommand : public SteamBot::UI::CommandBase
    {
    public:
        ListGamesCommand()
        {
            SteamBot::Modules::OwnedGames::use();
        }

    public:
        virtual bool global() const
        {
            return false;
        }

        virtual const std::string_view& command() const override
        {
            static const std::string_view string("list-games");
            return string;
        }

        virtual const std::string_view& description() const override
        {
            static const std::string_view string("list games on account");
            return string;
        }

        virtual const boost::program_options::positional_options_description* positionals() const override
        {
            static auto const positional=[](){
                auto positional_=new boost::program_options::positional_options_description();
                positional_->add("games", -1);
                return positional_;
            }();
            return positional;
        }

        virtual const boost::program_options::options_description* options() const override
        {
            static auto const options=[](){
                auto options_=new boost::program_options::options_description();
                options_->add_options()
                    ("games",
                     boost::program_options::value<SteamBot::OptionRegex>()->value_name("regex"),
                     "games to list")
                    ("playtime",
                     boost::program_options::bool_switch(),
                     "sort by playtime")
                    ("adult",
                     boost::program_options::bool_switch(),
                     "only list adult games")
                    ("early-access",
                     boost::program_options::bool_switch(),
                     "only list early access games")
                    ;
                return options_;
            }();
            return options;
        }

    public:
        class Execute : public ExecuteBase
        {
        private:
            std::optional<SteamBot::OptionRegex> gamesRegex;
            bool adult=false;
            bool earlyAccess=false;
            bool sortPlaytime=false;

        public:
            using ExecuteBase::ExecuteBase;

            virtual ~Execute() =default;

        private:
            bool printAdult(const OwnedGames::GameInfo&) const;
            bool isEarlyAccess(const OwnedGames::GameInfo&) const;
            bool isAdult(const OwnedGames::GameInfo&) const;
            void outputGameList(SteamBot::ClientInfo&, const OwnedGames::Ptr::element_type&) const;

        public:
            virtual bool init(const boost::program_options::variables_map& options) override
            {
                adult=options["adult"].as<bool>();
                earlyAccess=options["early-access"].as<bool>();
                sortPlaytime=options["playtime"].as<bool>();
                if (options.count("games"))
                {
                    gamesRegex=options["games"].as<SteamBot::OptionRegex>();
                }
                return true;
            }

            virtual void execute(SteamBot::ClientInfo* clientInfo) const override;
        };

        virtual std::shared_ptr<ExecuteBase> makeExecute(SteamBot::UI::CLI& cli) const override
        {
            return std::make_shared<Execute>(cli);
        }
    };

    ListGamesCommand::Init<ListGamesCommand> init;
}

/************************************************************************/

static void print(const SteamBot::Modules::LicenseList::Whiteboard::Licenses::LicenseInfo& license)
{
    auto packageIdValue=static_cast<std::underlying_type_t<decltype(license.packageId)>>(license.packageId);
    std::cout << "pkg " << packageIdValue;
    if (auto info=SteamBot::Modules::PackageInfo::Info::get(license.packageId))
    {
        if (!info->packageName.empty())
        {
            std::cout << " (" << info->packageName << ")";
        }
    }
    std::cout << " purchased " << SteamBot::Time::toString(license.timeCreated, false);
    // std::cout << " (" << SteamBot::enumToStringAlways(license.licenseType) << ")";
    if (license.paymentMethod!=SteamBot::PaymentMethod::None)
    {
        std::cout << " (" << SteamBot::enumToStringAlways(license.paymentMethod) << ")";
    }
}

/************************************************************************/

bool ListGamesCommand::Execute::isEarlyAccess(const OwnedGames::GameInfo& info) const
{
    if (auto json=SteamBot::AppInfo::get(info.appId, "common", "genres"))
    {
        if (auto genres=json->if_object())
        {
            for (const auto& genre: *genres)
            {
                auto id=SteamBot::JSON::toNumber<int>(genre.value());
                if (id==70)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

/************************************************************************/

bool ListGamesCommand::Execute::printAdult(const OwnedGames::GameInfo& info) const
{
    if (auto json=SteamBot::AppInfo::get(info.appId, "common", "content_descriptors"))
    {
        if (auto descriptors=json->if_object())
        {
            static const auto& names=[]() -> const auto& {
                static std::array<std::string_view,5> myNames;
                myNames.at(k_EContentDescriptor_NudityOrSexualContent-1)="nudity/sexual content";
                myNames.at(k_EContentDescriptor_FrequentViolenceOrGore-1)="frequent violence/gore";
                myNames.at(k_EContentDescriptor_AdultOnlySexualContent-1)="adult sexual content";
                myNames.at(k_EContentDescriptor_GratuitousSexualContent-1)="gratuitous sexual content";
                myNames.at(k_EContentDescriptor_AnyMatureContent-1)="any mature content";
                return myNames;
            }();

            bool first=true;
            for (const auto& descriptor: *descriptors)
            {
                if (first)
                {
                    std::cout << "\n          adult: ";
                    first=false;
                }
                else
                {
                    std::cout << ", ";
                }
                auto id=SteamBot::JSON::toNumber<EContentDescriptorID>(descriptor.value());
                if (id>=1 && id<=static_cast<int>(names.size()))
                {
                    std::cout << names[static_cast<size_t>(id-1)];
                }
                else
                {
                    std::cout << id;
                }
            }
            return !first;
        }
    }
    return false;
}

/************************************************************************/

bool ListGamesCommand::Execute::isAdult(const OwnedGames::GameInfo& info) const
{
    if (auto json=SteamBot::AppInfo::get(info.appId, "common", "content_descriptors"))
    {
        if (auto descriptors=json->if_object())
        {
            return !descriptors->empty();
        }
    }
    return false;
}

/************************************************************************/

void ListGamesCommand::Execute::outputGameList(SteamBot::ClientInfo& clientInfo, const OwnedGames::Ptr::element_type& ownedGames) const
{
    typedef std::shared_ptr<const OwnedGames::GameInfo> ItemType;

    std::vector<ItemType> games;
    {
        for (const auto& item : ownedGames.games)
        {
            const auto& info=*(item.second);
            if ((!adult || isAdult(info)) &&
                (!earlyAccess || isEarlyAccess(info)) &&
                (!gamesRegex || std::regex_search(info.name, *gamesRegex)))
            {
                games.emplace_back(item.second);
            }
        }
    }

    std::sort(games.begin(), games.end(), [this](const ItemType& left, const ItemType& right) -> bool {
        if (sortPlaytime)
        {
            auto compare=(left->playtimeForever<=>right->playtimeForever);
            if (compare==std::strong_ordering::less) return true;
            if (compare==std::strong_ordering::greater) return false;
        }
        return SteamBot::caseInsensitiveStringCompare_less(left->name, right->name);
    });

    struct
    {
        std::chrono::minutes playtime{0};
        unsigned int earlyAccess=0;
        unsigned int adult=0;
    } totals;

    for (const auto& game : games)
    {
        std::cout << std::setw(8) << static_cast<std::underlying_type_t<decltype(game->appId)>>(game->appId) << ": " << game->name;

        {
            bool adult_=isAdult(*game);
            bool earlyAccess_=isEarlyAccess(*game);
            if (adult_ || earlyAccess_)
            {
                std::cout << " (";
                if (adult_)
                {
                    std::cout << "Adult";
                    totals.adult++;
                }
                if (adult_ && earlyAccess_) std::cout << ", ";
                if (earlyAccess_)
                {
                    std::cout << "Early Access";
                    totals.earlyAccess++;
                }
                std::cout << ")";
            }
        }

        if (game->lastPlayed!=decltype(game->lastPlayed)())
        {
            std::cout << "; last played " << SteamBot::Time::toString(game->lastPlayed);
        }
        if (game->playtimeForever.count()!=0)
        {
            std::cout << "; playtime " << SteamBot::Time::toString(game->playtimeForever);
            totals.playtime+=game->playtimeForever;
        }

        auto licenses=CLI::Helpers::getLicenseInfo(clientInfo, game->appId);
        for (auto& license : licenses)
        {
            std::cout << "\n          ";
            ::print(*license);
        }

        if (adult)
        {
            printAdult(*game);
        }

        std::cout << "\n";
    }

    std::cout << "listed " << games.size() << " games (" << totals.adult << " adult, "
              << totals.earlyAccess << " early access) with a total playtime of "
              << SteamBot::Time::toString(totals.playtime) << "\n";
}

/************************************************************************/

void ListGamesCommand::Execute::execute(SteamBot::ClientInfo* clientInfo) const
{
    if (auto ownedGames=CLI::Helpers::getOwnedGames(*clientInfo))
    {
        outputGameList(*clientInfo, *ownedGames);
    }
    else
    {
        std::cout << "gamelist not available for \"" << clientInfo->accountName << "\"" << std::endl;
    }
}
