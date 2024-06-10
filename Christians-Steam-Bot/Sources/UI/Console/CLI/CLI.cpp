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
#include <limits>

#undef max

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
        auto commands_=new CommandList;
        SteamBot::Startup::InitBase<SteamBot::UI::CommandBase>::create([commands_](std::unique_ptr<SteamBot::UI::CommandBase> command) {
            const auto key=command->command();
            const bool success=commands_->try_emplace(key, std::move(command)).second;
            assert(success);
        });
        return commands_;
    }());

    return commands;
}

/************************************************************************/

const boost::program_options::positional_options_description* SteamBot::UI::CommandBase::positionals() const
{
    return nullptr;
}

/************************************************************************/

const boost::program_options::options_description* SteamBot::UI::CommandBase::options() const
{
    return nullptr;
}

/************************************************************************/

bool SteamBot::UI::CommandBase::parse(const std::vector<std::string>& args, boost::program_options::variables_map& variables) const
{
    try
    {
        if (auto desc=options())
        {
            boost::program_options::basic_command_line_parser parser(args);
            parser.options(*desc);
            if (auto positional=positionals())
            {
                parser.positional(*positional);
            }
            boost::program_options::store(parser.run(), variables);
            boost::program_options::notify(variables);
        }
        else
        {
            if (args.size()>0)
            {
                throw false;
            }
        }
        return true;
    }
    catch(...)
    {
        return false;
    }
}

/************************************************************************/

void SteamBot::UI::CommandBase::print(std::ostream& stream) const
{
    if (!global())
    {
        stream << "[<account>:] ";
    }

    stream << command();

    if (options())
    {
        size_t maxLength=0;
        std::vector<std::string> strings;
        {
            strings.reserve(options()->options().size());

            std::string optional;
            if (auto positional=positionals())
            {
                if (positional->max_total_count()==std::numeric_limits<unsigned>::max())
                {
                    optional=positional->name_for_position(100);
                }
            }

            for (const auto& option : options()->options())
            {
                {
                    std::string string;

                    {
                        std::string name=option->format_name();
                        if (option->long_name()==optional)
                        {
                            string='[';
                            string.append(std::move(name));
                            string+=']';
                            optional.clear();
                        }
                        else
                        {
                            string=std::move(name);
                        }
                    }

                    {
                        std::string parameter=option->format_parameter();
                        if (!parameter.empty())
                        {
                            string.append(" ");
                            string.append(std::move(parameter));
                        }
                    }

                    strings.push_back(std::move(string));
                }

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
            stream << " : " << options()->options()[i]->description();
        }
    }

    stream << std::endl;
}

/************************************************************************/

void CLI::printHelp(const std::string* theCommand)
{
    const auto& commands=getCommands();

    if (theCommand!=nullptr)
    {
        auto iterator=commands.find(*theCommand);
        if (iterator!=commands.end())
        {
            iterator->second->print(std::cout);
        }
        else
        {
            std::cout << "unknown command \"" << *theCommand << "\"" << std::endl;
        }
    }
    else
    {
        size_t maxLength=0;
        for (const auto& command : commands)
        {
            if (command.first.size()>maxLength)
            {
                maxLength=command.first.size();
            }
        }

        std::cout << "valid commands:";
        for (const auto& command : commands)
        {
            std::cout << "\n   " << command.first;
            for (size_t i=command.first.size(); i<maxLength; i++)
            {
                std::cout << " ";
            }
            std::cout << " : " << command.second->description();
        }
        std::cout << std::endl;
    }
}

/************************************************************************/
/*
 * Returns a list of account names to use.
 *
 * Expands things like @groupname or *, or just copies the name.
 */

static std::vector<SteamBot::ClientInfo*> expandAccountName(std::string_view name)
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

            boost::program_options::variables_map options;
            if (command.parse(args, options))
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
                else
                {
                    command.print(std::cout);
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
            printHelp(nullptr);
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
                std::cout << "[" << currentAccount->displayName() << "] ";
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
