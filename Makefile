#
# Taliesin media server
#
# Makefile used to build the software
#
# Copyright 2017 Nicolas Mora <mail@babelouest.org>
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

TALIESIN_SOURCE=./src
TALIESIN_TESTS=./test

all:
	cd $(TALIESIN_SOURCE) && $(MAKE)

debug:
	cd $(TALIESIN_SOURCE) && $(MAKE) debug

install:
	cd $(TALIESIN_SOURCE) && $(MAKE) install

memcheck:
	cd $(TALIESIN_SOURCE) && $(MAKE) memcheck

test-debug:
	cd $(TALIESIN_SOURCE) && $(MAKE) test-debug

test:
	cd $(TALIESIN_TESTS) && $(MAKE) test

clean:
	cd $(TALIESIN_SOURCE) && $(MAKE) clean
	cd $(TALIESIN_TESTS) && $(MAKE) clean
