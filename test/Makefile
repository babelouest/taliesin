#
# Taliesin - Media server
#
# Makefile used to build the tests
#
# Public domain, no copyright. Use at your own risk.
#

CC=gcc
CFLAGS=-Wall -D_REENTRANT -DDEBUG -g -O0
LDFLAGS=-lc -lulfius -lorcania -lrhonabwy -ljansson -lyder -lcheck -lpthread -lm -lrt -lsubunit
TARGET=config data_source_crud data_source_refresh data_source_create_test_environment data_source_browse_path data_source_browse_category playlist_crud search_simple search_advanced stream_webradio stream_jukebox data_source_delete_test_environment
VALGRIND_COMMAND=valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all

VERBOSE=0
MEMCHECK=0

PARAM_JWKS_PATH=../private-test.jwks
PARAM_MEDIA_FOLDER=/tmp/media

all: build test

clean:
	rm -f *.o *.log valgrind-*.txt $(TARGET)

unit-tests.o: unit-tests.c unit-tests.h
	$(CC) $(CFLAGS) -c unit-tests.c

%: %.c unit-tests.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_%: %
	@if [ "$(VERBOSE)" = "0" ] && [ "$(MEMCHECK)" = "0" ]; then \
		LD_LIBRARY_PATH=. ./run_test.sh ./$^ $(PARAM_JWKS_PATH) $(PARAM_MEDIA_FOLDER); \
	elif [ "$(MEMCHECK)" = "0" ]; then \
		LD_LIBRARY_PATH=. ./$^ $(PARAM_JWKS_PATH) $(PARAM_MEDIA_FOLDER); \
	else \
		CK_FORK=no LD_LIBRARY_PATH=. $(VALGRIND_COMMAND) ./$^ $(PARAM_JWKS_PATH) $(PARAM_MEDIA_FOLDER) 2>valgrind-$@.txt; \
	fi

build: $(TARGET)

test: $(TARGET) test_config test_data_source_crud test_data_source_refresh test_data_source_create_test_environment test_data_source_browse_path test_data_source_browse_category test_playlist_crud test_search_simple test_search_advanced test_stream_webradio test_stream_jukebox test_data_source_delete_test_environment
