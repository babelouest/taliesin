# Taliesin

Audio media server with a REST API interface and a React JS client application.

Copyright 2017 - Nicolas Mora <mail@babelouest.org>

![example snapshot](https://github.com/babelouest/taliesin/raw/master/doc/images/dashboard.png)

Can be used as a cloud application for a set of media files.

Main functionalities:
- Play media files as a jukebox: the user chooses the songs to play
- Play media files as a webradio: the files are played in sequence or at random in the same stream
- Play media files in the client application, in an external player like VLC, or even control one or multiple MPD services with [Angharad House Automation System](https://github.com/babelouest/angharad)

Navigation modes: via file names or media metadata.

Database required: MariaDB/MySQL or SQLite3 are supported

Authentication method: An OAuth2 server providing JWT like [Glewlwyd](https://github.com/babelouest/glewlwyd), or no authentication.

## Documentation

Installation documentation is available in the file [INSTALL.md](https://github.com/babelouest/taliesin/blob/master/docs/INSTALL.md).

Server API description is available in the file [API.md](https://github.com/babelouest/taliesin/blob/master/docs/API.md).
