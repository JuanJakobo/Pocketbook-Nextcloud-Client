CMAKE_MINIMUM_REQUIRED (VERSION 3.27.5)
PROJECT (Pocketbook-Nextcloud VERSION 1.10)


add_compile_options(-std=c++17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Where to build the targets
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

#Set if building with docker
SET(PB_INCLUDE_DIRECTORIES /SDK/usr/arm-obreey-linux-gnueabi/sysroot/usr/local/include/)
SET(FREETYPE_INCLUDE_DIRS /SDK/usr/arm-obreey-linux-gnueabi/sysroot/usr/include/freetype2/)
#Set if building without docker
# SET(SDK_BASE <SDK-PATH>)
# SET(PB_INCLUDE_DIRECTORIES ${SDK_BASE}/usr/arm-obreey-linux-gnueabi/sysroot/usr/local/include/)
# SET(FREETYPE_INCLUDE_DIRS ${SDK_BASE}/usr/arm-obreey-linux-gnueabi/sysroot/usr/include/freetype2/)
# SET(CMAKE_TOOLCHAIN_FILE ${SDK_BASE}/share/cmake/arm_conf.cmake)

# Include the SDK
include("${CMAKE_TOOLCHAIN_FILE}")

# Where to find pbres
string(REGEX REPLACE "[^/]+$" "pbres" PBRES "${CMAKE_C_COMPILER}")

# Add the standard link directories
link_directories("${PB_LINK_DIRECTORIES}")

# Add the standard include directories
include_directories("${PB_INCLUDE_DIRECTORIES}")
include_directories("${FREETYPE_INCLUDE_DIRS}")

include_directories(
	#$(TARGET_INCLUDE)
    ${CMAKE_SOURCE_DIR}/src/handler/
    ${CMAKE_SOURCE_DIR}/src/util/
    ${CMAKE_SOURCE_DIR}/src/ui/
    ${CMAKE_SOURCE_DIR}/src/ui/webDAVView/
    ${CMAKE_SOURCE_DIR}/src/ui/fileView/
    ${CMAKE_SOURCE_DIR}/src/ui/loginView/
    ${CMAKE_SOURCE_DIR}/src/ui/excludeFileView/
    ${CMAKE_SOURCE_DIR}/src/api/
)

set(SOURCES
            ${CMAKE_SOURCE_DIR}/src/main.cpp
			${CMAKE_SOURCE_DIR}/src/handler/contextMenu.cpp
			${CMAKE_SOURCE_DIR}/src/handler/eventHandler.cpp
			${CMAKE_SOURCE_DIR}/src/handler/mainMenu.cpp
            ${CMAKE_SOURCE_DIR}/src/handler/fileHandler.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/listView.cpp
			${CMAKE_SOURCE_DIR}/src/ui/listViewEntry.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/webDAVView/webDAVView.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/webDAVView/webDAVViewEntry.cpp
			${CMAKE_SOURCE_DIR}/src/ui/loginView/loginView.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/fileView/fileView.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/fileView/fileViewEntry.cpp
            ${CMAKE_SOURCE_DIR}/src/ui/excludeFileView/excludeFileView.cpp
			${CMAKE_SOURCE_DIR}/src/util/util.cpp
			${CMAKE_SOURCE_DIR}/src/util/log.cpp
            ${CMAKE_SOURCE_DIR}/src/api/webDAV.cpp
            ${CMAKE_SOURCE_DIR}/src/api/sqliteConnector.cpp
            ${CMAKE_SOURCE_DIR}/src/api/fileBrowser.cpp
)

add_executable(Nextcloud.app ${SOURCES})

target_compile_options(Nextcloud.app PRIVATE -Wall -Wextra -Wpedantic -Werror -pedantic -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=5 -Wswitch-default -Wundef -Wno-unused)

TARGET_LINK_LIBRARIES (Nextcloud.app PRIVATE inkview freetype  sqlite3 stdc++fs curl)

target_compile_definitions(Nextcloud.app PRIVATE DBVERSION=2 PROGRAMVERSION="1.10")

INSTALL (TARGETS Nextcloud.app)

