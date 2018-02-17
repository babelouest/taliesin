#.rst:
# FindLibavresample
# -----------
#
# Find Libavresample
#
# Find Libavresample headers and libraries
#
# libavresample
#   LIBAVRESAMPLE_FOUND          - True if libavresample found.
#   LIBAVRESAMPLE_INCLUDE_DIRS   - Where to find libavresample.h.
#   LIBAVRESAMPLE_LIBRARIES      - List of libraries when using libavresample.
#   LIBAVRESAMPLE_VERSION_STRING - The version of libavresample found.
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
pkg_check_modules(PC_LIBAVRESAMPLE QUIET Libavresample)

find_path(LIBAVRESAMPLE_INCLUDE_DIR
        NAMES libavresample/avresample.h
        HINTS ${PC_LIBAVRESAMPLE_INCLUDEDIR} ${PC_LIBAVRESAMPLE_INCLUDE_DIRS})

find_library(LIBAVRESAMPLE_LIBRARY
        NAMES libavresample avresample
        HINTS ${PC_LIBAVRESAMPLE_LIBDIR} ${PC_LIBAVRESAMPLE_LIBRARY_DIRS})

set(LIBAVRESAMPLE_VERSION_MAJOR 0)
set(LIBAVRESAMPLE_VERSION_MINOR 0)
set(LIBAVRESAMPLE_VERSION_MICRO 0)
if (PC_LIBAVRESAMPLE_VERSION)
    set(LIBAVRESAMPLE_VERSION_STRING ${PC_LIBAVRESAMPLE_VERSION})
elseif (LIBAVRESAMPLE_INCLUDE_DIR AND EXISTS "${LIBAVRESAMPLE_INCLUDE_DIR}/libavresample/version.h")
    set(regex_libavresample_version_major "^#define[ \t]+LIBAVRESAMPLE_VERSION_MAJOR[ \t]+([^\"]+).*")
    file(STRINGS "${LIBAVRESAMPLE_INCLUDE_DIR}/libavresample/version.h" libavresample_version_major REGEX "${regex_libavresample_version_major}")
    string(REGEX REPLACE "${regex_libavresample_version_major}" "\\1" LIBAVRESAMPLE_VERSION_MAJOR "${libavresample_version_major}")
    unset(regex_libavresample_version_major)
    unset(libavresample_version_major)

    set(regex_libavresample_version_minor "^#define[ \t]+LIBAVRESAMPLE_VERSION_MINOR[ \t]+([^\"]+).*")
    file(STRINGS "${LIBAVRESAMPLE_INCLUDE_DIR}/libavresample/version.h" libavresample_version_minor REGEX "${regex_libavresample_version_minor}")
    string(REGEX REPLACE "${regex_libavresample_version_minor}" "\\1" LIBAVRESAMPLE_VERSION_MINOR "${libavresample_version_minor}")
    unset(regex_libavresample_version_minor)
    unset(libavresample_version_minor)

    set(regex_libavresample_version_micro "^#define[ \t]+LIBAVRESAMPLE_VERSION_MICRO[ \t]+([^\"]+).*")
    file(STRINGS "${LIBAVRESAMPLE_INCLUDE_DIR}/libavresample/version.h" libavresample_version_micro REGEX "${regex_libavresample_version_micro}")
    string(REGEX REPLACE "${regex_libavresample_version_micro}" "\\1" LIBAVRESAMPLE_VERSION_MICRO "${libavresample_version_micro}")
    unset(regex_libavresample_version_micro)
    unset(libavresample_version_micro)
		
		set (LIBAVRESAMPLE_VERSION_STRING "${LIBAVRESAMPLE_VERSION_MAJOR}.${LIBAVRESAMPLE_VERSION_MINOR}.${LIBAVRESAMPLE_VERSION_MICRO}")
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libavresample
        REQUIRED_VARS LIBAVRESAMPLE_LIBRARY LIBAVRESAMPLE_INCLUDE_DIR
        VERSION_VAR LIBAVRESAMPLE_VERSION_STRING)

if (LIBAVRESAMPLE_FOUND)
    set(LIBAVRESAMPLE_LIBRARIES ${LIBAVRESAMPLE_LIBRARY})
    set(LIBAVRESAMPLE_INCLUDE_DIRS ${LIBAVRESAMPLE_INCLUDE_DIR})
endif ()

mark_as_advanced(LIBAVRESAMPLE_INCLUDE_DIR LIBAVRESAMPLE_LIBRARY)
