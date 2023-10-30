#.rst:
# FindLibavformat
# -----------
#
# Find Libavformat
#
# Find Libavformat headers and libraries
#
# libavformat
#   LIBAVFORMAT_FOUND          - True if libavformat found.
#   LIBAVFORMAT_INCLUDE_DIRS   - Where to find libavformat.h.
#   LIBAVFORMAT_LIBRARIES      - List of libraries when using libavformat.
#   LIBAVFORMAT_VERSION_STRING - The version of libavformat found.
#
#

#=============================================================================
# Copyright 2018 Nicolas Mora <mail@babelouest.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation;
# version 2.1 of the License.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU GENERAL PUBLIC LICENSE for more details.
#
# You should have received a copy of the GNU General Public
# License along with this library.	If not, see <http://www.gnu.org/licenses/>.
#=============================================================================

find_package(PkgConfig QUIET)
pkg_check_modules(PC_LIBAVFORMAT QUIET libavformat)

find_path(LIBAVFORMAT_INCLUDE_DIR
        NAMES libavformat/avformat.h
        HINTS ${PC_LIBAVFORMAT_INCLUDEDIR} ${PC_LIBAVFORMAT_INCLUDE_DIRS})

find_library(LIBAVFORMAT_LIBRARY
        NAMES libavformat avformat
        HINTS ${PC_LIBAVFORMAT_LIBDIR} ${PC_LIBAVFORMAT_LIBRARY_DIRS})

set(LIBAVFORMAT_VERSION_MAJOR 0)
set(LIBAVFORMAT_VERSION_MINOR 0)
set(LIBAVFORMAT_VERSION_MICRO 0)
if (PC_LIBAVFORMAT_VERSION)
    set(LIBAVFORMAT_VERSION_STRING ${PC_LIBAVFORMAT_VERSION})
elseif (LIBAVFORMAT_INCLUDE_DIR AND EXISTS "${LIBAVFORMAT_INCLUDE_DIR}/libavformat/version.h")
    set(regex_libavformat_version_major "^#define[ \t]+LIBAVFORMAT_VERSION_MAJOR[ \t]+([^\"]+).*")
    file(STRINGS "${LIBAVFORMAT_INCLUDE_DIR}/libavformat/version.h" libavformat_version_major REGEX "${regex_libavformat_version_major}")
    string(REGEX REPLACE "${regex_libavformat_version_major}" "\\1" LIBAVFORMAT_VERSION_MAJOR "${libavformat_version_major}")
    unset(regex_libavformat_version_major)
    unset(libavformat_version_major)

    set(regex_libavformat_version_minor "^#define[ \t]+LIBAVFORMAT_VERSION_MINOR[ \t]+([^\"]+).*")
    file(STRINGS "${LIBAVFORMAT_INCLUDE_DIR}/libavformat/version.h" libavformat_version_minor REGEX "${regex_libavformat_version_minor}")
    string(REGEX REPLACE "${regex_libavformat_version_minor}" "\\1" LIBAVFORMAT_VERSION_MINOR "${libavformat_version_minor}")
    unset(regex_libavformat_version_minor)
    unset(libavformat_version_minor)

    set(regex_libavformat_version_micro "^#define[ \t]+LIBAVFORMAT_VERSION_MICRO[ \t]+([^\"]+).*")
    file(STRINGS "${LIBAVFORMAT_INCLUDE_DIR}/libavformat/version.h" libavformat_version_micro REGEX "${regex_libavformat_version_micro}")
    string(REGEX REPLACE "${regex_libavformat_version_micro}" "\\1" LIBAVFORMAT_VERSION_MICRO "${libavformat_version_micro}")
    unset(regex_libavformat_version_micro)
    unset(libavformat_version_micro)
		
		set (LIBAVFORMAT_VERSION_STRING "${LIBAVFORMAT_VERSION_MAJOR}.${LIBAVFORMAT_VERSION_MINOR}.${LIBAVFORMAT_VERSION_MICRO}")
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libavformat
        REQUIRED_VARS LIBAVFORMAT_LIBRARY LIBAVFORMAT_INCLUDE_DIR
        VERSION_VAR LIBAVFORMAT_VERSION_STRING)

if (LIBAVFORMAT_FOUND)
    set(LIBAVFORMAT_LIBRARIES ${LIBAVFORMAT_LIBRARY})
    set(LIBAVFORMAT_INCLUDE_DIRS ${LIBAVFORMAT_INCLUDE_DIR})
    if (NOT TARGET Libav::Avformat)
        add_library(Libav::Avformat UNKNOWN IMPORTED)
        set_target_properties(Libav::Avformat PROPERTIES
                IMPORTED_LOCATION "${LIBAVFORMAT_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${LIBAVFORMAT_INCLUDE_DIR}")
    endif ()
endif ()

mark_as_advanced(LIBAVFORMAT_INCLUDE_DIR LIBAVFORMAT_LIBRARY)
