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

#include "AssetData.hpp"
#include "Helpers/StringCompare.hpp"
#include "Modules/Inventory.hpp"
#include "Modules/Executor.hpp"

#include <regex>

/************************************************************************/

typedef SteamBot::Inventory::Inventory Inventory;

/************************************************************************/

namespace
{
    class ListInventoryCommand : public SteamBot::UI::CommandBase
    {
    public:
        virtual bool global() const
        {
            return false;
        }

        virtual const std::string_view& command() const override
        {
            static const std::string_view string("list-inventory");
            return string;
        }

        virtual const std::string_view& description() const override
        {
            static const std::string_view string("list steam inventory items");
            return string;
        }

        virtual const boost::program_options::options_description* options() const override
        {
            static auto const options=[](){
                auto options=new boost::program_options::options_description();
                options->add_options()
                    ("tradable", boost::program_options::bool_switch()->value_name("tradable")->default_value(false), "only tradable items")
                    ("items", boost::program_options::value<SteamBot::OptionRegex>()->value_name("regex"), "items")
                    ;
                return options;
            }();
            return options;
        }

    public:
        class Execute : public ExecuteBase
        {
        private:
            bool tradable=false;
            std::optional<SteamBot::OptionRegex> itemsRegex;

        public:
            using ExecuteBase::ExecuteBase;

            virtual ~Execute() =default;

        private:
            void outputInventory(const Inventory&) const;

        public:
            virtual bool init(const boost::program_options::variables_map& options) override
            {
                tradable=options["tradable"].as<bool>();
                if (options.count("items"))
                {
                    itemsRegex=options["items"].as<SteamBot::OptionRegex>();
                }
                return true;
            }

            virtual void execute(SteamBot::ClientInfo*) const;
        };

        virtual std::shared_ptr<ExecuteBase> makeExecute(SteamBot::UI::CLI& cli) const override
        {
            return std::make_shared<Execute>(cli);
        }
    };

    ListInventoryCommand::Init<ListInventoryCommand> init;
}

/************************************************************************/

void ListInventoryCommand::Execute::outputInventory(const Inventory& inventory) const
{
    class Item
    {
    public:
        std::shared_ptr<const SteamBot::Inventory::Item> inventoryItem;
        std::shared_ptr<const SteamBot::AssetData::AssetInfo> assetInfo;

    public:
        Item(decltype(inventoryItem) inventoryItem_, decltype(assetInfo) assetInfo_)
            : inventoryItem(std::move(inventoryItem_)), assetInfo(std::move(assetInfo_))
        {
        }
    };

    std::vector<Item> items;
    {
        for (const auto& item : inventory.items)
        {
            if (auto assetInfo=SteamBot::AssetData::query(item))
            {
                if (!tradable || assetInfo->isTradable)
                {
                    if (!itemsRegex || std::regex_search(assetInfo->name, *itemsRegex) || std::regex_search(assetInfo->type, *itemsRegex))
                    {
                        items.emplace_back(item, assetInfo);
                    }
                }
            }
        }
    }

    std::sort(items.begin(), items.end(), [](const Item& left, const Item& right) -> bool {
        auto result=SteamBot::caseInsensitiveStringCompare(left.assetInfo->type, right.assetInfo->type);
        if (result==decltype(result)::equivalent)
        {
            result=SteamBot::caseInsensitiveStringCompare(left.assetInfo->name, right.assetInfo->name);
        }
        return result==std::weak_ordering::less;
    });

    SteamBot::UI::OutputText output;
    for (const auto& item : items)
    {
        output << "\n";

        output << toInteger(item.inventoryItem->appId);
        output << "/" << toInteger(item.inventoryItem->contextId);
        output << "/" << toInteger(item.inventoryItem->assetId);
        output << ": ";

        if (item.inventoryItem->amount>1)
        {
            output << item.inventoryItem->amount << "× ";
        }

        output << "\"" << item.assetInfo->type << "\" / \"" << item.assetInfo->name << "\"";
    }
}

/************************************************************************/

void ListInventoryCommand::Execute::execute(SteamBot::ClientInfo* clientInfo) const
{
    if (auto client=clientInfo->getClient())
    {
        bool success=SteamBot::Modules::Executor::executeWithFiber(client, [self=shared_from_this<Execute>()](SteamBot::Client& client) {
            SteamBot::UI::OutputText() << "ClI: list inventory";
            auto inventory=SteamBot::Inventory::get();
            if (inventory)
            {
                self->outputInventory(*inventory);
            }
            else
            {
                SteamBot::UI::OutputText() << "inventory not available";
            }
        });
        if (success)
        {
            std::cout << "started inventory loading for " << client->getClientInfo().accountName << std::endl;
        }
    }
}
