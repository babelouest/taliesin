#.rst:
# FindLibshout
# -----------
#
# Find libshout
#
# Find libshout headers and libraries
#
# libshout
#   LIBSHOUT_FOUND          - True if libshout found.
#   LIBSHOUT_INCLUDE_DIRS   - Where to find libshout.h.
#   LIBSHOUT_LIBRARIES      - List of libraries when using libshout.
#   LIBSHOUT_VERSION_STRING - The version of libshout found.
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
pkg_check_modules(PC_LIBSHOUT QUIET libshout)

find_path(LIBSHOUT_INCLUDE_DIR
        NAMES shout/shout.h
        HINTS ${PC_LIBSHOUT_INCLUDEDIR} ${PC_LIBSHOUT_INCLUDE_DIRS})

find_library(LIBSHOUT_LIBRARY
        NAMES libshout shout
        HINTS ${PC_LIBSHOUT_LIBDIR} ${PC_LIBSHOUT_LIBRARY_DIRS})

set(LIBSHOUT_VERSION_MAJOR 0)
set(LIBSHOUT_VERSION_MINOR 0)
set(LIBSHOUT_VERSION_MICRO 0)
if (PC_LIBSHOUT_VERSION)
    set(LIBSHOUT_VERSION_STRING ${PC_LIBSHOUT_VERSION})
elseif (LIBSHOUT_INCLUDE_DIR AND EXISTS "${LIBSHOUT_INCLUDE_DIR}/libshout/version.h")
    set(regex_libshout_version_major "^#define[ \t]+LIBSHOUT_VERSION_MAJOR[ \t]+([^\"]+).*")
    file(STRINGS "${LIBSHOUT_INCLUDE_DIR}/libshout/version.h" libshout_version_major REGEX "${regex_libshout_version_major}")
    string(REGEX REPLACE "${regex_libshout_version_major}" "\\1" LIBSHOUT_VERSION_MAJOR "${libshout_version_major}")
    unset(regex_libshout_version_major)
    unset(libshout_version_major)

    set(regex_libshout_version_minor "^#define[ \t]+LIBSHOUT_VERSION_MINOR[ \t]+([^\"]+).*")
    file(STRINGS "${LIBSHOUT_INCLUDE_DIR}/libshout/version.h" libshout_version_minor REGEX "${regex_libshout_version_minor}")
    string(REGEX REPLACE "${regex_libshout_version_minor}" "\\1" LIBSHOUT_VERSION_MINOR "${libshout_version_minor}")
    unset(regex_libshout_version_minor)
    unset(libshout_version_minor)

    set(regex_libshout_version_micro "^#define[ \t]+LIBSHOUT_VERSION_MICRO[ \t]+([^\"]+).*")
    file(STRINGS "${LIBSHOUT_INCLUDE_DIR}/libshout/version.h" libshout_version_micro REGEX "${regex_libshout_version_micro}")
    string(REGEX REPLACE "${regex_libshout_version_micro}" "\\1" LIBSHOUT_VERSION_MICRO "${libshout_version_micro}")
    unset(regex_libshout_version_micro)
    unset(libshout_version_micro)
		
		set (LIBSHOUT_VERSION_STRING "${LIBSHOUT_VERSION_MAJOR}.${LIBSHOUT_VERSION_MINOR}.${LIBSHOUT_VERSION_MICRO}")
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libshout
        REQUIRED_VARS LIBSHOUT_LIBRARY LIBSHOUT_INCLUDE_DIR
        VERSION_VAR LIBSHOUT_VERSION_STRING)

if (LIBSHOUT_FOUND)
    set(LIBSHOUT_LIBRARIES ${LIBSHOUT_LIBRARY})
    set(LIBSHOUT_INCLUDE_DIRS ${LIBSHOUT_INCLUDE_DIR})
    if (NOT TARGET Libshout::Libshout)
        add_library(Libshout::Libshout UNKNOWN IMPORTED)
        set_target_properties(Libshout::Libshout PROPERTIES
                IMPORTED_LOCATION "${LIBSHOUT_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${LIBSHOUT_INCLUDE_DIR}")
    endif ()
endif ()

mark_as_advanced(LIBSHOUT_INCLUDE_DIR LIBSHOUT_LIBRARY)
