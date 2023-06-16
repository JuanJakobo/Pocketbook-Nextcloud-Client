CMAKE_MINIMUM_REQUIRED (VERSION 3.10.1)

PROJECT (Pocketbook-Nextcloud VERSION 1.10)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1.0)
set(CMAKE_SYSTEM_PROCESSOR armv7a)
set(BUILD_SHARED_LIBS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

SET (TOOLCHAIN_PATH "../../SDK/SDK_6.3.0/SDK-B288")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_FIND_ROOT_PATH "${TOOLCHAIN_PATH}/usr/arm-obreey-linux-gnueabi/sysroot/")
set(CMAKE_INCLUDE_PATH "/usr/include")
message("CMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
include_directories("${TOOLCHAIN_PATH}/usr/arm-obreey-linux-gnueabi/sysroot/usr/include/freetype2")
include_directories("${TOOLCHAIN_PATH}/usr/arm-obreey-linux-gnueabi/sysroot/usr/local/include/")
list(APPEND CMAKE_MODULE_PATH "${TOOLCHAIN_PATH}/usr/share/cmake/modules")
list(REMOVE_DUPLICATES CMAKE_MODULE_PATH)
set(QT_QMAKE_EXECUTABLE "${TOOLCHAIN_PATH}/usr/arm-obreey-linux-gnueabi/sysroot/usr/qt5/bin/qmake")
set(CMAKE_PREFIX_PATH "${TOOLCHAIN_PATH}/usr/arm-obreey-linux-gnueabi/sysroot/ebrmain/lib/cmake")

set(CMAKE_C_COMPILER "${TOOLCHAIN_PATH}/usr/bin/arm-obreey-linux-gnueabi-clang")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PATH}/usr/bin/arm-obreey-linux-gnueabi-clang++")
set(CMAKE_C_FLAGS "-fsigned-char -Werror-return-type" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "-fsigned-char -Werror-return-type" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O2 -pipe -fomit-frame-pointer -march=armv7-a -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp " CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG -O2 -pipe -fomit-frame-pointer -march=armv7-a -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp " CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_DEBUG "-DDEBUG -O0 -g -pipe -fomit-frame-pointer -march=armv7-a -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp " CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -O0 -g -pipe -fomit-frame-pointer -march=armv7-a -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp " CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-s" CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "-s" CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-z,defs" CACHE STRING "" FORCE)

add_definitions(-DPLATFORM_FC)
set(PB_PLATFORM "ARM" CACHE STRING "ARM|PC Readonly!")

set(CMAKE_INSTALL_PREFIX "${TOOLCHAIN_PATH}/usr/arm-obreey-linux-gnueabi/sysroot/usr/local" CACHE PATH "Installation Prefix")
set(CMAKE_BUILD_TYPE Release CACHE STRING "Debug|Release|RelWithDebInfo|MinSizeRel")
set(ENV{PKG_CONFIG_DIR} "")
set(ENV{PKG_CONFIG_LIBDIR} ${CMAKE_FIND_ROOT_PATH}/usr/lib/pkgconfig)
set(ENV{PKG_CONFIG_SYSROOT_DIR} ${CMAKE_FIND_ROOT_PATH})
set(ENV{LD_LIBRARY_PATH} ${TOOLCHAIN_PATH}/usr/lib)
list(APPEND PB_LINK_DIRECTORIES "${TOOLCHAIN_PATH}/usr/arm-obreey-linux-gnueabi/sysroot/usr/lib")
list(APPEND PB_LINK_DIRECTORIES "${TOOLCHAIN_PATH}/usr/arm-obreey-linux-gnueabi/sysroot/usr/local/lib")
list(APPEND PB_INCLUDE_DIRECTORIES "${TOOLCHAIN_PATH}/usr/arm-obreey-linux-gnueabi/sysroot/usr/include")

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

#SET (TARGET_INCLUDE "")

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

target_compile_options(Nextcloud.app PRIVATE -Wall -Wextra -Wpedantic -Werror -pedantic -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror -Wno-unused)


TARGET_LINK_LIBRARIES (Nextcloud.app PRIVATE inkview freetype curl sqlite3 stdc++fs)
target_compile_definitions(Nextcloud.app PRIVATE DBVERSION=2 PROGRAMVERSION="1.10")

INSTALL (TARGETS Nextcloud.app)

