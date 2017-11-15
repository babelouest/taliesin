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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU GENERAL PUBLIC LICENSE for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

CC=gcc
#CC=clang
CFLAGS=-c -Wall $(ADDITIONALFLAGS)
LIBS=-lc -lulfius -lyder -ljansson -lorcania -lhoel -ljwt -lconfig -lpthread -lavcodec -lavutil -lavresample -lavformat -lavfilter -lswscale -lz
OBJECTS=glewlwyd_resource.o static_file_callback.o misc.o libav-transcode-filter.o config.o file_list.o filesystem.o data_source.o media.o playlist.o stream.o stream_jukebox.o stream_webradio.o search.o webservice.o taliesin.o
PREFIX=/usr/local
CONFIG_FILE=../taliesin.conf

all: release

clean:
	rm -f *.o taliesin valgrind.txt

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: taliesin

release: ADDITIONALFLAGS=-O3

release: taliesin

%.o: %.c taliesin.h
	$(CC) $(CFLAGS) $<

taliesin: $(OBJECTS)
	$(CC) -o $@ $^ $(LIBS)

memcheck: debug
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ./taliesin --config-file=$(CONFIG_FILE) 2>valgrind.txt

test-debug: debug
	./taliesin --config-file=$(CONFIG_FILE) -mconsole

install: taliesin
	cp -f taliesin $(PREFIX)/bin
