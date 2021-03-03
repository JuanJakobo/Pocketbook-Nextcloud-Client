#
# This file has been adapted from the PocketBook SDK 
#

PROJECT (Pocketbook-Nextcloud-Client)
CMAKE_MINIMUM_REQUIRED (VERSION 3.10.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET (TOOLCHAIN_PATH "../../SDK/pocketbook-sdk5-master")
SET (TOOLCHAIN_PREFIX "arm-obreey-linux-gnueabi")
SET (TOOLCHAIN_INSTALL "sysroot/usr")
ADD_DEFINITIONS(-DPLATFORM_FC)

IF (NOT CMAKE_BUILD_TYPE)
	SET (CMAKE_BUILD_TYPE Debug)
ENDIF (NOT CMAKE_BUILD_TYPE)

#For target "ARM"

#../../SDK/pocketbook-sdk5-master/arm-obreey-linux-gnueabi/sysroot/usr
SET(CMAKE_INSTALL_PREFIX "${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}/${TOOLCHAIN_INSTALL}" CACHE PATH "Install path prefix" FORCE)

#../../SDK/pocketbook-sdk5-master/bin/arm-obreey-linux-gnueabi-gcc
SET (CMAKE_C_COMPILER ${CMAKE_CURRENT_SOURCE_DIR}/${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-gcc)

#../../SDK/pocketbook-sdk5-master/bin/arm-obreey-linux-gnueabi-g++
SET (CMAKE_CXX_COMPILER ${CMAKE_CURRENT_SOURCE_DIR}/${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-g++)

#../../SDK/pocketbook-sdk5-master/bin/arm-obreey-linux-gnueabi--g++
SET (CMAKE_LINK ${CMAKE_CURRENT_SOURCE_DIR}/${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-g++)

#../../SDK/pocketbook-sdk5-master/bin/arm-obreey-linux-gnueabi--arr
SET (CMAKE_ARR ${CMAKE_CURRENT_SOURCE_DIR}/${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-arr)

#../../SDK/pocketbook-sdk5-master/bin/arm-obreey-linux-gnueabi-strip
SET (CMAKE_STRIP ${CMAKE_CURRENT_SOURCE_DIR}/${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}-strip)

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
		MESSAGE (STATUS "Build for ARM Debug")
		SET (CMAKE_C_FLAGS_DEBUG "-DDEBUG -fsigned-char -W -Wall -Wextra -Wno-format-y2k -fomit-frame-pointer -O0 -g3")
		SET (CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -fsigned-char -W -Wall -Wextra -Wno-format-y2k -fomit-frame-pointer -O0 -g3")
ELSE ()
		MESSAGE (STATUS "Build for ARM Release")
		SET (CMAKE_C_FLAGS_RELEASE "-DNDEBUG -s -fsigned-char -W -Wall -Wextra -O2 -Wno-format-y2k -fomit-frame-pointer")
		SET (CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -s -fsigned-char -W -Wall -Wextra -O2 -Wno-format-y2k -fomit-frame-pointer")
ENDIF (CMAKE_BUILD_TYPE STREQUAL "Debug")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(SOURCES ${CMAKE_SOURCE_DIR}/src/main.cpp 
			${CMAKE_SOURCE_DIR}/src/handler/eventHandler.cpp 		
			${CMAKE_SOURCE_DIR}/src/handler/menuHandler.cpp 
			${CMAKE_SOURCE_DIR}/src/util/nextcloud.cpp 
			${CMAKE_SOURCE_DIR}/src/util/item.cpp
			${CMAKE_SOURCE_DIR}/src/ui/listView.cpp 
			${CMAKE_SOURCE_DIR}/src/ui/listViewEntry.cpp 
			${CMAKE_SOURCE_DIR}/src/ui/loginView.cpp
			${CMAKE_SOURCE_DIR}/src/util/util.cpp 
			${CMAKE_SOURCE_DIR}/src/util/log.cpp 

)

add_executable(Nextcloud.app ${SOURCES})

#SET (TARGET_INCLUDE "")

include_directories(
	#$(TARGET_INCLUDE)
    ${CMAKE_SOURCE_DIR}/src/handler/
    ${CMAKE_SOURCE_DIR}/src/util/
    ${CMAKE_SOURCE_DIR}/src/ui/
)

SET (TARGET_LIB inkview freetype z curl)

TARGET_LINK_LIBRARIES (Nextcloud.app ${TARGET_LIB})

INSTALL (TARGETS Nextcloud.app)

