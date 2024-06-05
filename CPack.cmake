# cd build/Debug; cpack -G DEB CPackConfig.cmake
#   dpkg --contents ChristiansSteamBot-0.0.1-Linux.deb
#   dpkg-deb -I ChristiansSteamBot-0.0.1-Linux.deb

# https://stackoverflow.com/questions/40445763/how-to-idiomatically-set-the-version-number-in-a-cmake-cpack-project
set(CPACK_PACKAGE_VERSION_MAJOR ${CMAKE_PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${CMAKE_PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${CMAKE_PROJECT_VERSION_PATCH})

# Common settings
# https://cmake.org/cmake/help/latest/module/CPack.html#variables-common-to-all-cpack-generators
set(CPACK_PACKAGE_NAME "Christians Steam Bot")
set(CPACK_PACKAGE_VENDOR "Christian Stieber")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A work-in-progress tool to interact with Steam")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_CONTACT "Christian Stieber <stieber.chr@gmail.com>")

# DEB settings
# https://cmake.org/cmake/help/latest/cpack_gen/deb.html#cpack_gen:CPack%20DEB%20Generator
set(CPACK_DEBIAN_PACKAGE_SECTION "misc")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

# WiX settings
# https://cmake.org/cmake/help/latest/cpack_gen/wix.html#cpack_gen:CPack%20WIX%20Generator
set(CPACK_WIX_PROGRAM_MENU_FOLDER "Christians Steam Bot")
set(CPACK_WIX_CULTURES "en-US")
set(CPACK_WIX_UPGRADE_GUID "E4B4DD75-C3BF-4BB1-9F72-96C8FA65C639")
if (CPACK_GENERATOR STREQUAL "WIX")
  file(DOWNLOAD "https://www.gnu.org/licenses/gpl-3.0.rtf" "LICENSE.rtf")
  set(CPACK_RESOURCE_FILE_LICENSE "LICENSE.rtf")
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "Christians Steam Bot")
endif()
