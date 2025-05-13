######################################################################
#
# Use with a command like
#   cmake -S . -B build/Debug -D CMAKE_BUILD_TYPE=Debug
#   cmake --build build/Debug

# set(CMAKE_VERBOSE_MAKEFILE ON)

######################################################################

cmake_minimum_required(VERSION 3.25)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(DEFINED ENV{CHRISTIAN_VERSION})
  set(CHRISTIAN_VERSION "$ENV{CHRISTIAN_VERSION}")
else()
  set(CHRISTIAN_VERSION "0.0.0")
endif()
string(REPLACE "release/" "" CHRISTIAN_VERSION "${CHRISTIAN_VERSION}")
message("Version: ${CHRISTIAN_VERSION}")

project(ChristiansSteamBot
  LANGUAGES CXX
  HOMEPAGE_URL "https://github.com/Christian-Stieber/Christians-Steam-Bot"
  VERSION ${CHRISTIAN_VERSION}
)

add_executable(${PROJECT_NAME})

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_options(${PROJECT_NAME} PRIVATE "-static-libstdc++" "-static-libgcc")
endif()

if (MSVC)
  target_link_options(${PROJECT_NAME} PRIVATE "/SUBSYSTEM:CONSOLE,6.1")
endif()


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
addSource("UI" Command)
addSource("UI/Console" Console Table Manager_Linux Manager_Windows GetLine_Linux)
addSource("UI/Console/CLI" CLI Helpers)

addSource("UI/Console/CLI/Commands"
  Launch Quit Status Select Create Exit Help HandleTrade ListTradeOffers ListGames AddLicense
  DiscoveryQueue SaleSticker SaleQueue SaleEvent ListInventory SendInventory PlayStopGame LoadURL
  ViewStream StopStream CreateAddRemoveGroup ListGroups Settings ShowLicense ListFiles)

######################################################################

if(WIN32)
  install(TARGETS ${PROJECT_NAME} DESTINATION ".")
else()
  install(TARGETS ${PROJECT_NAME})
endif()

set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME};Christians Steam Bot")

######################################################################

configure_file("${PROJECT_SOURCE_DIR}/CPack.cmake" "${PROJECT_BINARY_DIR}/CPackOptions.cmake" @ONLY)
set (CPACK_PROJECT_CONFIG_FILE "${PROJECT_BINARY_DIR}/CPackOptions.cmake")

include(CPack)
