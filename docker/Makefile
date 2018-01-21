#
# Taliesin media server
#
# Makefile used to build docker images
#
# Copyright 2017-2018 Nicolas Mora <mail@babelouest.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU GENERAL PUBLIC LICENSE
# License as published by the Free Software Foundation;
# version 3 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU GENERAL PUBLIC LICENSE for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

TARGET_SRC=src
TARGET_X86_64=x86_64
TARGET_X86_64_SQLITE3_NOAUTH=x86_64_sqlite3_noauth

build-src:
	cp -R ../src ../webapp $(TARGET_SRC)/
	docker build -t babelouest/taliesin_x86_64:src $(TARGET_SRC)/
	docker run -v /tmp/taliesin/:/var/taliesin/host/ babelouest/taliesin_x86_64:src
	rm -rf $(TARGET_SRC)/src/ $(TARGET_SRC)/webapp/
	cp /tmp/taliesin/taliesin.tar.gz $(TARGET_X86_64)

build-x86_64:
	docker build -t babelouest/taliesin_x86_64:latest $(TARGET_X86_64)/

build-x86_64_sqlite_noauth_quickstart:
	docker build -t babelouest/taliesin_x86_64_sqlite_noauth_quickstart:latest $(TARGET_X86_64_SQLITE3_NOAUTH)

build-all: build-src build-x86_64 build-x86_64_sqlite_noauth_quickstart
