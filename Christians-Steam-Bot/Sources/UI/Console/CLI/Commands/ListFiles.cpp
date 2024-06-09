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

#include "Client/Client.hpp"
#include "Modules/Executor.hpp"
#include "Modules/OwnedGames.hpp"
#include "Helpers/Time.hpp"
#include "Helpers/NumberString.hpp"
#include "Cloud.hpp"

/************************************************************************/

namespace
{
    class ListCloudCommand : public SteamBot::UI::CommandBase
    {
    public:
        virtual bool global() const
        {
            return false;
        }

        virtual const std::string_view& command() const override
        {
            static const std::string_view string("list-files");
            return string;
        }

        virtual const std::string_view& description() const override
        {
            static const std::string_view string("list steam-cloud files for a game");
            return string;
        }

        virtual const boost::program_options::positional_options_description* positionals() const override
        {
            static auto const positional=[](){
                auto positional_=new boost::program_options::positional_options_description();
                positional_->add("app", -1);
                return positional_;
            }();
            return positional;
        }

        virtual const boost::program_options::options_description* options() const override
        {
            static auto const options=[](){
                auto options_=new boost::program_options::options_description();
                options_->add_options()
                    ("app",
                     boost::program_options::value<uint32_t>()->value_name("appId"),
                     "game to list")
                    ;
                return options_;
            }();
            return options;
        }

    public:
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
                if (options.count("app"))
                {
                    try
                    {
                        appId=static_cast<SteamBot::AppID>(boost::numeric_cast<std::underlying_type_t<SteamBot::AppID>>(options["app"].as<uint32_t>()));
                        return true;
                    }
                    catch(const boost::numeric::bad_numeric_cast&)
                    {
                    }
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

    ListCloudCommand::Init<ListCloudCommand> init;
}

/************************************************************************/

static size_t printFiles(const SteamBot::Cloud::Files& files)
{
    size_t totalSize=0;

    enum class Columns : unsigned int { Name, Size, Timestamp, Platforms, Max };
    SteamBot::UI::Table<Columns> table;

    for (const auto& file: files.files)
    {
        totalSize+=file.fileSize;

        decltype(table)::Line line;
        line[Columns::Name] << file.fileName;
        line[Columns::Size] << SteamBot::printSize(file.fileSize);
        line[Columns::Timestamp] << SteamBot::Time::toString(file.timestamp);
        {
            const char* separator="";
            for (const std::string& platform: file.platforms)
            {
                line[Columns::Platforms] << separator << platform;
                separator=", ";
            }
        }
        table.add(line);
    }

    while (table.startLine())
    {
        std::cout << "   \"" << table.getContent(Columns::Name) << "\""
                  << table.getFiller(Columns::Name) << " | " << table.getContent(Columns::Size)
                  << table.getFiller(Columns::Size) << " | " << table.getContent(Columns::Timestamp)
                  << table.getFiller(Columns::Timestamp) << " |";
        if (table.hasContent(Columns::Platforms))
        {
            std::cout << " " << table.getContent(Columns::Platforms);
        }
        std::cout << '\n';
    }

    return totalSize;
}

/************************************************************************/

void ListCloudCommand::Execute::execute(SteamBot::ClientInfo* clientInfo) const
{
    if (auto client=clientInfo->getClient())
    {
        SteamBot::Cloud::Files files;

        {
            std::ostringstream header;
            SteamBot::Modules::Executor::execute(client, [this, &files, &header](SteamBot::Client&) mutable {
                files.load(appId);
                header << appId;
            });

            std::cout << header.view();
        }

        std::sort(files.files.begin(), files.files.end(), [](const SteamBot::Cloud::Files::File& left, const SteamBot::Cloud::Files::File& right) {
            return left.timestamp<right.timestamp;
        });

        if (files.files.empty())
        {
            std::cout << " has no files\n";
        }
        else
        {
            std::cout << " has " << files.files.size() << " files:\n";
            size_t totalSize=printFiles(files);
            std::cout << "listed " << files.files.size() << " files with a total size of " << SteamBot::printSize(totalSize) << "\n";
        }
    }
}
