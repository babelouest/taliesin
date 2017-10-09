# Taliesin

Audio media server with a REST API.

Beta version, still work in progress. Not yet easy user ready, but the API is almost complete.

## Install

Requires a recent version of libav or ffmpeg, as in Debian Stretch or Ubuntu 17.04 version for example, I'm not sure about the version number, maybe 11 at least. Requires [ulfius](https://github.com/babelouest/ulfius), [hoel](https://github.com/babelouest/hoel), [libjwt](https://github.com/benmcollins/libjwt), libconfig and their dependencies.

```shell
$ apt install -y libjansson-dev libavfilter-dev libavcodec-dev libavformat-dev libavresample-dev libavutil-dev libcurl4-gnutls-dev libgnutls28-dev libgcrypt20-dev libsqlite3-dev libmariadbclient-dev libconfig8-dev
$ git clone https://github.com/benmcollins/libjwt.git
$ cd libjwt
$ autoreconf -a && ./configure --without-openssl && make && sudo make install
$ cd ..

# Install Orcania
$ git clone https://github.com/babelouest/orcania.git
$ cd orcania/
$ make
$ sudo make install
$ cd ../..

# Install Yder
$ git clone https://github.com/babelouest/yder.git
$ cd yder/src/
$ make
$ sudo make install
$ cd ../..

# Install Ulfius
$ git clone https://github.com/babelouest/ulfius.git
$ cd ulfius/src/
$ make
$ sudo make install
$ cd ../..

# Install Hoel
$ git clone https://github.com/babelouest/hoel.git
$ cd hoel/src/
$ make
$ sudo make install
$ cd ../..

$ git clone git@github.com:babelouest/taliesin.git
$ cd taliesin/src
$ make && sudo make install
$ mysql taliesin < ../taliesin.mariadb.sql # or sqlite /tmp/taliesin.db < ../taliesin.sqlite3.sql
$ sudo mkdir -p /usr/local/etc/taliesin && sudo cp taliesin.conf.sample /usr/local/etc/taliesin/taliesin.conf
# Edit /usr/local/etc/taliesin/taliesin.conf to your needs
$ sudo cp taliesin.service /etc/systemd/system
$ sudo systemctl enable taliesin.service
$ sudo service taliesin start
```

Then open [http://localhost:8576/app/](http://localhost:8576/app/).
