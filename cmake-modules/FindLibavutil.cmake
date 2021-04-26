#.rst:
# FindLibavutil
# -----------
#
# Find Libavutil
#
# Find Libavutil headers and libraries
#
# libavutil
#   LIBAVUTIL_FOUND          - True if libavutil found.
#   LIBAVUTIL_INCLUDE_DIRS   - Where to find libavutil.h.
#   LIBAVUTIL_LIBRARIES      - List of libraries when using libavutil.
#   LIBAVUTIL_VERSION_STRING - The version of libavutil found.
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
pkg_check_modules(PC_LIBAVUTIL QUIET libavutil)

find_path(LIBAVUTIL_INCLUDE_DIR
        NAMES libavutil/avutil.h
        HINTS ${PC_LIBAVUTIL_INCLUDEDIR} ${PC_LIBAVUTIL_INCLUDE_DIRS})

find_library(LIBAVUTIL_LIBRARY
        NAMES libavutil avutil
        HINTS ${PC_LIBAVUTIL_LIBDIR} ${PC_LIBAVUTIL_LIBRARY_DIRS})

set(LIBAVUTIL_VERSION_MAJOR 0)
set(LIBAVUTIL_VERSION_MINOR 0)
set(LIBAVUTIL_VERSION_MICRO 0)
if (PC_LIBAVUTIL_VERSION)
    set(LIBAVUTIL_VERSION_STRING ${PC_LIBAVUTIL_VERSION})
elseif (LIBAVUTIL_INCLUDE_DIR AND EXISTS "${LIBAVUTIL_INCLUDE_DIR}/libavutil/version.h")
    set(regex_libavutil_version_major "^#define[ \t]+LIBAVUTIL_VERSION_MAJOR[ \t]+([^\"]+).*")
    file(STRINGS "${LIBAVUTIL_INCLUDE_DIR}/libavutil/version.h" libavutil_version_major REGEX "${regex_libavutil_version_major}")
    string(REGEX REPLACE "${regex_libavutil_version_major}" "\\1" LIBAVUTIL_VERSION_MAJOR "${libavutil_version_major}")
    unset(regex_libavutil_version_major)
    unset(libavutil_version_major)

    set(regex_libavutil_version_minor "^#define[ \t]+LIBAVUTIL_VERSION_MINOR[ \t]+([^\"]+).*")
    file(STRINGS "${LIBAVUTIL_INCLUDE_DIR}/libavutil/version.h" libavutil_version_minor REGEX "${regex_libavutil_version_minor}")
    string(REGEX REPLACE "${regex_libavutil_version_minor}" "\\1" LIBAVUTIL_VERSION_MINOR "${libavutil_version_minor}")
    unset(regex_libavutil_version_minor)
    unset(libavutil_version_minor)

    set(regex_libavutil_version_micro "^#define[ \t]+LIBAVUTIL_VERSION_MICRO[ \t]+([^\"]+).*")
    file(STRINGS "${LIBAVUTIL_INCLUDE_DIR}/libavutil/version.h" libavutil_version_micro REGEX "${regex_libavutil_version_micro}")
    string(REGEX REPLACE "${regex_libavutil_version_micro}" "\\1" LIBAVUTIL_VERSION_MICRO "${libavutil_version_micro}")
    unset(regex_libavutil_version_micro)
    unset(libavutil_version_micro)
		
		set (LIBAVUTIL_VERSION_STRING "${LIBAVUTIL_VERSION_MAJOR}.${LIBAVUTIL_VERSION_MINOR}.${LIBAVUTIL_VERSION_MICRO}")
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libavutil
        REQUIRED_VARS LIBAVUTIL_LIBRARY LIBAVUTIL_INCLUDE_DIR
        VERSION_VAR LIBAVUTIL_VERSION_STRING)

if (LIBAVUTIL_FOUND)
    set(LIBAVUTIL_LIBRARIES ${LIBAVUTIL_LIBRARY})
    set(LIBAVUTIL_INCLUDE_DIRS ${LIBAVUTIL_INCLUDE_DIR})
endif ()

mark_as_advanced(LIBAVUTIL_INCLUDE_DIR LIBAVUTIL_LIBRARY)
