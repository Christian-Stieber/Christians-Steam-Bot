#include "Startup.hpp"

#include <string_view>
#include <memory>

#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

/************************************************************************/

namespace SteamBot
{
    class ClientInfo;
}

/************************************************************************/

namespace SteamBot
{
    namespace UI
    {
        class CLI;
    }
}

/************************************************************************/

namespace SteamBot
{
    namespace UI
    {
        class CommandBase
        {
        public:
            virtual bool global() const =0;
            virtual const std::string_view& command() const =0;

            virtual const boost::program_options::options_description* options() const;
            virtual const boost::program_options::positional_options_description* positionals() const;

            bool parse(const std::vector<std::string>&, boost::program_options::variables_map&) const;

            void print(std::ostream&) const;

        public:
            // Note: execute() can be called on multiple clients concurrently!
            class ExecuteBase
            {
            public:
                SteamBot::UI::CLI& cli;

            public:
                ExecuteBase(SteamBot::UI::CLI& cli_)
                    : cli(cli_)
                {
                }

                virtual ~ExecuteBase() =default;

                virtual bool init(const boost::program_options::variables_map&)
                {
                    return true;
                }

                virtual void execute(SteamBot::ClientInfo*) const =0;
            };

            virtual std::unique_ptr<ExecuteBase> makeExecute(SteamBot::UI::CLI&) const =0;

        public:
            template <typename T> using Init=SteamBot::Startup::Init<CommandBase, T>;
        };
    }
}
