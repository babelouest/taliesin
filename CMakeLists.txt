#
# Taliesin
#
# CMake file used to build program
#
# Copyright 2018-2022 Nicolas Mora <mail@babelouest.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the MIT License
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#

cmake_minimum_required(VERSION 3.5)

project(taliesin C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

option(FILE_OFFSET_BITS_64 "Use compile flag _FILE_OFFSET_BITS=64" OFF)
if (FILE_OFFSET_BITS_64)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FILE_OFFSET_BITS=64")
endif ()

# library info

set(PROGRAM_VERSION_MAJOR "1")
set(PROGRAM_VERSION_MINOR "0")
set(PROGRAM_VERSION_PATCH "20")
set(PROJECT_DESCRIPTION "Lightweight audio streaming server")
set(PROJECT_BUGREPORT_PATH "https://github.com/babelouest/taliesin/issues")
set(ORCANIA_VERSION_REQUIRED "2.3.3")
set(YDER_VERSION_REQUIRED "1.4.19")
set(ULFIUS_VERSION_REQUIRED "2.7.14")
set(HOEL_VERSION_REQUIRED "1.4.27")
set(RHONABWY_VERSION_REQUIRED "1.1.12")
set(IDDAWC_VERSION_REQUIRED "1.1.8")

include(GNUInstallDirs)
include(CheckSymbolExists)

# cmake modules

set(T_CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)
list(APPEND CMAKE_MODULE_PATH "${T_CMAKE_MODULE_PATH}")

# check if _GNU_SOURCE is available

if (NOT _GNU_SOURCE)
    check_symbol_exists(__GNU_LIBRARY__ "features.h" _GNU_SOURCE)

    if (NOT _GNU_SOURCE)
        unset(_GNU_SOURCE CACHE)
        check_symbol_exists(_GNU_SOURCE "features.h" _GNU_SOURCE)
    endif ()
endif ()

if (_GNU_SOURCE)
    add_definitions(-D_GNU_SOURCE)
endif ()

include(FindJansson)
set(JANSSON_MIN_VERSION 2.4)
find_package(Jansson ${JANSSON_MIN_VERSION} REQUIRED)
if (JANSSON_FOUND)
    set(LIBS ${JANSSON_LIBRARIES})
endif ()

include(FindLibconfig)
find_package(Libconfig REQUIRED)
if (LIBCONFIG_FOUND)
    set(LIBS ${LIBS} ${LIBCONFIG_LIBRARIES})
    include_directories(${LIBCONFIG_INCLUDE_DIRS})
endif ()

include(FindLibavcodec)
set(LIBAVCODEC_MIN_VERSION 56.0.0)
find_package(Libavcodec ${LIBAVCODEC_MIN_VERSION} REQUIRED)
if (LIBAVCODEC_FOUND)
    set(LIBS ${LIBS} ${LIBAVCODEC_LIBRARIES})
endif ()

include(FindLibavutil)
set(LIBAVUTIL_MIN_VERSION 55.0.0)
find_package(Libavutil ${LIBAVUTIL_MIN_VERSION} REQUIRED)
if (LIBAVUTIL_FOUND)
    set(LIBS ${LIBS} ${LIBAVUTIL_LIBRARIES})
endif ()

include(FindLibswresample)
set(LIBSWRESAMPLE_MIN_VERSION 2.0.0)
find_package(Libswresample ${LIBSWRESAMPLE_MIN_VERSION} REQUIRED)
if (LIBSWRESAMPLE_FOUND)
    set(LIBS ${LIBS} ${LIBSWRESAMPLE_LIBRARIES})
endif ()

include(FindLibavformat)
set(LIBAVFORMAT_MIN_VERSION 54.0.0)
find_package(Libavformat ${LIBAVFORMAT_MIN_VERSION} REQUIRED)
if (LIBAVFORMAT_FOUND)
    set(LIBS ${LIBS} ${LIBAVFORMAT_LIBRARIES})
endif ()

include(FindLibswscale)
set(LIBSWSCALE_MIN_VERSION 4.0.0)
find_package(Libswscale ${LIBSWSCALE_MIN_VERSION} REQUIRED)
if (LIBSWSCALE_FOUND)
    set(LIBS ${LIBS} ${LIBSWSCALE_LIBRARIES})
endif ()

include(FindLibshout)
find_package(Libshout REQUIRED)
if (LIBSHOUT_FOUND)
    set(LIBS ${LIBS} ${LIBSHOUT_LIBRARIES})
endif ()

include(FindZLIB)
find_package(ZLIB REQUIRED)
set(LIBS ${LIBS} ${ZLIB_LIBRARIES})

# build

if (DISABLE_OAUTH2)
    add_executable(taliesin ${CMAKE_CURRENT_SOURCE_DIR}/src/taliesin.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/static_compressed_inmemory_website_callback.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/http_compression_callback.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/data_source.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/file_list.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/filesystem.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/libav-transcode-filter.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/media.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/playlist.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/search.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/static_compressed_inmemory_website_callback.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/http_compression_callback.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/stream.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/stream_jukebox.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/stream_webradio.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/webservice.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/taliesin.c )
else ()
    add_executable(taliesin ${CMAKE_CURRENT_SOURCE_DIR}/src/taliesin.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/iddawc_resource.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/static_compressed_inmemory_website_callback.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/http_compression_callback.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/data_source.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/file_list.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/filesystem.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/libav-transcode-filter.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/iddawc_resource.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/media.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/playlist.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/search.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/static_compressed_inmemory_website_callback.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/http_compression_callback.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/stream.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/stream_jukebox.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/stream_webradio.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/webservice.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/taliesin.c )
endif ()
set_target_properties(taliesin PROPERTIES COMPILE_OPTIONS "-Wextra;-Wconversion")

include(FindOrcania)
find_package(Orcania ${ORCANIA_VERSION_REQUIRED} REQUIRED)
set(LIBS ${LIBS} ${ORCANIA_LIBRARIES})
include_directories(${ORCANIA_INCLUDE_DIRS})

include(FindYder)
find_package(Yder ${YDER_VERSION_REQUIRED} REQUIRED)
set(LIBS ${LIBS} ${YDER_LIBRARIES})
include_directories(${YDER_INCLUDE_DIRS})

include(FindUlfius)
find_package(Ulfius ${ULFIUS_VERSION_REQUIRED} REQUIRED)
set(LIBS ${LIBS} ${ULFIUS_LIBRARIES})
include_directories(${ULFIUS_INCLUDE_DIRS})

include(FindHoel)
find_package(Hoel ${HOEL_VERSION_REQUIRED} REQUIRED)
set(LIBS ${LIBS} ${HOEL_LIBRARIES})
include_directories(${HOEL_INCLUDE_DIRS})

if (DISABLE_OAUTH2)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDISABLE_OAUTH2")
else ()
    include(FindIddawc)
    find_package(Iddawc ${IDDAWC_VERSION_REQUIRED} REQUIRED)
    set(LIBS ${LIBS} ${IDDAWC_LIBRARIES})
    include_directories(${IDDAWC_INCLUDE_DIRS})

    include(FindRhonabwy)
    find_package(Rhonabwy ${RHONABWY_VERSION_REQUIRED} REQUIRED)
    set(LIBS ${LIBS} ${RHONABWY_LIBRARIES})
    include_directories(${RHONABWY_INCLUDE_DIRS})
endif ()

target_link_libraries(taliesin ${LIBS} pthread)

# tests

option(BUILD_TESTING "Build the testing tree." OFF)

if (BUILD_TESTING)
    include(FindCheck)
    find_package(Check REQUIRED)
    if (CHECK_FOUND)
        include(FindSubunit)
        find_package(Subunit REQUIRED)

        enable_testing()

        set(CMAKE_CTEST_COMMAND ctest -V)

        set(TST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)
        set(LIBS ${LIBS} ${CHECK_LIBRARIES} ulfius yder orcania)
        find_package(Threads REQUIRED)
        set(LIBS ${LIBS} ${SUBUNIT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} m rt)

				set(TESTS config data_source_crud data_source_refresh data_source_create_environment data_source_browse_path data_source_browse_category playlist_crud search_simple search_advanced stream_webradio stream_jukebox data_source_delete_environment )

        configure_file(
                "${T_CMAKE_MODULE_PATH}/CTestCustom.cmake.in"
                "${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake"
                @ONLY)

        foreach (t ${TESTS})
            add_executable(${t} EXCLUDE_FROM_ALL ${TST_DIR}/${t}.c ${TST_DIR}/unit-tests.c ${TST_DIR}/unit-tests.h)
            target_include_directories(${t} PUBLIC ${TST_DIR})
            target_link_libraries(${t} PUBLIC ${LIBS})
            add_test(NAME ${t}
                    WORKING_DIRECTORY ${TST_DIR}
                    COMMAND ${t})
        endforeach ()
    endif ()
endif ()

# install target

install(TARGETS taliesin
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES docs/taliesin.conf.sample 
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/taliesin RENAME taliesin.conf COMPONENT config)
install(DIRECTORY webapp/
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/taliesin/webapp/ COMPONENT runtime)
install(FILES webapp-src/config.json.sample 
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/taliesin/webapp/ RENAME config.json COMPONENT config)
install(FILES README.md
        DESTINATION ${CMAKE_INSTALL_DOCDIR}/ COMPONENT runtime)
install(FILES docs/INSTALL.md
        DESTINATION ${CMAKE_INSTALL_DOCDIR}/ COMPONENT runtime)
install(FILES docs/API.md
        DESTINATION ${CMAKE_INSTALL_DOCDIR}/ COMPONENT runtime)
install(FILES docs/taliesin-init
        DESTINATION ${CMAKE_INSTALL_DOCDIR}/ COMPONENT runtime)
install(FILES docs/taliesin.service
        DESTINATION ${CMAKE_INSTALL_DOCDIR}/ COMPONENT runtime)
install(FILES docs/taliesin.mariadb.sql
        DESTINATION ${CMAKE_INSTALL_DOCDIR}/ COMPONENT runtime)
install(FILES docs/taliesin.sqlite3.sql
        DESTINATION ${CMAKE_INSTALL_DOCDIR}/ COMPONENT runtime)

set(TARGETS taliesin)

install(TARGETS ${TARGETS}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        CONFIGURATIONS Release)

# uninstall target

if (NOT TARGET uninstall)
    configure_file(
            "${T_CMAKE_MODULE_PATH}/CMakeUninstall.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
            IMMEDIATE @ONLY)
    add_custom_target(uninstall
            COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif ()

# packaging

set(CPACK_PACKAGE_VERSION_MAJOR ${PROGRAM_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROGRAM_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROGRAM_VERSION_PATCH})

set(PACKAGE_FILE_NAME
        "${CMAKE_PROJECT_NAME}_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

set(PACKAGE_IGNORED_FILES
        "${CMAKE_CURRENT_BINARY_DIR}/;/.git/;.gitignore;~$;${CPACK_SOURCE_IGNORE_FILES}")

set(CPACK_GENERATOR "TGZ;DEB")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "mail@babelouest.org")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION ${PROJECT_DESCRIPTION})
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/babelouest/taliesin")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.3.4), libconfig9, libjansson4 (>= 2.7), liborcania (>= 2.1), libyder (>= 1.4), libulfius (>= 2.7), libhoel (>= 1.4), lsb-base (>= 3.0-6)")
set(CPACK_PACKAGE_FILE_NAME ${PACKAGE_FILE_NAME})

set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${PACKAGE_FILE_NAME})
set(CPACK_SOURCE_IGNORE_FILES ${PACKAGE_IGNORED_FILES})

include(CPack)

add_custom_target(dist
        COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
