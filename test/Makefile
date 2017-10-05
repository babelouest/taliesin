#
# Taliesin - Media server
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
CFLAGS=-Wall -D_REENTRANT -DDEBUG -g -O0
LIBS=-lc -lulfius -lorcania -ljansson -lyder -lcheck -lpthread -lm -lrt -lsubunit

PARAM_USER=user1
PARAM_PASSWORD=MyUser1Password!
PARAM_SCOPE=taliesin
PARAM_ADM_USER=admin
PARAM_ADM_PASSWORD=MyAdminPassword2016!
PARAM_ADM_SCOPE=taliesin_admin
PARAM_GLEWLWYD_URL=http://localhost:4593/api
PARAM_MEDIA_FOLDER=/tmp/media

all: build test

clean:
	rm -f *.o *.log *.err config data_source_crud data_source_refresh data_source_create_test_environment data_source_browse_path data_source_browse_category playlist_crud stream_webradio stream_jukebox search_simple search_advanced data_source_delete_test_environment

unit-tests.o: unit-tests.c unit-tests.h
	@$(CC) $(CFLAGS) -c unit-tests.c

config: config.c unit-tests.o
	@$(CC) $(CFLAGS) config.c unit-tests.o -o config $(LIBS)

test_config: config
	./run_test.sh ./config $(PARAM_USER) $(PARAM_PASSWORD) $(PARAM_SCOPE) $(PARAM_ADM_USER) $(PARAM_ADM_PASSWORD) $(PARAM_ADM_SCOPE) $(PARAM_GLEWLWYD_URL) $(PARAM_MEDIA_FOLDER)

data_source_crud: data_source_crud.c unit-tests.o
	@$(CC) $(CFLAGS) data_source_crud.c unit-tests.o -o data_source_crud $(LIBS)

test_data_source_crud: data_source_crud
	@-./run_test.sh ./data_source_crud $(PARAM_USER) $(PARAM_PASSWORD) $(PARAM_SCOPE) $(PARAM_ADM_USER) $(PARAM_ADM_PASSWORD) $(PARAM_ADM_SCOPE) $(PARAM_GLEWLWYD_URL) $(PARAM_MEDIA_FOLDER)

data_source_refresh: data_source_refresh.c unit-tests.o
	@$(CC) $(CFLAGS) data_source_refresh.c unit-tests.o -o data_source_refresh $(LIBS)

test_data_source_refresh: data_source_refresh
	@-./run_test.sh ./data_source_refresh $(PARAM_USER) $(PARAM_PASSWORD) $(PARAM_SCOPE) $(PARAM_ADM_USER) $(PARAM_ADM_PASSWORD) $(PARAM_ADM_SCOPE) $(PARAM_GLEWLWYD_URL) $(PARAM_MEDIA_FOLDER)

data_source_create_test_environment: data_source_create_test_environment.c unit-tests.o
	@$(CC) $(CFLAGS) data_source_create_test_environment.c unit-tests.o -o data_source_create_test_environment $(LIBS)

test_data_source_create_test_environment: data_source_create_test_environment
	@-./run_test.sh ./data_source_create_test_environment $(PARAM_USER) $(PARAM_PASSWORD) $(PARAM_SCOPE) $(PARAM_ADM_USER) $(PARAM_ADM_PASSWORD) $(PARAM_ADM_SCOPE) $(PARAM_GLEWLWYD_URL) $(PARAM_MEDIA_FOLDER)

data_source_browse_path: data_source_browse_path.c unit-tests.o
	@$(CC) $(CFLAGS) data_source_browse_path.c unit-tests.o -o data_source_browse_path $(LIBS)

test_data_source_browse_path: data_source_browse_path
	@-./run_test.sh ./data_source_browse_path $(PARAM_USER) $(PARAM_PASSWORD) $(PARAM_SCOPE) $(PARAM_ADM_USER) $(PARAM_ADM_PASSWORD) $(PARAM_ADM_SCOPE) $(PARAM_GLEWLWYD_URL)

data_source_browse_category: data_source_browse_category.c unit-tests.o
	@$(CC) $(CFLAGS) data_source_browse_category.c unit-tests.o -o data_source_browse_category $(LIBS)

