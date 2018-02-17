#
# Taliesin
#
# CMake file used to build program
#
# Copyright 2018 Nicolas Mora <mail@babelouest.org>
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

# library info

set(PROGRAM_VERSION_MAJOR "1")
set(PROGRAM_VERSION_MINOR "0")
set(PROGRAM_VERSION_PATCH "12")
set(PROJECT_DESCRIPTION "Lightweight audio streaming server")
set(PROJECT_BUGREPORT_PATH "https://github.com/babelouest/taliesin/issues")
set(ORCANIA_VERSION_DOWNLOAD "1.2.0")
set(YDER_VERSION_DOWNLOAD "1.2.0")
set(ULFIUS_VERSION_DOWNLOAD "2.3.0")
set(HOEL_VERSION_DOWNLOAD "1.4.0")

include(GNUInstallDirs)
include(CheckSymbolExists)

# cmake modules

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)

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

include(FindLibJWT)
find_package(LibJWT REQUIRED)
if (LIBJWT_FOUND)
    set(LIBS ${LIBS} ${LIBJWT_LIBRARIES})
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

include(FindLibavresample)
set(LIBAVRESAMPLE_MIN_VERSION 56.0.0)
find_package(Libavresample ${LIBARESAMPLE_MIN_VERSION} REQUIRED)
if (LIBAVRESAMPLE_FOUND)
    set(LIBS ${LIBS} ${LIBAVRESAMPLE_LIBRARIES})
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

include(FindZLIB)
find_package(ZLIB REQUIRED)
set(LIBS ${LIBS} ${ZLIB_LIBRARIES})

# build

add_executable(taliesin ${CMAKE_CURRENT_SOURCE_DIR}/src/taliesin.h
                ${CMAKE_CURRENT_SOURCE_DIR}/src/glewlwyd_resource.h
                ${CMAKE_CURRENT_SOURCE_DIR}/src/static_file_callback.h
                ${CMAKE_CURRENT_SOURCE_DIR}/src/config.c
                ${CMAKE_CURRENT_SOURCE_DIR}/src/data_source.c
                ${CMAKE_CURRENT_SOURCE_DIR}/src/file_list.c
                ${CMAKE_CURRENT_SOURCE_DIR}/src/filesystem.c
                ${CMAKE_CURRENT_SOURCE_DIR}/src/glewlwyd_resource.c
                ${CMAKE_CURRENT_SOURCE_DIR}/src/libav-transcode-filter.c
                ${CMAKE_CURRENT_SOURCE_DIR}/src/media.c
                ${CMAKE_CURRENT_SOURCE_DIR}/src/misc.c
                ${CMAKE_CURRENT_SOURCE_DIR}/src/playlist.c
                ${CMAKE_CURRENT_SOURCE_DIR}/src/search.c
                ${CMAKE_CURRENT_SOURCE_DIR}/src/static_file_callback.c
                ${CMAKE_CURRENT_SOURCE_DIR}/src/stream.c
                ${CMAKE_CURRENT_SOURCE_DIR}/src/stream_jukebox.c
                ${CMAKE_CURRENT_SOURCE_DIR}/src/stream_webradio.c
                ${CMAKE_CURRENT_SOURCE_DIR}/src/webservice.c
                ${CMAKE_CURRENT_SOURCE_DIR}/src/taliesin.c )
target_link_libraries(taliesin ${LIBS} pthread)

set(Orcania_FIND_QUIETLY ON) # force to find Orcania quietly
include(FindOrcania)
find_package(Orcania 1.1 QUIET) # try to find orcania
if (NOT ORCANIA_FOUND)
    include(DownloadProject)
    download_project(PROJ orcania # ... otherwise, download archive
            URL "https://github.com/babelouest/orcania/archive/v${ORCANIA_VERSION_DOWNLOAD}.tar.gz"
            QUIET)
    add_subdirectory(${orcania_SOURCE_DIR} ${orcania_BINARY_DIR})
    include_directories(${orcania_SOURCE_DIR}/include)
    add_dependencies(taliesin orcania)
		set(ORCANIA_LIBRARIES orcania)
endif ()
set(LIBS ${LIBS} ${ORCANIA_LIBRARIES})

