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
#include "Modules/WebSession.hpp"

/************************************************************************/

namespace
{
    class LoadURLCommand : public SteamBot::UI::CommandBase
    {
    public:
        LoadURLCommand() =default;

    public:
        virtual bool global() const
        {
            return false;
        }

        virtual const std::string_view& command() const override
        {
            static const std::string_view string("load-url");
            return string;
        }

        virtual const std::string_view& description() const override
        {
            static const std::string_view string("load a Steam-webpage");
            return string;
        }

        virtual const boost::program_options::positional_options_description* positionals() const override
        {
            static auto const positional=[](){
                auto positional_=new boost::program_options::positional_options_description();
                positional_->add("url", -1);
                return positional_;
            }();
            return positional;
        }

        virtual const boost::program_options::options_description* options() const override
        {
            static auto const options=[](){
                auto options_=new boost::program_options::options_description();
                options_->add_options()
                    ("url",
                     boost::program_options::value<SteamBot::OptionURL>()->value_name("url"),
                     "page url")
                    ;
                return options_;
            }();
            return options;
        }

    public:
        class Execute : public ExecuteBase
        {
        private:
            SteamBot::OptionURL url;

        public:
            using ExecuteBase::ExecuteBase;

            virtual ~Execute() =default;

        public:
            virtual bool init(const boost::program_options::variables_map& options) override
            {
                if (options.count("url"))
                {
                    url=options["url"].as<SteamBot::OptionURL>();
                    return true;
                }
                return false;
            }

            bool loadURL() const
            {
                typedef SteamBot::Modules::WebSession::Messageboard::Request Request;

                auto request=std::make_shared<Request>();
                request->queryMaker=[this](){
                    return std::make_unique<SteamBot::HTTPClient::Query>(boost::beast::http::verb::get, url);
                };

                auto response=SteamBot::Modules::WebSession::makeQuery(std::move(request));
                if (response->query->response.result()!=boost::beast::http::status::ok)
                {
                    return false;
                }
                return true;
            }

            virtual void execute(SteamBot::ClientInfo* clientInfo) const override
            {
                if (auto client=clientInfo->getClient())
                {
                    bool success=false;
                    SteamBot::Modules::Executor::execute(client, [this, &success](SteamBot::Client&) mutable {
                        success=loadURL();
                    });
                    std::cout << "page load: " << (success ? "success" : "failure") << '\n';
                }
            }
        };

        virtual std::shared_ptr<ExecuteBase> makeExecute(SteamBot::UI::CLI& cli) const override
        {
            return std::make_shared<Execute>(cli);
        }
    };

    LoadURLCommand::Init<LoadURLCommand> init;
}
