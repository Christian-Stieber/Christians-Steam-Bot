cmake_minimum_required(VERSION 3.23)

project(ChristiansSteamBot CXX)
add_executable(${PROJECT_NAME})

include(Christians-Steam-Framework/CMakefile.Common)

######################################################################

add_subdirectory(Christians-Steam-Framework)

######################################################################

list(APPEND SRCS Main)

list(APPEND SRCS UI/Console/Console UI/Console/Manager_Linux)
list(APPEND SRCS UI/Console/CLI/CLI UI/Console/CLI/Helpers UI/Console/CLI/TokenizeLine)
list(APPEND SRCS UI/Console/CLI/Commands/Exit UI/Console/CLI/Commands/Launch UI/Console/CLI/Commands/Help)

#list(APPEND SRCS UI/Console/CLI/Commands/ListGames UI/Console/CLI/Commands/Status UI/Console/CLI/Commands/Select)
#list(APPEND SRCS UI/Console/CLI/Commands/Create UI/Console/CLI/Commands/PlayStopGame)
#list(APPEND SRCS UI/Console/CLI/Commands/AddLicense UI/Console/CLI/Commands/DiscoveryQueue UI/Console/CLI/Commands/SaleEvent UI/Console/CLI/Commands/Quit)
#list(APPEND SRCS UI/Console/CLI/Commands/Settings UI/Console/CLI/Commands/ListInventory UI/Console/CLI/Commands/SaleSticker)
#list(APPEND SRCS UI/Console/CLI/Commands/SendInventory UI/Console/CLI/Commands/AcceptTrade UI/Console/CLI/Commands/DeclineTrade UI/Console/CLI/Commands/CancelTrade)
#list(APPEND SRCS UI/Console/CLI/Commands/SaleQueue UI/Console/CLI/Commands/ListTradeOffers UI/Console/CLI/Commands/CreateAddRemoveGroup UI/Console/CLI/Commands/ListGroups)

######################################################################

list(TRANSFORM SRCS PREPEND "Christians-Steam-Bot/Sources/")
list(TRANSFORM SRCS APPEND ".cpp")

######################################################################

target_sources(${PROJECT_NAME} PRIVATE ${SRCS})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Christians-Steam-Bot/Headers")

target_link_libraries(${PROJECT_NAME} ChristiansSteamFramework)