set(Yder_FIND_QUIETLY ON) # force to find Yder quietly
include(FindYder)
find_package(Yder 1.1 QUIET) # try to find Yder
if (NOT YDER_FOUND)
    include(DownloadProject)
    option(CHECK_ORCANIA "specific param" off)
    download_project(PROJ yder # ... otherwise, download archive
            URL "https://github.com/babelouest/yder/archive/v${YDER_VERSION_DOWNLOAD}.tar.gz"
            QUIET)
    add_subdirectory(${yder_SOURCE_DIR} ${yder_BINARY_DIR})
    include_directories(${yder_SOURCE_DIR}/include)
    include_directories(${orcania_SOURCE_DIR}/include)
    add_dependencies(taliesin yder)
		set(YDER_LIBRARIES yder)
endif ()
set(LIBS ${LIBS} ${YDER_LIBRARIES})

set(Ulfius_FIND_QUIETLY ON)
include(FindUlfius)
find_package(Ulfius 2.2 QUIET)
if (NOT ULFIUS_FOUND)
    include(DownloadProject)
    option(WITH_WEBSOCKET "specific param" off)
    option(INSTALL_HEADER "specific param" off)
    option(CHECK_YDER "specific param" off)
    download_project(PROJ ulfius
            URL "https://github.com/babelouest/ulfius/archive/v${ULFIUS_VERSION_DOWNLOAD}.tar.gz"
            QUIET)
    add_subdirectory(${ulfius_SOURCE_DIR} ${ulfius_BINARY_DIR})
    include_directories(${yder_SOURCE_DIR}/include)
    include_directories(${orcania_SOURCE_DIR}/include)
    include_directories(${ulfius_SOURCE_DIR}/include)
    add_dependencies(taliesin ulfius)
		set(ULFIUS_LIBRARIES ulfius)
endif ()
set(LIBS ${LIBS} ${ULFIUS_LIBRARIES})

set(Hoel_FIND_QUIETLY ON)
include(FindHoel)
find_package(Hoel 1.3 QUIET)
if (NOT HOEL_FOUND)
    include(DownloadProject)
    option(WITH_PGSQL "specific param" off)
    option(INSTALL_HEADER "specific param" off)
    option(CHECK_YDER "specific param" off)
    download_project(PROJ hoel
            URL "https://github.com/babelouest/hoel/archive/v${HOEL_VERSION_DOWNLOAD}.tar.gz"
            QUIET)
    add_subdirectory(${hoel_SOURCE_DIR} ${hoel_BINARY_DIR})
    include_directories(${hoel_SOURCE_DIR}/include)
    add_dependencies(taliesin hoel)
		set(HOEL_LIBRARIES hoel)
endif ()
set(LIBS ${LIBS} ${HOEL_LIBRARIES})

target_link_libraries(taliesin ${LIBS})

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
                "${CMAKE_MODULE_PATH}/CTestCustom.cmake.in"
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
install(FILES taliesin.conf.sample 
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/taliesin RENAME taliesin.conf COMPONENT config)
install(DIRECTORY webapp/
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/taliesin/webapp/ COMPONENT runtime)
install(FILES webapp-src/public/config.json.sample 
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/taliesin/webapp/ RENAME config.json COMPONENT config)

set(TARGETS taliesin)

install(TARGETS ${TARGETS}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        CONFIGURATIONS Release)

# uninstall target

if (NOT TARGET uninstall)
    configure_file(
            "${CMAKE_MODULE_PATH}/CMakeUninstall.cmake.in"
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

set(CPACK_GENERATOR "TGZ;DEB;RPM")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "mail@babelouest.org")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION ${PROJECT_DESCRIPTION})
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/babelouest/taliesin")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.3.4), libconfig9, libjansson4 (>= 2.7), liborcania (>= 1.2.0), libyder (>= 1.2.0), libulfius (>= 2.3.0), libhoel (>= 1.4.0), lsb-base (>= 3.0-6)")
set(CPACK_PACKAGE_FILE_NAME ${PACKAGE_FILE_NAME})

set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${PACKAGE_FILE_NAME})
set(CPACK_SOURCE_IGNORE_FILES ${PACKAGE_IGNORED_FILES})

include(CPack)

add_custom_target(dist
        COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
