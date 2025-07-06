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
#include "Modules/PackageData.hpp"
#include "Modules/Executor.hpp"
#include "Steam/AppType.hpp"
#include "Steam/BillingType.hpp"
#include "AppInfo.hpp"

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
    Licenses::Ptr licenses;

    void getWhiteboardData(std::shared_ptr<SteamBot::Client>);
    void process() const;

public:
    Processor(std::shared_ptr<SteamBot::Client>);
};

/************************************************************************/

template <typename T, typename U=uint32_t> class Counters
{
    typedef  std::pair<T, U> Entry;
    std::vector<Entry> counters;

public:
    U& set(T key)
    {
        for (auto& entry: counters)
        {
            if (entry.first==key)
            {
                return entry.second;
            }
        }
        auto &result=counters.emplace_back();
        result.first=key;
        return result.second;
    }

    U& add(T key)
    {
        return ++set(key);
    }

    U get(T key) const
    {
        for (auto& entry: counters)
        {
            if (entry.first==key)
            {
                return entry.second;
            }
        }
        return U{};
    }

    bool empty() const
    {
        return counters.empty();
    }

    void sort()
    {
        std::sort(counters.begin(), counters.end(),
                  [](const Entry& left, const Entry &right)
                  {
                      return left.second>right.second;
                  });
    }

    void print()
    {
        sort();
        for (const auto& entry: counters)
        {
            std::cout << "   " << entry.second << " \xC3\x97 " << SteamBot::enumToString(entry.first) << "\n";
        }
    }

    const auto& getCounters() const
    {
        return counters;
    }
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

void Processor::process() const
{
    // Count for licenses that are not SinglePurchase
    Counters<SteamBot::LicenseType> weird;

    Counters<SteamBot::PaymentMethod> payment;
    static constexpr auto paymentStore=static_cast<SteamBot::PaymentMethod>(9999);

    struct
    {
        uint32_t freePromotion=0;
        uint32_t freeOnDemand=0;
    } complimentaryType;

    struct AppTypeInfo
    {
        uint32_t total=0;
        uint32_t earlyAccess=0;
        uint32_t freePromotion=0;

        bool operator>(const AppTypeInfo& right) const
        {
            return total>right.total;
        }

        AppTypeInfo& operator++()
        {
            ++total;
            return *this;
        }
    };

    Counters<SteamBot::AppType, AppTypeInfo> appTypes;
    Counters<SteamBot::BillingType> billing;

    std::vector<std::shared_ptr<const Licenses::LicenseInfo>> noPackageData;

    std::unordered_map<SteamBot::AppID, uint32_t> appCounters;

    for (const auto& pair: licenses->licenses)
    {
        auto licenseType=pair.second->licenseType;
        if (licenseType!=SteamBot::LicenseType::SinglePurchase)
        {
            weird.add(licenseType);
        }

        auto paymentMethod=pair.second->paymentMethod;
        switch (paymentMethod)
        {
        case SteamBot::PaymentMethod::None:
        case SteamBot::PaymentMethod::ActivationCode:
        case SteamBot::PaymentMethod::Complimentary:
            payment.add(paymentMethod);
            break;

        default:
            payment.add(paymentStore);
            break;
        }

        const auto package=SteamBot::Modules::PackageData::getPackageInfo(*pair.second);
        if (package)
        {
            auto billingType=SteamBot::getBillingType(*package);
            billing.add(billingType);

            bool freePromotion=SteamBot::getFreePromotion(*package);

            if (paymentMethod==SteamBot::PaymentMethod::Complimentary)
            {
                if (freePromotion)
                {
                    complimentaryType.freePromotion++;
                }
                if (billingType==SteamBot::BillingType::FreeOnDemand)
                {
                    complimentaryType.freeOnDemand++;
                }
            }

            for (const SteamBot::AppID appId: package->appIds)
            {
                auto appType=SteamBot::AppInfo::getAppType(appId);
                auto& info=appTypes.set(appType);
                if (appCounters[appId]++==0)
                {
                    info.total++;
                    if (SteamBot::AppInfo::isEarlyAccess(appId)) info.earlyAccess++;
                }
                if (freePromotion)
                {
                    info.freePromotion++;
                }
            }
        }
        else
        {
            noPackageData.push_back(pair.second);
        }
    }

    if (!weird.empty())
    {
        std::cout << "You have licenses that are not \"" << SteamBot::enumToString(SteamBot::LicenseType::SinglePurchase) << "\":\n";
        weird.print();
    }

    if (!noPackageData.empty())
    {
        std::cout << "No package data for " << noPackageData.size() << " licenses???\n";
    }

    std::cout << "Your payment types are:\n";
    payment.sort();
    for (const auto& entry: payment.getCounters())
    {
        std::string_view name=SteamBot::enumToString(entry.first);
        if (entry.first==paymentStore)
        {
            assert(name.empty());
            name="Steam-store";
        }
        std::cout << "   " << entry.second << " \xC3\x97 " << name << "\n";
        if (entry.first==SteamBot::PaymentMethod::Complimentary)
        {
            if (complimentaryType.freePromotion!=0)
            {
                std::cout << "      " << complimentaryType.freePromotion << " \xC3\x97 free promotion\n";
            }
            if (complimentaryType.freeOnDemand!=0)
            {
                std::cout << "      " << complimentaryType.freeOnDemand << " \xC3\x97 F2P\n";
            }
        }
    }

    std::cout << "You have these app types:\n";
    appTypes.sort();
    for (const auto& entry: appTypes.getCounters())
    {
        std::cout << "   " << entry.second.total << " \xC3\x97 " << SteamBot::enumToString(entry.first);

        {
            const char *separator=" (";
            if (entry.second.earlyAccess>0)
            {
                std::cout << separator << entry.second.earlyAccess << " early access";
                separator=", ";
            }
            if (entry.second.freePromotion>0)
            {
                std::cout << separator << entry.second.freePromotion << " free promotion";
                separator=", ";
            }
            if (separator[0]==',')
            {
                std::cout << ")";
            }
        }
        std::cout << "\n";
    }
}

/************************************************************************/

Processor::Processor(std::shared_ptr<SteamBot::Client> client)
{
    getWhiteboardData(std::move(client));
    if (licenses)
    {
        std::cout << licenses->licenses.size() << " licenses\n";
        process();
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