test_data_source_browse_category: data_source_browse_category
	@-./run_test.sh ./data_source_browse_category $(PARAM_USER) $(PARAM_PASSWORD) $(PARAM_SCOPE) $(PARAM_ADM_USER) $(PARAM_ADM_PASSWORD) $(PARAM_ADM_SCOPE) $(PARAM_GLEWLWYD_URL)

playlist_crud: playlist_crud.c unit-tests.o
	@$(CC) $(CFLAGS) playlist_crud.c unit-tests.o -o playlist_crud $(LIBS)

test_playlist_crud: playlist_crud
	@-./run_test.sh ./playlist_crud $(PARAM_USER) $(PARAM_PASSWORD) $(PARAM_SCOPE) $(PARAM_ADM_USER) $(PARAM_ADM_PASSWORD) $(PARAM_ADM_SCOPE) $(PARAM_GLEWLWYD_URL)

search_simple: search_simple.c unit-tests.o
	@$(CC) $(CFLAGS) search_simple.c unit-tests.o -o search_simple $(LIBS)

test_search_simple: search_simple
	@-./run_test.sh ./search_simple $(PARAM_USER) $(PARAM_PASSWORD) $(PARAM_SCOPE) $(PARAM_ADM_USER) $(PARAM_ADM_PASSWORD) $(PARAM_ADM_SCOPE) $(PARAM_GLEWLWYD_URL)

search_advanced: search_advanced.c unit-tests.o
	@$(CC) $(CFLAGS) search_advanced.c unit-tests.o -o search_advanced $(LIBS)

test_search_advanced: search_advanced
	@-./run_test.sh ./search_advanced $(PARAM_USER) $(PARAM_PASSWORD) $(PARAM_SCOPE) $(PARAM_ADM_USER) $(PARAM_ADM_PASSWORD) $(PARAM_ADM_SCOPE) $(PARAM_GLEWLWYD_URL)

stream_webradio: stream_webradio.c unit-tests.o
	@$(CC) $(CFLAGS) stream_webradio.c unit-tests.o -o stream_webradio $(LIBS)

test_stream_webradio: stream_webradio
	@-./run_test.sh ./stream_webradio $(PARAM_USER) $(PARAM_PASSWORD) $(PARAM_SCOPE) $(PARAM_ADM_USER) $(PARAM_ADM_PASSWORD) $(PARAM_ADM_SCOPE) $(PARAM_GLEWLWYD_URL)

stream_jukebox: stream_jukebox.c unit-tests.o
	@$(CC) $(CFLAGS) stream_jukebox.c unit-tests.o -o stream_jukebox $(LIBS)

test_stream_jukebox: stream_jukebox
	@-./run_test.sh ./stream_jukebox $(PARAM_USER) $(PARAM_PASSWORD) $(PARAM_SCOPE) $(PARAM_ADM_USER) $(PARAM_ADM_PASSWORD) $(PARAM_ADM_SCOPE) $(PARAM_GLEWLWYD_URL)

data_source_delete_test_environment: data_source_delete_test_environment.c unit-tests.o
	@$(CC) $(CFLAGS) data_source_delete_test_environment.c unit-tests.o -o data_source_delete_test_environment $(LIBS)

test_data_source_delete_test_environment: data_source_delete_test_environment
	@-./run_test.sh ./data_source_delete_test_environment $(PARAM_USER) $(PARAM_PASSWORD) $(PARAM_SCOPE) $(PARAM_ADM_USER) $(PARAM_ADM_PASSWORD) $(PARAM_ADM_SCOPE) $(PARAM_GLEWLWYD_URL)

build: config data_source_crud data_source_refresh data_source_create_environment data_source_browse_path data_source_browse_category playlist_crud search_simple search_advanced stream_webradio stream_jukebox data_source_delete_environment

test: test_config test_data_source_crud test_data_source_refresh test_data_source_create_test_environment test_data_source_browse_path test_data_source_browse_category test_playlist_crud test_search_simple test_search_advanced test_stream_webradio test_stream_jukebox test_data_source_delete_test_environment
