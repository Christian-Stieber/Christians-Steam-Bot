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
#include "UI/Table.hpp"

#include "Settings.hpp"
#include "Modules/Executor.hpp"

/************************************************************************/

namespace
{
    class SettingsCommand : public SteamBot::UI::CommandBase
    {
    public:
        virtual bool global() const
        {
            return false;
        }

        virtual const std::string_view& command() const override
        {
            static const std::string_view string("set");
            return string;
        }

        virtual const std::string_view& description() const override
        {
            static const std::string_view string("list or change settings");
            return string;
        }

        virtual const boost::program_options::positional_options_description* positionals() const override
        {
            static auto const positional=[](){
                auto positional_=new boost::program_options::positional_options_description();
                positional_->add("name", 1);
                positional_->add("value", 2);
                return positional_;
            }();
            return positional;
        }

        virtual const boost::program_options::options_description* options() const override
        {
            static auto const options=[](){
                auto options_=new boost::program_options::options_description();
                options_->add_options()
                    ("name",
                     boost::program_options::value<std::string>(),
                     "setting name")
                    ("value",
                     boost::program_options::value<std::string>(),
                     "setting value")
                    ;
                return options_;
            }();
            return options;
        }

    public:
        class Execute : public ExecuteBase
        {
        private:
            std::string name;
            std::string value;

        public:
            using ExecuteBase::ExecuteBase;

            virtual ~Execute() =default;

        public:
            virtual bool init(const boost::program_options::variables_map& options) override
            {
                const bool hasName=options.count("name");
                const bool hasValue=options.count("value");
                if (hasName==hasValue)
                {
                    if (hasName)
                    {
                        name=options["name"].as<std::string>();
                        value=options["value"].as<std::string>();
                    }
                    return true;
                }
                return false;
            }

            virtual void execute(SteamBot::ClientInfo* clientInfo) const override;
        };

        virtual std::shared_ptr<ExecuteBase> makeExecute(SteamBot::UI::CLI& cli) const override
        {
            return std::make_shared<Execute>(cli);
        }
    };

    SettingsCommand::Init<SettingsCommand> init;
}

/************************************************************************/

void SettingsCommand::Execute::execute(SteamBot::ClientInfo* clientInfo) const
{
    if (auto client=clientInfo->getClient())
    {
        if (name.empty() && value.empty())
        {
            std::map<std::string_view, std::string> items;
            SteamBot::Modules::Executor::execute(std::move(client), [&items](SteamBot::Client&) mutable {
                items=SteamBot::Settings::getValues();
            });

            enum class Columns : unsigned int { Name, Value, Max };
            SteamBot::UI::Table<Columns> table;

            for (const auto& item: items)
            {
                decltype(table)::Line line;
                line[Columns::Name] << item.first;
                if (!item.second.empty())
                {
                    line[Columns::Value] << item.second;
                }
                table.add(line);
            }

            table.sort(Columns::Name);

            while (table.startLine())
            {
                std::cout << table.getContent(Columns::Name);
                if (table.hasContent(Columns::Value))
                {
                    std::cout << table.getFiller(Columns::Name) << " -> " << table.getContent(Columns::Value);
                }
                std::cout << '\n';
            }
            std::cout << std::flush;
        }
        else
        {
            bool success=false;

            SteamBot::Modules::Executor::execute(std::move(client), [&success, this](SteamBot::Client&) {
                success=SteamBot::Settings::changeValue(name, value);
            });

            if (success)
            {
                std::cout << "changed setting" << std::endl;
            }
            else
            {
                std::cout << "failed to change setting (bad name or invalid value)" << std::endl;
            }
        }
    }
}
