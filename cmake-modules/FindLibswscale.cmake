#.rst:
# FindLibswscale
# -----------
#
# Find libswscale
#
# Find libswscale headers and libraries
#
# libswscale
#   LIBSWSCALE_FOUND          - True if libswscale found.
#   LIBSWSCALE_INCLUDE_DIRS   - Where to find libswscale.h.
#   LIBSWSCALE_LIBRARIES      - List of libraries when using libswscale.
#   LIBSWSCALE_VERSION_STRING - The version of libswscale found.
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
pkg_check_modules(PC_LIBSWSCALE QUIET Libswscale)

find_path(LIBSWSCALE_INCLUDE_DIR
        NAMES libswscale/swscale.h
        HINTS ${PC_LIBSWSCALE_INCLUDEDIR} ${PC_LIBSWSCALE_INCLUDE_DIRS})

find_library(LIBSWSCALE_LIBRARY
        NAMES libswscale swscale
        HINTS ${PC_LIBSWSCALE_LIBDIR} ${PC_LIBSWSCALE_LIBRARY_DIRS})

set(LIBSWSCALE_VERSION_MAJOR 0)
set(LIBSWSCALE_VERSION_MINOR 0)
set(LIBSWSCALE_VERSION_MICRO 0)
if (PC_LIBSWSCALE_VERSION)
    set(LIBSWSCALE_VERSION_STRING ${PC_LIBSWSCALE_VERSION})
elseif (LIBSWSCALE_INCLUDE_DIR AND EXISTS "${LIBSWSCALE_INCLUDE_DIR}/libswscale/version.h")
    set(regex_libswscale_version_major "^#define[ \t]+LIBSWSCALE_VERSION_MAJOR[ \t]+([^\"]+).*")
    file(STRINGS "${LIBSWSCALE_INCLUDE_DIR}/libswscale/version.h" libswscale_version_major REGEX "${regex_libswscale_version_major}")
    string(REGEX REPLACE "${regex_libswscale_version_major}" "\\1" LIBSWSCALE_VERSION_MAJOR "${libswscale_version_major}")
    unset(regex_libswscale_version_major)
    unset(libswscale_version_major)

    set(regex_libswscale_version_minor "^#define[ \t]+LIBSWSCALE_VERSION_MINOR[ \t]+([^\"]+).*")
    file(STRINGS "${LIBSWSCALE_INCLUDE_DIR}/libswscale/version.h" libswscale_version_minor REGEX "${regex_libswscale_version_minor}")
    string(REGEX REPLACE "${regex_libswscale_version_minor}" "\\1" LIBSWSCALE_VERSION_MINOR "${libswscale_version_minor}")
    unset(regex_libswscale_version_minor)
    unset(libswscale_version_minor)

    set(regex_libswscale_version_micro "^#define[ \t]+LIBSWSCALE_VERSION_MICRO[ \t]+([^\"]+).*")
    file(STRINGS "${LIBSWSCALE_INCLUDE_DIR}/libswscale/version.h" libswscale_version_micro REGEX "${regex_libswscale_version_micro}")
    string(REGEX REPLACE "${regex_libswscale_version_micro}" "\\1" LIBSWSCALE_VERSION_MICRO "${libswscale_version_micro}")
    unset(regex_libswscale_version_micro)
    unset(libswscale_version_micro)
		
		set (LIBSWSCALE_VERSION_STRING "${LIBSWSCALE_VERSION_MAJOR}.${LIBSWSCALE_VERSION_MINOR}.${LIBSWSCALE_VERSION_MICRO}")
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libswscale
        REQUIRED_VARS LIBSWSCALE_LIBRARY LIBSWSCALE_INCLUDE_DIR
        VERSION_VAR LIBSWSCALE_VERSION_STRING)

if (LIBSWSCALE_FOUND)
    set(LIBSWSCALE_LIBRARIES ${LIBSWSCALE_LIBRARY})
    set(LIBSWSCALE_INCLUDE_DIRS ${LIBSWSCALE_INCLUDE_DIR})
endif ()

mark_as_advanced(LIBSWSCALE_INCLUDE_DIR LIBSWSCALE_LIBRARY)
