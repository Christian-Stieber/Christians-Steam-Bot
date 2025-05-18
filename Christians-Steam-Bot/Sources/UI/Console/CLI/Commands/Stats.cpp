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

#include "EnumString.hpp"
#include "Modules/LicenseList.hpp"
#include "Modules/Executor.hpp"

/************************************************************************/

typedef SteamBot::Modules::LicenseList::Whiteboard::Licenses Licenses;

/************************************************************************/

namespace
{
    class StatsCommand : public SteamBot::UI::CommandBase
    {
    public:
        virtual bool global() const
        {
            return false;
        }

        virtual const std::string_view& command() const override
        {
            static const std::string_view string("stats");
            return string;
        }

        virtual const std::string_view& description() const override
        {
            static const std::string_view string("show some counts about your library");
            return string;
        }

    public:
        class Execute : public ExecuteBase
        {
        public:
            using ExecuteBase::ExecuteBase;

            virtual ~Execute() =default;

        public:
            virtual void execute(SteamBot::ClientInfo*) const;
        };

        virtual std::shared_ptr<ExecuteBase> makeExecute(SteamBot::UI::CLI& cli) const override
        {
            return std::make_shared<Execute>(cli);
        }
    };

    StatsCommand::Init<StatsCommand> init;
}

/************************************************************************/

class Processor
{
private:
    struct
    {
        uint32_t game=0;
        uint32_t DLC=0;
        uint32_t demo=0;
        uint32_t other=0;
    } app;

    struct
    {
        uint32_t adult=0;
        uint32_t earlyAccess=0;
    } category;

    struct
    {
        uint32_t free=0;
        uint32_t store=0;
        uint32_t key=0;
    } payment;

    struct
    {
        uint32_t f2p=0;
        uint32_t promotional=0;
        uint32_t regular=0;
        uint32_t other=0;
    } license;

    Licenses::Ptr licenses;

    void getWhiteboardData(std::shared_ptr<SteamBot::Client>);
    void printWeirdType() const;

public:
    Processor(std::shared_ptr<SteamBot::Client>);
};

/************************************************************************/

void Processor::getWhiteboardData(std::shared_ptr<SteamBot::Client> client)
{
    typedef SteamBot::Modules::Executor::GetWhiteboard<Licenses::Ptr> GetWhiteboard;
    GetWhiteboard::ResultType result;
    GetWhiteboard::perform(std::move(client), result);
    licenses=std::move(std::get<Licenses::Ptr>(result));

    if (!licenses)
    {
        std::cout << "no license data available; try again later\n";
    }
}

/************************************************************************/

void Processor::printWeirdType() const
{
    std::unordered_map<SteamBot::LicenseType, uint32_t> weird;
    for (const auto& pair: licenses->licenses)
    {
        auto type=pair.second->licenseType;
        if (type!=SteamBot::LicenseType::SinglePurchase)
        {
            weird[type]++;
        }
    }
    if (!weird.empty())
    {
        std::cout << "You have licenses that are not \"" << SteamBot::enumToString(SteamBot::LicenseType::SinglePurchase) << "\": \n";
        for (const auto& pair: weird)
        {
            std::cout << "   " << pair.second << " \xC3\x97 " << SteamBot::enumToString(pair.first) << "\n";
        }
    }
}

/************************************************************************/

Processor::Processor(std::shared_ptr<SteamBot::Client> client)
{
    getWhiteboardData(std::move(client));
    if (licenses)
    {
        std::cout << licenses->licenses.size() << " licenses\n";
        printWeirdType();
    }
}

/************************************************************************/

void StatsCommand::Execute::execute(SteamBot::ClientInfo* clientInfo) const
{
    if (auto client=clientInfo->getClient())
    {
        Processor processor(std::move(client));
    }
}
