# Taliesin

Lightweight audio media server with a REST API interface and a React JS client application.

![example snapshot](https://github.com/babelouest/taliesin/raw/master/docs/images/dashboard.png)

Can be used as a cloud application for a set of media files.

The web application uses a responsive design to be usable as much in a desktop computer with a large screen or a small screen such as smartphones or tablets.

![Dashboard small screen](https://github.com/babelouest/taliesin/raw/master/docs/images/dashboard-smartphone.png)

Main functionalities:
- Play media files as a jukebox: the user chooses the songs to play
- Play media files as a webradio: the files are played in sequence or at random in the same stream
- Play media files in the client application, in an external player like VLC, or even control one or multiple MPD services with [Angharad House Automation System](https://github.com/babelouest/angharad)

Handles media metadata like id3 tags, media cover, whether the cover is integrated in the media file or as an image in the folder of the media.

The user can create audio jukebox streams of the following formats: OGG/Vorbis, MP3 and FLAC, it's also possible to specify the channels, bitrate or sample rate for any stream at creation.

Navigation modes: via file names or media metadata.

Database required: MariaDB/MySQL or SQLite3 are supported

Authentication method: An OAuth2 server providing JWT like [Glewlwyd](https://github.com/babelouest/glewlwyd), or no authentication.

Taliesin is a Free Software.

The API source code is available under the terms of the GNU GPL V3 license, the web application is available under the terms of the MIT license.

## Documentation

Installation documentation is available in the file [INSTALL.md](https://github.com/babelouest/taliesin/blob/master/docs/INSTALL.md).

Server API description is available in the file [API.md](https://github.com/babelouest/taliesin/blob/master/docs/API.md).

## Screenshots

Screenshots examples are available in the [images](https://github.com/babelouest/taliesin/tree/master/docs/images) folder.

## Problems, questions?

Pull requests are welcome if you want to help, you can open an issue or send an e-mail (mail@babelouest.io) if you have questions or problems.

Copyright 2017-2022 - Nicolas Mora <mail@babelouest.io>
