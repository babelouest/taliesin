#.rst:
# FindLibavfilter
# -----------
#
# Find Libavfilter
#
# Find Libavfilter headers and libraries
#
# libavfilter
#   LIBAVFILTER_FOUND          - True if libavfilter found.
#   LIBAVFILTER_INCLUDE_DIRS   - Where to find libavfilter.h.
#   LIBAVFILTER_LIBRARIES      - List of libraries when using libavfilter.
#   LIBAVFILTER_VERSION_STRING - The version of libavfilter found.
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
pkg_check_modules(PC_LIBAVFILTER QUIET libavfilter)

find_path(LIBAVFILTER_INCLUDE_DIR
        NAMES libavfilter/avfilter.h
        HINTS ${PC_LIBAVFILTER_INCLUDEDIR} ${PC_LIBAVFILTER_INCLUDE_DIRS})

find_library(LIBAVFILTER_LIBRARY
        NAMES libavfilter avfilter
        HINTS ${PC_LIBAVFILTER_LIBDIR} ${PC_LIBAVFILTER_LIBRARY_DIRS})

set(LIBAVFILTER_VERSION_MAJOR 0)
set(LIBAVFILTER_VERSION_MINOR 0)
set(LIBAVFILTER_VERSION_MICRO 0)
if (PC_LIBAVFILTER_VERSION)
    set(LIBAVFILTER_VERSION_STRING ${PC_LIBAVFILTER_VERSION})
elseif (LIBAVFILTER_INCLUDE_DIR AND EXISTS "${LIBAVFILTER_INCLUDE_DIR}/libavfilter/version.h")
    set(regex_libavfilter_version_major "^#define[ \t]+LIBAVFILTER_VERSION_MAJOR[ \t]+([^\"]+).*")
    file(STRINGS "${LIBAVFILTER_INCLUDE_DIR}/libavfilter/version.h" libavfilter_version_major REGEX "${regex_libavfilter_version_major}")
    string(REGEX REPLACE "${regex_libavfilter_version_major}" "\\1" LIBAVFILTER_VERSION_MAJOR "${libavfilter_version_major}")
    unset(regex_libavfilter_version_major)
    unset(libavfilter_version_major)

    set(regex_libavfilter_version_minor "^#define[ \t]+LIBAVFILTER_VERSION_MINOR[ \t]+([^\"]+).*")
    file(STRINGS "${LIBAVFILTER_INCLUDE_DIR}/libavfilter/version.h" libavfilter_version_minor REGEX "${regex_libavfilter_version_minor}")
    string(REGEX REPLACE "${regex_libavfilter_version_minor}" "\\1" LIBAVFILTER_VERSION_MINOR "${libavfilter_version_minor}")
    unset(regex_libavfilter_version_minor)
    unset(libavfilter_version_minor)

    set(regex_libavfilter_version_micro "^#define[ \t]+LIBAVFILTER_VERSION_MICRO[ \t]+([^\"]+).*")
    file(STRINGS "${LIBAVFILTER_INCLUDE_DIR}/libavfilter/version.h" libavfilter_version_micro REGEX "${regex_libavfilter_version_micro}")
    string(REGEX REPLACE "${regex_libavfilter_version_micro}" "\\1" LIBAVFILTER_VERSION_MICRO "${libavfilter_version_micro}")
    unset(regex_libavfilter_version_micro)
    unset(libavfilter_version_micro)
		
		set (LIBAVFILTER_VERSION_STRING "${LIBAVFILTER_VERSION_MAJOR}.${LIBAVFILTER_VERSION_MINOR}.${LIBAVFILTER_VERSION_MICRO}")
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libavfilter
        REQUIRED_VARS LIBAVFILTER_LIBRARY LIBAVFILTER_INCLUDE_DIR
        VERSION_VAR LIBAVFILTER_VERSION_STRING)

if (LIBAVFILTER_FOUND)
    set(LIBAVFILTER_LIBRARIES ${LIBAVFILTER_LIBRARY})
    set(LIBAVFILTER_INCLUDE_DIRS ${LIBAVFILTER_INCLUDE_DIR})
endif ()

mark_as_advanced(LIBAVFILTER_INCLUDE_DIR LIBAVFILTER_LIBRARY)
