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

#include "Client/Client.hpp"
#include "Modules/Executor.hpp"
#include "Cloud.hpp"
#include "Vector.hpp"
#include "Helpers/StringCompare.hpp"
#include "Helpers/NumberString.hpp"
#include "EnumString.hpp"

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
            static const std::string_view string("list-cloud");
            return string;
        }

        virtual const std::string_view& description() const override
        {
            static const std::string_view string("list steam-cloud usage");
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
                    ("size",
                     boost::program_options::bool_switch(),
                     "sort by total size")
                    ("count",
                     boost::program_options::bool_switch(),
                     "sort by file count")
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
            bool sortSize=false;
            bool sortCount=false;

        private:
            void filterApps(SteamBot::Cloud::Apps&) const;
            void sortApps(SteamBot::Cloud::Apps&) const;

        public:
            using ExecuteBase::ExecuteBase;

            virtual ~Execute() =default;

        public:
            virtual bool init(const boost::program_options::variables_map& options) override
            {
                sortSize=options["size"].as<bool>();
                sortCount=options["count"].as<bool>();
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

    ListCloudCommand::Init<ListCloudCommand> init;
}

/************************************************************************/

void ListCloudCommand::Execute::filterApps(SteamBot::Cloud::Apps& apps) const
{
    if (gamesRegex)
    {
        SteamBot::erase(apps.apps, [this](const SteamBot::Cloud::Apps::App& app){
            return !std::regex_search(app.name, *gamesRegex);
        });
    }
}

/************************************************************************/

void ListCloudCommand::Execute::sortApps(SteamBot::Cloud::Apps& apps) const
{
    typedef SteamBot::Cloud::Apps::App App;
    bool(*compare)(const App&, const App&)=nullptr;

    if (sortSize)
    {
        compare=[](const App& left, const App& right) {
            return left.totalSize<right.totalSize;
        };
    }
    else if (sortCount)
    {
        compare=[](const App& left, const App& right) {
            return left.totalCount<right.totalCount;
        };
    }
    else
    {
        compare=[](const App& left, const App& right) {
            return SteamBot::caseInsensitiveStringCompare_less(left.name, right.name);
        };
    }

    std::sort(apps.apps.begin(), apps.apps.end(), compare);
}

/************************************************************************/

static std::string printSize(uint64_t size)
{
    const char* unit=nullptr;

    if (size>=1024*1024)
    {
        size=(size*10)/(1024*1024)+5;
        unit="MiB";
    }
    else if (size>=1024)
    {
        size=(size*10)/(1024)+5;
        unit="KiB";
    }

    auto result=SteamBot::toString(size);

    if (unit!=nullptr)
    {
        // do the "floating point" correction
        assert(!result.empty());
        const char decimal=result.back();
        result.pop_back();
        if (decimal!='0')
        {
            result+='.';
            result+=decimal;
        }
    }
    else
    {
        unit="bytes";
    }

    result+=' ';
    result+=unit;

    return result;
}

/************************************************************************/

static void printApps(const SteamBot::Cloud::Apps& apps)
{
    uint32_t totalCount=0;
    uint64_t totalSize=0;

    for (const auto& app: apps.apps)
    {
        std::cout << SteamBot::toInteger(app.appId);
        if (!app.name.empty())
        {
            std::cout << " (" << app.name << ")";
        }
        std::cout << ": " << app.totalCount << " files with " << printSize(app.totalSize) << '\n';

        totalCount+=app.totalCount;
        totalSize+=app.totalSize;
    }

    std::cout << "listed " << apps.apps.size() << " games with " << totalCount << " files using " << printSize(totalSize) << '\n';
}

/************************************************************************/

void ListCloudCommand::Execute::execute(SteamBot::ClientInfo* clientInfo) const
{
    if (auto client=clientInfo->getClient())
    {
        SteamBot::Cloud::Apps apps;
        SteamBot::Modules::Executor::execute(client, [&apps](SteamBot::Client&) mutable {
            apps.load();
        });

        filterApps(apps);
        sortApps(apps);
        printApps(apps);
    }
}
