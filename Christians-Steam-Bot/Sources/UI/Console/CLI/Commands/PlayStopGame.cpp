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
#include "Modules/PlayGames.hpp"

/************************************************************************/

typedef SteamBot::Modules::PlayGames::Messageboard::PlayGames PlayGames;

/************************************************************************/

namespace
{
    class GameCommand : public SteamBot::UI::CommandBase
    {
    private:
        const bool play;
        const std::string_view commandString;
        const std::string_view descriptionString;

    public:
        GameCommand(bool play_)
            : play(play_),
              commandString(play_ ? "play-game" : "stop-game"),
              descriptionString(play_ ? "play games" : "stop games")
        {
        }

        virtual ~GameCommand() =default;

    public:
        virtual bool global() const
        {
            return false;
        }

        virtual const boost::program_options::positional_options_description* positionals() const override
        {
            static auto const positional=[](){
                auto positional_=new boost::program_options::positional_options_description();
                positional_->add("appids", -1);
                return positional_;
            }();
            return positional;
        }

        virtual const std::string_view& command() const override
        {
            return commandString;
        }

        virtual const std::string_view& description() const override
        {
            return descriptionString;
        }

        virtual const boost::program_options::options_description* options() const override
        {
            static auto const options=[this](){
                auto options_=new boost::program_options::options_description();
                options_->add_options()
                    ("appids",
                     boost::program_options::value<std::vector<SteamBot::AppID>>()->value_name("app-id")->multitoken()->required(),
                     (play ? "appids to play" : "appids to stop"))
                    ;
                return options_;
            }();
            return options;
        }

    public:
        class Execute : public ExecuteBase
        {
        private:
            bool play;
            std::vector<SteamBot::AppID> appIds;

        public:
            Execute(SteamBot::UI::CLI& cli_, bool play_)
                : ExecuteBase(cli_), play(play_)
            {
            }

            virtual ~Execute() =default;

        public:
            virtual bool init(const boost::program_options::variables_map& options) override
            {
                assert(options.count("appids"));
                appIds=options["appids"].as<std::vector<SteamBot::AppID>>();
                return true;
            }

            virtual void execute(SteamBot::ClientInfo* clientInfo) const override
            {
                if (auto client=clientInfo->getClient())
                {
                    bool success=SteamBot::Modules::Executor::execute(client, [this](SteamBot::Client&) mutable {
                        for (auto appId : appIds)
                        {
                            PlayGames::play(appId, play);
                        }
                    });

                    if (success)
                    {
                        auto ownedGames=std::move(CLI::Helpers::GameInfo{client->getClientInfo()}.ownedGames);
                        std::cout << (play ? "started" : "stopped") << " game(s):";
                        for (auto appId : appIds)
                        {
                            std::cout << " " << toInteger(appId);
                            if (auto info=ownedGames->getInfo(appId))
                            {
                                std::cout << " (" << info->name << ")";
                            }
                        }
                        std::cout << " on account " << client->getClientInfo().accountName << std::endl;
                    }
                }
            }
        };

        virtual std::shared_ptr<ExecuteBase> makeExecute(SteamBot::UI::CLI& cli) const override
        {
            return std::make_shared<Execute>(cli, play);
        }
    };
}

/************************************************************************/

namespace
{
    class PlayGameCommand : public GameCommand
    {
    public:
        PlayGameCommand() : GameCommand(true) { }
        virtual ~PlayGameCommand() =default;
    };

    class StopGameCommand : public GameCommand
    {
    public:
        StopGameCommand() : GameCommand(false) { }
        virtual ~StopGameCommand() =default;
    };

    GameCommand::Init<PlayGameCommand> initPlay;
    GameCommand::Init<StopGameCommand> initStop;
}
