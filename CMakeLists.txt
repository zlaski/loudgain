# loudgain CMakeLists.txt
# Copyright (C) 2014 Alessandro Ghedini <alessandro@ghedini.me>
# Modifications Copyright (C) 2019 Matthias C. Hormann <mhormann@gmx.de>
# Modifications Copyright (C) 2022 Ziemowit Laski <zlaski@ziemas.net>
# This file is released under the 2 clause BSD license, see COPYING

CMAKE_MINIMUM_REQUIRED(VERSION 3.23)
PROJECT(loudgain C CXX)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/" ${CMAKE_MODULE_PATH})

SET(VERSION_MAJOR 0)
SET(VERSION_MINOR 6)
SET(VERSION_PATCH 8)

SET(loudgain_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

SET(INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

INCLUDE(FindPkgConfig)

IF (APPLE)
  # Fix linking on 10.14+. See https://stackoverflow.com/questions/54068035
  LINK_DIRECTORIES(/usr/local/lib)
ENDIF (APPLE)

FIND_PACKAGE(EBUR128)
IF (NOT EBUR128_FOUND)
  MESSAGE(FATAL_ERROR "libebur128 not found.")
ENDIF (NOT EBUR128_FOUND)

FIND_PACKAGE(Taglib)
IF (NOT Taglib_FOUND)
  MESSAGE(FATAL_ERROR "Taglib not found.")
ENDIF (NOT Taglib_FOUND)

set(FFmpeg_FIND_COMPONENTS AVCODEC AVFORMAT AVUTIL SWRESAMPLE)
FIND_PACKAGE(FFmpeg)
IF (NOT FFmpeg_FOUND)
  MESSAGE(FATAL_ERROR "FFmpeg not found.")
ENDIF (NOT FFmpeg_FOUND)

FILE(GLOB SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "src/*.cc" "src/*.c")

ADD_EXECUTABLE(loudgain ${SOURCES})

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${EBUR128_INCLUDE_DIR}
  ${TAGLIB_INCLUDE_DIR}
  ${AVCODEC_INCLUDE_DIRS}
  ${AVFORMAT_INCLUDE_DIRS}
  ${SWRESAMPLE_INCLUDE_DIRS}
  ${AVUTIL_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
)

# Mac OS X has no pty.h
INCLUDE (CheckIncludeFiles)
CHECK_INCLUDE_FILES (pty.h HAVE_PTY_H)

CONFIGURE_FILE("config.h.in" "config.h")

SET(LIBS )

TARGET_LINK_LIBRARIES(loudgain
  ${EBUR128_LIBRARY}
  ${TAGLIB_LIBRARIES}
  ${AVCODEC_LIBRARIES}
  ${AVFORMAT_LIBRARIES}
  ${SWRESAMPLE_LIBRARIES}
  ${AVUTIL_LIBRARIES}
)

SET_TARGET_PROPERTIES(loudgain PROPERTIES
  COMPILE_FLAGS "-Wall -pedantic -g"
)

SET(CMAKE_C_FLAGS "-std=gnu99 -D_GNU_SOURCE")

SET(CMAKE_CXX_FLAGS "-std=gnu++11 -D_GNU_SOURCE")

INSTALL(TARGETS loudgain DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

INSTALL(FILES
  ${PROJECT_SOURCE_DIR}/docs/loudgain.1
  DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1
)
INSTALL(PROGRAMS
  ${PROJECT_SOURCE_DIR}/bin/rgbpm
  DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

# provide 'uninstall' target
if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()
