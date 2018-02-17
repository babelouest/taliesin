#.rst:
# FindLibavcodec
# -----------
#
# Find libavcodec
#
# Find libavcodec headers and libraries
#
# libavcodec
#   LIBAVCODEC_FOUND          - True if libavcodec found.
#   LIBAVCODEC_INCLUDE_DIRS   - Where to find libavcodec.h.
#   LIBAVCODEC_LIBRARIES      - List of libraries when using libavcodec.
#   LIBAVCODEC_VERSION_STRING - The version of libavcodec found.
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
pkg_check_modules(PC_LIBAVCODEC QUIET Libavcodec)

find_path(LIBAVCODEC_INCLUDE_DIR
        NAMES libavcodec/avcodec.h
        HINTS ${PC_LIBAVCODEC_INCLUDEDIR} ${PC_LIBAVCODEC_INCLUDE_DIRS})

find_library(LIBAVCODEC_LIBRARY
        NAMES libavcodec avcodec
        HINTS ${PC_LIBAVCODEC_LIBDIR} ${PC_LIBAVCODEC_LIBRARY_DIRS})

set(LIBAVCODEC_VERSION_MAJOR 0)
set(LIBAVCODEC_VERSION_MINOR 0)
set(LIBAVCODEC_VERSION_MICRO 0)
if (PC_LIBAVCODEC_VERSION)
    set(LIBAVCODEC_VERSION_STRING ${PC_LIBAVCODEC_VERSION})
elseif (LIBAVCODEC_INCLUDE_DIR AND EXISTS "${LIBAVCODEC_INCLUDE_DIR}/libavcodec/version.h")
    set(regex_libavcodec_version_major "^#define[ \t]+LIBAVCODEC_VERSION_MAJOR[ \t]+([^\"]+).*")
    file(STRINGS "${LIBAVCODEC_INCLUDE_DIR}/libavcodec/version.h" libavcodec_version_major REGEX "${regex_libavcodec_version_major}")
    string(REGEX REPLACE "${regex_libavcodec_version_major}" "\\1" LIBAVCODEC_VERSION_MAJOR "${libavcodec_version_major}")
    unset(regex_libavcodec_version_major)
    unset(libavcodec_version_major)

    set(regex_libavcodec_version_minor "^#define[ \t]+LIBAVCODEC_VERSION_MINOR[ \t]+([^\"]+).*")
    file(STRINGS "${LIBAVCODEC_INCLUDE_DIR}/libavcodec/version.h" libavcodec_version_minor REGEX "${regex_libavcodec_version_minor}")
    string(REGEX REPLACE "${regex_libavcodec_version_minor}" "\\1" LIBAVCODEC_VERSION_MINOR "${libavcodec_version_minor}")
    unset(regex_libavcodec_version_minor)
    unset(libavcodec_version_minor)

    set(regex_libavcodec_version_micro "^#define[ \t]+LIBAVCODEC_VERSION_MICRO[ \t]+([^\"]+).*")
    file(STRINGS "${LIBAVCODEC_INCLUDE_DIR}/libavcodec/version.h" libavcodec_version_micro REGEX "${regex_libavcodec_version_micro}")
    string(REGEX REPLACE "${regex_libavcodec_version_micro}" "\\1" LIBAVCODEC_VERSION_MICRO "${libavcodec_version_micro}")
    unset(regex_libavcodec_version_micro)
    unset(libavcodec_version_micro)
		
		set (LIBAVCODEC_VERSION_STRING "${LIBAVCODEC_VERSION_MAJOR}.${LIBAVCODEC_VERSION_MINOR}.${LIBAVCODEC_VERSION_MICRO}")
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libavcodec
        REQUIRED_VARS LIBAVCODEC_LIBRARY LIBAVCODEC_INCLUDE_DIR
        VERSION_VAR LIBAVCODEC_VERSION_STRING)

if (LIBAVCODEC_FOUND)
    set(LIBAVCODEC_LIBRARIES ${LIBAVCODEC_LIBRARY})
    set(LIBAVCODEC_INCLUDE_DIRS ${LIBAVCODEC_INCLUDE_DIR})
endif ()

mark_as_advanced(LIBAVCODEC_INCLUDE_DIR LIBAVCODEC_LIBRARY)
