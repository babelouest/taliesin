#.rst:
# FindLibavdevice
# -----------
#
# Find libavdevice
#
# Find libavdevice headers and libraries
#
# libavdevice
#   LIBAVDEVICE_FOUND          - True if libavdevice found.
#   LIBAVDEVICE_INCLUDE_DIRS   - Where to find libavdevice.h.
#   LIBAVDEVICE_LIBRARIES      - List of libraries when using libavdevice.
#   LIBAVDEVICE_VERSION_STRING - The version of libavdevice found.
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
pkg_check_modules(PC_LIBAVDEVICE QUIET libavdevice)

find_path(LIBAVDEVICE_INCLUDE_DIR
        NAMES libavdevice/avdevice.h
        HINTS ${PC_LIBAVDEVICE_INCLUDEDIR} ${PC_LIBAVDEVICE_INCLUDE_DIRS})

find_library(LIBAVDEVICE_LIBRARY
        NAMES libavdevice avdevice
        HINTS ${PC_LIBAVDEVICE_LIBDIR} ${PC_LIBAVDEVICE_LIBRARY_DIRS})

set(LIBAVDEVICE_VERSION_MAJOR 0)
set(LIBAVDEVICE_VERSION_MINOR 0)
set(LIBAVDEVICE_VERSION_MICRO 0)
if (PC_LIBAVDEVICE_VERSION)
    set(LIBAVDEVICE_VERSION_STRING ${PC_LIBAVDEVICE_VERSION})
elseif (LIBAVDEVICE_INCLUDE_DIR AND EXISTS "${LIBAVDEVICE_INCLUDE_DIR}/libavdevice/version.h")
    set(regex_libavdevice_version_major "^#define[ \t]+LIBAVDEVICE_VERSION_MAJOR[ \t]+([^\"]+).*")
    file(STRINGS "${LIBAVDEVICE_INCLUDE_DIR}/libavdevice/version.h" libavdevice_version_major REGEX "${regex_libavdevice_version_major}")
    string(REGEX REPLACE "${regex_libavdevice_version_major}" "\\1" LIBAVDEVICE_VERSION_MAJOR "${libavdevice_version_major}")
    unset(regex_libavdevice_version_major)
    unset(libavdevice_version_major)

    set(regex_libavdevice_version_minor "^#define[ \t]+LIBAVDEVICE_VERSION_MINOR[ \t]+([^\"]+).*")
    file(STRINGS "${LIBAVDEVICE_INCLUDE_DIR}/libavdevice/version.h" libavdevice_version_minor REGEX "${regex_libavdevice_version_minor}")
    string(REGEX REPLACE "${regex_libavdevice_version_minor}" "\\1" LIBAVDEVICE_VERSION_MINOR "${libavdevice_version_minor}")
    unset(regex_libavdevice_version_minor)
    unset(libavdevice_version_minor)

    set(regex_libavdevice_version_micro "^#define[ \t]+LIBAVDEVICE_VERSION_MICRO[ \t]+([^\"]+).*")
    file(STRINGS "${LIBAVDEVICE_INCLUDE_DIR}/libavdevice/version.h" libavdevice_version_micro REGEX "${regex_libavdevice_version_micro}")
    string(REGEX REPLACE "${regex_libavdevice_version_micro}" "\\1" LIBAVDEVICE_VERSION_MICRO "${libavdevice_version_micro}")
    unset(regex_libavdevice_version_micro)
    unset(libavdevice_version_micro)
		
		set (LIBAVDEVICE_VERSION_STRING "${LIBAVDEVICE_VERSION_MAJOR}.${LIBAVDEVICE_VERSION_MINOR}.${LIBAVDEVICE_VERSION_MICRO}")
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libavdevice
        REQUIRED_VARS LIBAVDEVICE_LIBRARY LIBAVDEVICE_INCLUDE_DIR
        VERSION_VAR LIBAVDEVICE_VERSION_STRING)

if (LIBAVDEVICE_FOUND)
    set(LIBAVDEVICE_LIBRARIES ${LIBAVDEVICE_LIBRARY})
    set(LIBAVDEVICE_INCLUDE_DIRS ${LIBAVDEVICE_INCLUDE_DIR})
    if (NOT TARGET Libav::Avdevice)
        add_library(Libav::Avdevice UNKNOWN IMPORTED)
        set_target_properties(Libav::Avdevice PROPERTIES
                IMPORTED_LOCATION "${LIBAVDEVICE_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${LIBAVDEVICE_INCLUDE_DIR}")
    endif ()
endif ()

mark_as_advanced(LIBAVDEVICE_INCLUDE_DIR LIBAVDEVICE_LIBRARY)
