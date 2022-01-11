#
# Taliesin media server
#
# Makefile used to build docker images
#
# Copyright 2017-2022 Nicolas Mora <mail@babelouest.org>
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
TARGET_X86_64_CUSTOM=x86_64_custom
TALIESIN_PACKAGE=taliesin-full_$(TALIESIN_VERSION)_alpine_$(ALPINE_VERSION)_x86_64.tar.gz
MEDIA=/tmp/media

TALIESIN_VERSION=$(shell curl -s https://api.github.com/repos/babelouest/taliesin/releases/latest | grep tag_name | cut -d '"' -f 4 | cut -c 2-)
ALPINE_VERSION=3.15.0

$(TALIESIN_PACKAGE):
	wget https://github.com/babelouest/taliesin/releases/download/v$(TALIESIN_VERSION)/$(TALIESIN_PACKAGE)

docker-custom: taliesin-full_$(TALIESIN_VERSION)_alpine_$(ALPINE_VERSION)_x86_64.tar.gz
	rm -f $(TARGET_SRC)/taliesin-full_$(TALIESIN_VERSION)_alpine_$(ALPINE_VERSION)_x86_64.tar.gz
	cp taliesin-full_$(TALIESIN_VERSION)_alpine_$(ALPINE_VERSION)_x86_64.tar.gz $(TARGET_SRC)/
	docker build -t babelouest/taliesin_x86_64:latest --build-arg TALIESIN_VERSION=$(TALIESIN_VERSION) --build-arg ALPINE_VERSION=$(ALPINE_VERSION) $(TARGET_SRC)/

docker-quickstart: taliesin-full_$(TALIESIN_VERSION)_alpine_$(ALPINE_VERSION)_x86_64.tar.gz
	rm -f $(TARGET_X86_64_CUSTOM)/taliesin-full_$(TALIESIN_VERSION)_alpine_$(ALPINE_VERSION)_x86_64.tar.gz
	cp taliesin-full_$(TALIESIN_VERSION)_alpine_$(ALPINE_VERSION)_x86_64.tar.gz $(TARGET_X86_64_CUSTOM)/
	docker build -t babelouest/taliesin_x86_64:quickstart $(TARGET_X86_64_CUSTOM)/

docker-quickstart-run:
	docker run -it --rm -p 8576:8576 -v $(MEDIA):/media/ babelouest/taliesin_x86_64:quickstart

clean:
	rm -f taliesin-full*_x86_64.tar.gz $(TARGET_SRC)/taliesin-full*_x86_64.tar.gz $(TARGET_X86_64_CUSTOM)/taliesin-full*_x86_64.tar.gz
	-docker rmi -f babelouest/taliesin_x86_64:quickstart babelouest/taliesin_x86_64:latest
	-docker system prune -f
