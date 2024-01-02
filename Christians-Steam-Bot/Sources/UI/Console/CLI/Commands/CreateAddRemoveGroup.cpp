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

#include "Helpers/JSON.hpp"

/************************************************************************/

namespace
{
    enum ChangeMode { Create, Add, Remove };
}

/************************************************************************/

namespace
{
    class Execute : public SteamBot::UI::CommandBase::ExecuteBase
    {
    private:
        ChangeMode changeMode;
        std::string group;
        std::vector<SteamBot::OptionBotName> accounts;

    public:
        Execute(SteamBot::UI::CLI& cli_, ChangeMode changeMode_)
            : ExecuteBase(cli_), changeMode(changeMode_)
        {
        }

        virtual ~Execute() =default;

    public:
        virtual bool init(const boost::program_options::variables_map& options) override
        {
            assert(options.count("group"));
            assert(options.count("account"));
            group=options["group"].as<std::string>();
            accounts=options["account"].as<decltype(accounts)>();
            return true;
        }

    private:
        bool checkGroup(const std::vector<SteamBot::ClientInfo*>& clients) const
        {
            switch(changeMode)
            {
            case ChangeMode::Create:
                return clients.empty();

            case ChangeMode::Add:
            case ChangeMode::Remove:
                return !clients.empty();
            }

            assert(false);
            return false;
        }

        virtual void execute(SteamBot::ClientInfo*) const override
        {
            std::string_view groupName=group;
            if (groupName.starts_with('@'))
            {
                groupName.remove_prefix(1);
            }
            auto clients=SteamBot::ClientInfo::getGroup(groupName);
            if (checkGroup(clients))
            {
                for (const auto& account : accounts)
                {
                    auto info=account.clientInfo;
                    assert(info!=nullptr);

                    auto& dataFile=SteamBot::DataFile::get(info->accountName, SteamBot::DataFile::FileType::Account);
                    dataFile.update([changeMode=this->changeMode, groupName](boost::json::value& json) {
                        auto& array=SteamBot::JSON::createItem(json, "Groups");
                        if (array.is_null())
                        {
                            array.emplace_array();
                        }
                        for (auto iterator=array.as_array().begin(); iterator!=array.as_array().end(); ++iterator)
                        {
                            if (iterator->as_string()==groupName)
                            {
                                switch(changeMode)
                                {
                                case ChangeMode::Create:
                                case ChangeMode::Add:
                                    return false;

                                case ChangeMode::Remove:
                                    array.as_array().erase(iterator);
                                    if (array.as_array().empty())
                                    {
                                        SteamBot::JSON::eraseItem(json, "Groups");
                                    }
                                    return true;
                                }
                            }
                        }
                        switch(changeMode)
                        {
                        case ChangeMode::Create:
                        case ChangeMode::Add:
                            array.as_array().emplace_back(groupName);
                            return true;

                        case ChangeMode::Remove:
                            return false;
                        }

                        assert(false);
                        return false;
                    });
                }
            }
            else
            {
                switch(changeMode)
                {
                case ChangeMode::Create:
                    std::cout << "cannot create group \"" << groupName << "\": group already exists" << std::endl;
                    break;

                case ChangeMode::Add:
                    std::cout << "cannot add to group \"" << groupName << "\": no such group" << std::endl;
                    break;

                case ChangeMode::Remove:
                    std::cout << "cannot remove from group \"" << groupName << "\": no such group" << std::endl;
                    break;
                }
            }
        }
    };
}

/************************************************************************/

namespace
{
    class CommandBase : public SteamBot::UI::CommandBase
    {
    public:
        virtual bool global() const
        {
            return true;
        }

        virtual const boost::program_options::positional_options_description* positionals() const override
        {
            static auto const positional=[](){
                auto positional=new boost::program_options::positional_options_description();
                positional->add("group", 1);
                positional->add("account", -1);
                return positional;
            }();
            return positional;
        }
    };
}

/************************************************************************/

namespace
{
    class CreateGroupCommand : public CommandBase
    {
    public:
        virtual const std::string_view& command() const override
        {
            static const std::string_view string("create-group");
            return string;
        }

        virtual const std::string_view& description() const override
        {
            static const std::string_view string("create a new group");
            return string;
        }

        virtual const boost::program_options::options_description* options() const override
        {
            static auto const options=[](){
                auto options=new boost::program_options::options_description();
                options->add_options()
                    ("group",
                     boost::program_options::value<std::string>()->value_name("name")->required(),
                     "group id to create")
                    ("account",
                     boost::program_options::value<std::vector<SteamBot::OptionBotName>>()->value_name("bot-name")->multitoken()->required(),
                     "accounts to add")
                    ;
                return options;
            }();
            return options;
        }

    public:
        virtual std::shared_ptr<ExecuteBase> makeExecute(SteamBot::UI::CLI& cli) const override
        {
            return std::make_shared<Execute>(cli, ChangeMode::Create);
        }
    };

    CreateGroupCommand::Init<CreateGroupCommand> createInit;
}

/************************************************************************/

namespace
{
    class AddGroupCommand : public CommandBase
    {
    public:
        virtual const std::string_view& command() const override
        {
            static const std::string_view string("add-group");
            return string;
        }

        virtual const std::string_view& description() const override
        {
            static const std::string_view string("add to a group");
            return string;
        }

        virtual const boost::program_options::options_description* options() const override
        {
            static auto const options=[](){
                auto options=new boost::program_options::options_description();
                options->add_options()
                    ("group",
                     boost::program_options::value<std::string>()->value_name("name")->required(),
                     "group id to add to")
                    ("account",
                     boost::program_options::value<std::vector<SteamBot::OptionBotName>>()->value_name("bot-name")->multitoken()->required(),
                     "accounts to add")
                    ;
                return options;
            }();
            return options;
        }

    public:
        virtual std::shared_ptr<ExecuteBase> makeExecute(SteamBot::UI::CLI& cli) const override
        {
            return std::make_shared<Execute>(cli, ChangeMode::Add);
        }
    };

    AddGroupCommand::Init<AddGroupCommand> addInit;
}

/************************************************************************/

namespace
{
    class RemoveGroupCommand : public CommandBase
    {
    public:
        virtual const std::string_view& command() const override
        {
            static const std::string_view string("remove-group");
            return string;
        }

        virtual const std::string_view& description() const override
        {
            static const std::string_view string("remove to a group");
            return string;
        }

        virtual const boost::program_options::options_description* options() const override
        {
            static auto const options=[](){
                auto options=new boost::program_options::options_description();
                options->add_options()
                    ("group",
                     boost::program_options::value<std::string>()->value_name("name")->required(),
                     "group id to remove to")
                    ("account",
                     boost::program_options::value<std::vector<SteamBot::OptionBotName>>()->value_name("bot-name")->multitoken()->required(),
                     "accounts to remove")
                    ;
                return options;
            }();
            return options;
        }

    public:
        virtual std::shared_ptr<ExecuteBase> makeExecute(SteamBot::UI::CLI& cli) const override
        {
            return std::make_shared<Execute>(cli, ChangeMode::Remove);
        }
    };

    RemoveGroupCommand::Init<RemoveGroupCommand> removeInit;
}
