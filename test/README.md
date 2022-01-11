# Taliesin unit tests

## Prerequisites

To run a valid batch of unit tests, you must enable Oauth2 authentication, use a valid Oauth2 authentication service correctly configured to provide JWT tokens with scopes `taliesin` and `taliesin_admin` (see [INSTALL.md](INSTALL.md)).

## Media folder

The media folder is located by default in `/tmp/media`, but you can set your own folder with the make variable `PARAM_MEDIA_FOLDER`.

You must have a media folder with the exact following content:

```
media/folder_cover/cover.jpg
media/folder_no_cover/Stallman 2.ogg
media/fss/free-software-song.au
media/fss/free-software-song-herzog.ogg
media/fss/free-software-song.ogg
media/fss/FreeSWSong.ogg
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/cover.jpg
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 01 Aria.mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 02 Variatio 1 a 1 Clav..mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 03 Variatio 2 a 1 Clav..mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 04 Variatio 3 a 1 Clav. Canone all Unisuono.mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 05 Variatio 4 a 1 Clav..mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 06 Variatio 5 a 1 ovvero 2 Clav..mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 07 Variatio 6 a 1 Clav. Canone alla Seconda.mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 08 Variatio 7 a 1 ovvero 2 Clav..mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 09 Variatio 8 a 2 Clav..mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 10 Variatio 9 a 1 Clav. Canone alla Terza.mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 11 Variatio 10 a 1 Clav. Fughetta.mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 12 Variatio 11 a 2 Clav..mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 13 Variatio 12 Canone alla Quarta.mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 14 Variatio 13 a 2 Clav..mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 15 Variatio 14 a 2 Clav..mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 16 Variatio 15 a 1 Clav. Canone alla Quinta.mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 17 Variatio 16 a 1 Clav. Ouverture.mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 18 Variatio 17 a 2 Clav..mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 19 Variatio 18 a 1 Clav. Canone alla Sexta.mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 20 Variatio 19 a 1 Clav..mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 21 Variatio 20 a 2 Clav..mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 22 Variatio 21 Canone alla Settima.mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 23 Variatio 22 a 1 Clav..mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 24 Variatio 23 a 2 Clav..mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 25 Variatio 24 a 1 Clav. Canone all Ottava.mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 26 Variatio 25 a 2 Clav..mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 27 Variatio 26 a 2 Clav..mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 28 Variatio 27 a 2 Clav. Canone alla Nona - Variatio 28 a 2 Clav..mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 29 Variatio 29 a 1 ovvero 2 Clav..mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 30 Variatio 30 a 1 Clav. Quodlibet.mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 31 Aria da Capo è Fine.mp3
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - OGV-CD2-0.png
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - open-goldberg-variations-booklet.pdf
media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - open-goldberg-variations.pdf
media/media_cover/01 - Aria.mp3
media/media_no_cover/Stallman 1.ogg
media/short/short1.mp3
media/short/short2.mp3
media/short/short3.mp3
```

The the folder `media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/` is the complete album [Bach's Open Goldberg Variations](https://www.opengoldbergvariations.org/) By Kimiko Ishizaka, which is distributed in Creative Commons CC0 licence, in mp3 format. It's important to get those files or equivalent ones because the search tests and categories tests are based on their ID3 tags.

The file `media/media_cover/01 - Aria.mp3` is a raw copy of `media/Kimiko Ishizaka/J.S. Bach: "Open" Goldberg Variations, BWV 988 (Piano)/Kimiko Ishizaka - J.S. Bach- -Open- Goldberg Variations, BWV 988 (Piano) - 03 Variatio 2 a 1 Clav..mp3` to test the attached cover.

The other files are not used for tag tests so you can use any other files, as long as they have the same path.

The folder `media/fss` contains variations of the [Free Software Song](https://www.gnu.org/music/free-software-song.html) and available on the FSF website:
```
https://www.gnu.org/music/free-software-song.au
https://www.gnu.org/music/free-software-song.ogg
https://www.gnu.org/music/FreeSWSong.ogg
https://www.gnu.org/music/free-software-song-herzog.ogg
```

The files `media/media_no_cover/Stallman 1.ogg` and `media/folder_no_cover/Stallman 2.ogg` I used are from the following web page: [http://web.mit.edu/echemi/www/040324.html](http://web.mit.edu/echemi/www/040324.html).

The files `media/short/short[1|2|3].mp3` are simple very short mp3 files (2 or 3 seconds) used to test webradio or jukebox playing with fast transcoding.

## Run tests

To run the unit tests, you must have a running instance of taliesin available on the same machine. The database used for the tests will have lots of IO commands, so a good option is to use a fresh database without any other Taliesin instance running. The config file that will be used must be placed in the `taliesin` root folder, where the file `taliesin.conf.sample` is located.

Then, go to the `src/` folder, and run one of the following commands:

```shell
$ make test-debug # To compile taliesin in debug mode and launch it
$ make memcheck # To compile taliesin in debug mode and launch it in a valgrind VM to check for memory leaks and other memory issues
```

Then, in another terminal, go to the folder `test/` and run the command `make test` with no parameters to run all the tests with the default options values.

The default options values are:

```
PARAM_MEDIA_FOLDER=/tmp/media
```

To set a parameter, append to the make command `KEY=value`, example:

```shell
$ make test PARAM_MEDIA_FOLDER=/new/path/to/test/folder
```

You can also run one single test at a time, run `maks test_[test_name]`, example:
```shell
$ make test_data_source_crud
```

The unit tests `data_source_create_test_environment` and `data_source_delete_test_environment` are used to create and clean a data source for the following tests:
- `data_source_browse_category`
- `data_source_browse_path`
- `playlist_crud`
- `search_advanced`
- `search_simple`
- `stream_playlist`
- `stream_webradio`

So if you want to run the test `search_advanced` for example, you must previously have ran `data_source_create_test_environment` When all your tests are complete, you can run the test `data_source_delete_test_environment` to clean your test database.
