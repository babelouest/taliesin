#
# Taliesin media server
#
# Makefile used to build the software
#
# Copyright 2017-2023 Nicolas Mora <mail@babelouest.org>
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

ifndef DISABLE_OAUTH2
	OAUTH2_OBJECTS=iddawc_resource.o
	INITIAL_LIBS=$(shell pkg-config --libs librhonabwy) $(shell pkg-config --libs libiddawc)
else
	INITIAL_FLAG=-DDISABLE_OAUTH2
endif

CC=gcc
#CC=clang
CFLAGS=-c -Wall -Werror -Wextra -Wconversion -Wpedantic $(ADDITIONALFLAGS)
#CFLAGS=-c -Wall -Werror -Wextra -Wconversion -D_FILE_OFFSET_BITS=64 $(ADDITIONALFLAGS)
LIBS=-lc $(shell pkg-config --libs liborcania) $(shell pkg-config --libs libyder) $(shell pkg-config --libs libulfius) $(shell pkg-config --libs libhoel) $(shell pkg-config --libs jansson) $(shell pkg-config --libs gnutls) $(shell pkg-config --libs libconfig) $(shell pkg-config --libs nettle) $(shell pkg-config --libs hogweed) $(shell pkg-config --libs nettle) $(shell pkg-config --libs shout) -lpthread -lavcodec -lavutil -lswresample -lavformat -lswscale -lz $(INITIAL_LIBS)
OBJECTS=$(OAUTH2_OBJECTS) taliesin.o webservice.o static_compressed_inmemory_website_callback.o http_compression_callback.o misc.o libav-transcode-filter.o config.o file_list.o filesystem.o data_source.o media.o playlist.o stream.o stream_jukebox.o stream_webradio.o search.o
PREFIX=/usr/local
CONFIG_FILE=../taliesin.conf

all: release

clean:
	rm -f *.o taliesin valgrind.txt

debug: ADDITIONALFLAGS=$(INITIAL_FLAG) -DDEBUG -g -O0

debug: taliesin

release: ADDITIONALFLAGS=$(INITIAL_FLAG) -O3

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
