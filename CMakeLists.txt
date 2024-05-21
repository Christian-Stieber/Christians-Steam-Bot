######################################################################
#
# Use with a command like
#   cmake -S . -B build/Debug -D CMAKE_BUILD_TYPE=Debug
#   cmake --build build/Debug

# set(CMAKE_VERBOSE_MAKEFILE ON)

######################################################################

cmake_minimum_required(VERSION 3.25)

project(ChristiansSteamBot CXX)
add_executable(${PROJECT_NAME})

######################################################################

add_subdirectory(Christians-Steam-Framework)
target_link_libraries(${PROJECT_NAME} PRIVATE ChristiansSteamFramework)

######################################################################

include(Christians-Steam-Framework/CompileOptions.cmake)
setCompileOptions(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Christians-Steam-Bot/Headers")

######################################################################

function(addSource directory)
  list(TRANSFORM ARGN PREPEND "Christians-Steam-Bot/Sources/${directory}/")
  list(TRANSFORM ARGN APPEND ".cpp")
  target_sources(${PROJECT_NAME} PRIVATE ${ARGN})
endfunction(addSource)

addSource("." Main Asan)
addSource("UI/Console" Console Table Manager_Linux)
addSource("UI/Console/CLI" CLI Helpers)

addSource("UI/Console/CLI/Commands"
  Launch Quit Status Select Create Exit Help HandleTrade ListTradeOffers ListGames AddLicense
  DiscoveryQueue SaleSticker SaleQueue SaleEvent ListInventory SendInventory PlayStopGame
  ViewStream StopStream CreateAddRemoveGroup ListGroups)
