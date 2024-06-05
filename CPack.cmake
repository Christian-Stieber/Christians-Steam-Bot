# cd build/Debug; cpack -G DEB CPackConfig.cmake
#   dpkg --contents ChristiansSteamBot-0.0.1-Linux.deb
#   dpkg-deb -I ChristiansSteamBot-0.0.1-Linux.deb

# https://stackoverflow.com/questions/40445763/how-to-idiomatically-set-the-version-number-in-a-cmake-cpack-project
set(CPACK_PACKAGE_VERSION_MAJOR ${CMAKE_PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${CMAKE_PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${CMAKE_PROJECT_VERSION_PATCH})

set(CPACK_PACKAGE_VENDOR "Christian Stieber")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A work-in-progress tool to interact with Steam")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_CONTACT "Christian Stieber <stieber.chr@gmail.com>")

set(CPACK_DEBIAN_PACKAGE_SECTION "misc")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
