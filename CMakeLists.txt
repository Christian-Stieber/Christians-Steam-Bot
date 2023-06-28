cmake_minimum_required(VERSION 3.23)

project(ChristiansSteamBot CXX)
add_executable(${PROJECT_NAME})

include(Christians-Steam-Framework/CMakefile.Common)

######################################################################

add_subdirectory(Christians-Steam-Framework)

######################################################################

list(APPEND SRCS Christians-Steam-Bot/Sources/Main.cpp)

######################################################################

target_sources(${PROJECT_NAME} PRIVATE ${SRCS})

target_link_libraries(${PROJECT_NAME} ChristiansSteamFramework)
