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
#include "Modules/PackageData.hpp"

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
                auto positional=new boost::program_options::positional_options_description();
                positional->add("games", -1);
                return positional;
            }();
            return positional;
        }

        virtual const boost::program_options::options_description* options() const override
        {
            static auto const options=[](){
                auto options=new boost::program_options::options_description();
                options->add_options()
                    ("games",
                     boost::program_options::value<SteamBot::OptionRegex>()->value_name("regex"),
                     "games to list")
                    ;
                return options;
            }();
            return options;
        }

    public:
        class Execute : public ExecuteBase
        {
        private:
            std::optional<SteamBot::OptionRegex> gamesRegex;

        public:
            using ExecuteBase::ExecuteBase;

            virtual ~Execute() =default;

        private:
            void outputGameList(SteamBot::ClientInfo&, const OwnedGames::Ptr::element_type&) const;

        public:
            virtual bool init(const boost::program_options::variables_map& options) override
            {
                if (options.count("games"))
                {
                    gamesRegex=options["games"].as<SteamBot::OptionRegex>();
                }
                return true;
            }

            virtual void execute(SteamBot::ClientInfo* clientInfo) const override;
        };

        virtual std::unique_ptr<ExecuteBase> makeExecute(SteamBot::UI::CLI& cli) const override
        {
            return std::make_unique<Execute>(cli);
        }
    };

    ListGamesCommand::Init<ListGamesCommand> init;
}

/************************************************************************/

static void print(const SteamBot::Modules::LicenseList::Whiteboard::Licenses::LicenseInfo& license)
{
    auto packageIdValue=static_cast<std::underlying_type_t<decltype(license.packageId)>>(license.packageId);
    std::cout << "pkg " << packageIdValue
              << " purchased " << SteamBot::Time::toString(license.timeCreated, false);
}

/************************************************************************/

void ListGamesCommand::Execute::outputGameList(SteamBot::ClientInfo& clientInfo, const OwnedGames::Ptr::element_type& ownedGames) const
{
    typedef std::shared_ptr<const OwnedGames::GameInfo> ItemType;

    std::vector<ItemType> games;
    {
        for (const auto& item : ownedGames.games)
        {
            if (!gamesRegex || std::regex_search(item.second->name, *gamesRegex))
            {
                games.emplace_back(item.second);
            }
        }
    }

    std::sort(games.begin(), games.end(), [](const ItemType& left, const ItemType& right) -> bool {
        return SteamBot::caseInsensitiveStringCompare_less(left->name, right->name);
    });

    for (const auto& game : games)
    {
        std::cout << std::setw(8) << static_cast<std::underlying_type_t<decltype(game->appId)>>(game->appId) << ": " << game->name;
        if (game->lastPlayed!=decltype(game->lastPlayed)())
        {
            std::cout << "; last played " << SteamBot::Time::toString(game->lastPlayed);
        }
        if (game->playtimeForever.count()!=0)
        {
            std::cout << "; playtime " << SteamBot::Time::toString(game->playtimeForever);
        }

        auto licenses=CLI::Helpers::getLicenseInfo(clientInfo, game->appId);
        if (licenses.size()==1)
        {
            std::cout << "; ";
            ::print(*(licenses.front()));
        }
        else
        {
            for (auto& license : licenses)
            {
                std::cout << "\n          ";
                ::print(*license);
            }
        }
        std::cout << "\n";
    }
    std::cout << std::flush;
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
