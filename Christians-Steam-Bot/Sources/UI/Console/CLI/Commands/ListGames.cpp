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
#include "Steam/AppType.hpp"

#include "steamdatabase/protobufs/steam/enums_productinfo.pb.h"

#include <iomanip>

/************************************************************************/

typedef CLI::Helpers::OwnedGames OwnedGames;
typedef CLI::Helpers::Licenses Licenses;
typedef CLI::Helpers::LicenseInfo LicenseInfo;
typedef SteamBot::Modules::PackageData::PackageInfo PackageInfo;

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
                     boost::program_options::value<SteamBot::OptionRegexID>()->value_name("regex|appId"),
                     "games to list")
                    ("playtime",
                     boost::program_options::bool_switch(),
                     "sort by playtime")
                    ("last-played",
                     boost::program_options::bool_switch(),
                     "sort by last played")
                    ("no-dlc",
                     boost::program_options::bool_switch(),
                     "don't list DLCs")
                    ("adult",
                     boost::program_options::bool_switch(),
                     "only list adult games")
                    ("early-access",
                     boost::program_options::bool_switch(),
                     "only list early access games")
                    ("farmable",
                     boost::program_options::bool_switch(),
                     "only list games with remaining card drops")
                    ;
                return options_;
            }();
            return options;
        }

    public:
        class Execute : public ExecuteBase
        {
        private:
            std::optional<SteamBot::OptionRegexID> gamesRegex;
            bool adult=false;
            bool earlyAccess=false;
            bool farmable=false;
            bool sortPlaytime=false;
            bool sortLastPlayed=false;
            bool noDLC=false;

        public:
            using ExecuteBase::ExecuteBase;

            virtual ~Execute() =default;

        private:
            struct GameItem
            {
                SteamBot::AppID appId=SteamBot::AppID::None;

                std::string name;
                SteamBot::AppType appType=SteamBot::AppType::Unknown;
                std::optional<boost::json::object> adult;
                bool earlyAccess=false;

                std::shared_ptr<const OwnedGames::GameInfo> gameInfo;
            };

        private:
            bool printAdult(const GameItem&) const;
            std::vector<GameItem> createGameList(const CLI::Helpers::GameInfo&) const;
            void sortGameList(std::vector<GameItem>&) const;
            void outputGameList(SteamBot::ClientInfo&, const CLI::Helpers::GameInfo&) const;

        public:
            virtual bool init(const boost::program_options::variables_map& options) override
            {
                noDLC=options["no-dlc"].as<bool>();
                adult=options["adult"].as<bool>();
                earlyAccess=options["early-access"].as<bool>();
                farmable=options["farmable"].as<bool>();
                sortPlaytime=options["playtime"].as<bool>();
                sortLastPlayed=options["last-played"].as<bool>();
                if (options.count("games"))
                {
                    gamesRegex=options["games"].as<SteamBot::OptionRegexID>();
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

bool ListGamesCommand::Execute::printAdult(const  ListGamesCommand::Execute::GameItem& game) const
{
    bool first=true;
    if (game.adult)
    {
        static constexpr auto names=[]() {
            std::array<std::string_view,5> myNames;
            myNames.at(k_EContentDescriptor_NudityOrSexualContent-1)="nudity/sexual content";
            myNames.at(k_EContentDescriptor_FrequentViolenceOrGore-1)="frequent violence/gore";
            myNames.at(k_EContentDescriptor_AdultOnlySexualContent-1)="adult sexual content";
            myNames.at(k_EContentDescriptor_GratuitousSexualContent-1)="gratuitous sexual content";
            myNames.at(k_EContentDescriptor_AnyMatureContent-1)="any mature content";
            return myNames;
        }();

        for (const auto& descriptor: *game.adult)
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
    }
    return !first;
}

/************************************************************************/

struct Totals
{
    std::chrono::minutes playtime{0};
    unsigned int earlyAccess=0;
    unsigned int adult=0;
    unsigned int DLC=0;
    unsigned int nonGame=0;
};

/************************************************************************/

std::vector<ListGamesCommand::Execute::GameItem> ListGamesCommand::Execute::createGameList(const CLI::Helpers::GameInfo& gameInfo) const
{
    std::unordered_set<SteamBot::AppID> appIds;
    for (const auto &license : gameInfo.licenses->licenses)
    {
        auto package=SteamBot::Modules::PackageData::getPackageInfo(*(license.second));
        if (package!=nullptr)
        {
            appIds.insert(package->appIds.begin(), package->appIds.end());
        }
    }

    std::vector<GameItem> games;
    for (auto appId : appIds)
    {
        GameItem item;

        item.appId=appId;

        item.appType=SteamBot::AppInfo::getAppType(appId);
        if (item.appType==SteamBot::AppType::DLC)
        {
            continue;
        }

        item.earlyAccess=SteamBot::AppInfo::isEarlyAccess(appId);
        if (earlyAccess && !item.earlyAccess)
        {
            continue;
        }

        if (auto json=SteamBot::AppInfo::get(appId, "common", "content_descriptors"))
        {
            if (auto descriptors=json->if_object())
            {
                if (!descriptors->empty())
                {
                    item.adult.emplace(std::move(*descriptors));
                }
            }
        }
        if (adult && !item.adult)
        {
            continue;
        }

        if (farmable)
        {
            bool isFarmable=false;
            if (auto badgeData=gameInfo.badgeData.get())
            {
                auto iterator=badgeData->badges.find(appId);
                if (iterator!=badgeData->badges.end())
                {
                    if (iterator->second.cardsReceived<iterator->second.cardsEarned)
                    {
                        isFarmable=true;
                    }
                }
            }
            if (!isFarmable)
            {
                continue;
            }
        }

        if (auto json=SteamBot::AppInfo::get(appId, "common", "name"))
        {
            if (auto string=json->if_string())
            {
                item.name=std::move(*string);
            }
        }
        if (gamesRegex && ! gamesRegex->doesMatch(item.name, appId))
        {
            continue;
        }

        games.emplace_back(std::move(item));
    }

    return games;
}

/************************************************************************/

void ListGamesCommand::Execute::sortGameList(std::vector<ListGamesCommand::Execute::GameItem> &games) const
{
    std::sort(games.begin(), games.end(), [this](const GameItem& left, const GameItem& right) -> bool {
        static const OwnedGames::GameInfo empty;
        const auto& l=left.gameInfo ? *(left.gameInfo) : empty;
        const auto& r=right.gameInfo ? *(right.gameInfo) : empty;

        if (sortPlaytime)
        {
            auto compare=(l.playtimeForever<=>r.playtimeForever);
            if (compare==std::strong_ordering::less) return true;
            if (compare==std::strong_ordering::greater) return false;
        }
        else if (sortLastPlayed)
        {
            auto compare=(l.lastPlayed<=>r.lastPlayed);
            if (compare==std::strong_ordering::less) return true;
            if (compare==std::strong_ordering::greater) return false;
        }
        return SteamBot::caseInsensitiveStringCompare_less(left.name, right.name);
    });
}

/************************************************************************/

void ListGamesCommand::Execute::outputGameList(SteamBot::ClientInfo& clientInfo, const CLI::Helpers::GameInfo& gameInfo) const
{
    auto games=createGameList(gameInfo);

    sortGameList(games);

    Totals totals;

    for (const auto& game : games)
    {
        std::cout << std::setw(8) << static_cast<std::underlying_type_t<decltype(game.appId)>>(game.appId) << ": ";
        if (game.appType!=SteamBot::AppType::Game)
        {
            std::cout << "(" << SteamBot::enumToStringAlways(game.appType) << ") ";
            totals.nonGame++;
        }
        std::cout << game.name;

        if (game.adult || game.earlyAccess)
        {
            std::cout << " (";
            if (game.adult)
            {
                std::cout << "Adult";
                totals.adult++;
                if (game.earlyAccess)
                {
                    std::cout << ", ";
                }
            }
            if (game.earlyAccess)
            {
                std::cout << "Early Access";
                totals.earlyAccess++;
            }
            std::cout << ")";
        }

        if (game.gameInfo)
        {
            if (game.gameInfo->lastPlayed!=decltype(game.gameInfo->lastPlayed)())
            {
                std::cout << "; last played " << SteamBot::Time::toString(game.gameInfo->lastPlayed);
            }
            if (game.gameInfo->playtimeForever.count()!=0)
            {
                std::cout << "; playtime " << SteamBot::Time::toString(game.gameInfo->playtimeForever);
                totals.playtime+=game.gameInfo->playtimeForever;
            }
        }

        {
            auto DLCs=SteamBot::AppInfo::getDLCs(game.appId);
            for (auto appId: DLCs)
            {
                auto licenses=CLI::Helpers::getLicenseInfo(clientInfo, appId);
                if (!licenses.empty())
                {
                    if (!noDLC)
                    {
                        std::cout << "\n          (DLC) " << appId;
                    }
                    totals.DLC++;
                }
            }
        }

        printAdult(game);

        if (gameInfo.badgeData)
        {
            auto iterator=gameInfo.badgeData->badges.find(game.appId);
            if (iterator!=gameInfo.badgeData->badges.end())
            {
                if (iterator->second.cardsReceived<iterator->second.cardsEarned)
                {
                    std::cout << "\n          ";
                    std::cout << iterator->second.cardsEarned << " cards earned, ";
                    std::cout << iterator->second.cardsReceived << " cards received";
                }
            }
        }

        std::cout << "\n";
    }

    std::cout << "listed " << games.size()-totals.nonGame << " games";
    if (totals.nonGame>0)
    {
        std::cout << " and " << totals.nonGame << " other";
    }
    std::cout << " ("
              << totals.adult << " adult, "
              << totals.earlyAccess << " early access) and "
              << totals.DLC << " DLCs, with a total playtime of "
              << SteamBot::Time::toString(totals.playtime) << "\n";
}

/************************************************************************/

void ListGamesCommand::Execute::execute(SteamBot::ClientInfo* clientInfo) const
{
    CLI::Helpers::GameInfo gameInfo(*clientInfo);
    if (gameInfo.ownedGames)
    {
        outputGameList(*clientInfo, gameInfo);
    }
    else
    {
        std::cout << "gamelist not available for \"" << clientInfo->accountName << "\"" << std::endl;
    }
}
