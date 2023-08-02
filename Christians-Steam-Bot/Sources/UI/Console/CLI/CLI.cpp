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

#include "UI/Command.hpp"
#include "./Helpers.hpp"
#include "Vector.hpp"

#include <map>

/************************************************************************/

CLI::CLI(ConsoleUI& ui_)
    : ui(ui_),
      helpers(std::make_unique<Helpers>(*this))
{
}

/************************************************************************/

CLI::~CLI() =default;

/************************************************************************/

static const auto& getCommands()
{
    typedef std::map<std::string_view, std::unique_ptr<SteamBot::UI::CommandBase>> CommandList;

    static const CommandList& commands=*([](){
        auto commands=new CommandList;
        SteamBot::Startup::InitBase<SteamBot::UI::CommandBase>::initAll([commands](std::unique_ptr<SteamBot::UI::CommandBase> command) {
            const auto key=command->command();
            const bool success=commands->try_emplace(key, std::move(command)).second;
            assert(success);
        });
        return commands;
    }());

    return commands;
}

/************************************************************************/

void SteamBot::UI::CommandBase::print(std::ostream& stream) const
{
    if (!global())
    {
        stream << "[<account>:] ";
    }

    stream << command();

    size_t maxLength=0;
    std::vector<std::string> strings;
    {
        strings.reserve(options().options().size());
        for (const auto& option : options().options())
        {
            strings.push_back(option->format_parameter());

            size_t length=strings.back().size();
            if (length>maxLength)
            {
                maxLength=length;
            }
        }
    }

    for (size_t i=0; i<strings.size(); i++)
    {
        stream << "\n   " << strings[i];
        for (size_t tab=maxLength-strings[i].size(); tab>0; tab--)
        {
            stream << " ";
        }
        stream << " : " << options().options()[i]->description();
    }

    stream << std::endl;
}

/************************************************************************/

void CLI::listCommands()
{
    std::cout << "valid commands:";
    for (const auto& command : getCommands())
    {
        std::cout << "\n   " << command.first;
    }
    std::cout << std::endl;
}

/************************************************************************/
/*
 * Returns a list of account names to use.
 *
 * Expands things like @groupname or *, or just copies the name.
 */

std::vector<SteamBot::ClientInfo*> expandAccountName(std::string_view name)
{
    std::vector<SteamBot::ClientInfo*> result;

    if (name.starts_with('@'))
    {
        name.remove_prefix(1);
        result=SteamBot::ClientInfo::getGroup(name);
        if (result.empty())
        {
            std::cout << "group \"" << name << "\" not found" << std::endl;
        }
    }
    else if (name=="*")
    {
        result=SteamBot::ClientInfo::getClients();
        SteamBot::erase(result, [](const SteamBot::ClientInfo* info) {
            return !info->getClient();
        });
        if (result.empty())
        {
            std::cout << "no running accounts found" << std::endl;
        }
    }
    else
    {
        if (auto info=SteamBot::ClientInfo::find(name))
        {
            result.push_back(info);
        }
        else
        {
            std::cout << "unknown account \"" << name << "\"" << std::endl;
        }
    }

    return result;
}

/************************************************************************/
/*
 * Note: commands can be prefixed with "<accountname>:" as the first
 * word (even if it doesn't make sense, like "account: help").
 */

void CLI::command(const std::string& line)
{
    std::vector<SteamBot::ClientInfo*> clients;

    auto args=boost::program_options::split_unix(line);

    if (args.size()>0)
    {
        if (args[0].size()>0 && args[0].back()==':')
        {
            auto name=std::move(args[0]);
            name.pop_back();
            args.erase(args.begin());

            clients=expandAccountName(name);
            if (clients.empty())
            {
                return;
            }
        }

        if (clients.empty())
        {
            if (currentAccount!=nullptr)
            {
                clients.push_back(currentAccount);
            }
        }
    }

    if (args.size()>0)
    {
        auto iterator=getCommands().find(args[0]);
        if (iterator!=getCommands().end())
        {
            args.erase(args.begin());

            auto& command=*(iterator->second);

            bool parseSuccess=false;
            boost::program_options::variables_map options;
            try
            {
                boost::program_options::basic_command_line_parser parser(args);
                parser.options(command.options());
                boost::program_options::store(parser.run(), options);
                parseSuccess=true;
            }
            catch(...)
            {
            }

            if (parseSuccess)
            {
                auto execute=command.makeExecute(*this);
                if (execute->init(options))
                {
                    if (command.global())
                    {
                        execute->execute(nullptr);
                    }
                    else
                    {
                        if (!clients.empty())
                        {
                            std::chrono::seconds delay(0);
                            for (SteamBot::ClientInfo* clientInfo : clients)
                            {
                                // ToDo: add some trickery to run execute() on the client thread, but
                                // only for commands that are not "launch"
                                boost::this_fiber::sleep_for(delay);
                                execute->execute(clientInfo);
                                delay+=std::chrono::seconds(2);
                            }
                        }
                        else
                        {
                            std::cout << "no current account; select one first or specify an account name" << std::endl;
                        }
                    }
                }
            }
            else
            {
                command.print(std::cout);
            }
        }
        else
        {
            std::cout << "unknown command: \"" << args[0] << "\"" << std::endl;
            listCommands();
        }
    }
}

/************************************************************************/

void CLI::run()
{
    typedef SteamBot::UI::ConsoleUI::ManagerBase ManagerBase;

    ui.manager->setMode(ManagerBase::Mode::LineInput);
    {
        std::cout << "Command line mode is now active." << std::endl;
        std::cout << "End it by entering an empty line." << std::endl;
        while (!quit)
        {
            if (currentAccount!=nullptr)
            {
                std::cout << "[" << currentAccount->accountName << "] ";
            }
            std::cout << "command> " << std::flush;
            std::string line;
            std::getline(std::cin, line);
            if (line.empty())
            {
                break;
            }
            command(line);
        }
        std::cout << "Command line mode ended." << std::endl;
    }
    if (quit)
    {
        SteamBot::UI::Thread::quit();
    }
    else
    {
        ui.manager->setMode(ManagerBase::Mode::NoInput);
    }
}
