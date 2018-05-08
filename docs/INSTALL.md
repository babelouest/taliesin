# Taliesin installation

## Automatic install script for Raspberry Pi running Raspbian

Follow the documentation [docs/minimal/README.md](https://github.com/babelouest/taliesin/blob/master/docs/minimal/README.md) to install a minimal instance of Taliesin on a Raspberry PI with Raspbian installed.

## Pre-compiled packages

You can install Taliesin with a pre-compiled package available in the [release pages](https://github.com/babelouest/taliesin/releases/latest/). The package files `taliesin-full_*` contain the package libraries of `orcania`, `yder`, `ulfius` and `hoel` precompiled for `taliesin`, plus `taliesin` package. To install a pre-compiled package, you need to have installed the following libraries:

```
libjansson
libavfilter
libavcodec
libavformat
libavresample
libavutil
libcurl-gnutls
libgnutls
libgcrypt
libsqlite3
libmariadbclient
libconfig
```

For example, to install Taliesin with the `taliesin-full_1.0.12_Debian_stretch_x86_64.tar.gz` package downloaded on the `releases` page, you must execute the following commands:

```shell
$ sudo apt install -y autoconf libjansson-dev libssl-dev libavfilter libavcodec libavformat libavresample libavutil libcurl-gnutls libgnutls libgcrypt libsqlite3 libmariadbclient libconfig
$ wget https://github.com/benmcollins/libjwt/archive/v1.9.tar.gz
$ tar -zxvf v1.9.tar.gz
$ cd libjwt-1.9
$ autoreconf -i
$ ./configure
$ make && sudo make install
$ wget https://github.com/babelouest/taliesin/releases/download/v1.0.12/taliesin-full_1.0.12_Debian_stretch_x86_64.tar.gz
$ tar xf hoel-dev-full_1.4.0_Debian_stretch_x86_64.tar.gz
$ sudo dpkg -i liborcania_1.2.0_Debian_stretch_x86_64.deb
$ sudo dpkg -i libyder_1.2.0_Debian_stretch_x86_64.deb
$ sudo dpkg -i libhoel_1.4.0_Debian_stretch_x86_64.deb
$ sudo dpkg -i libulfius_2.3.0_Debian_stretch_x86_64.deb
$ sudo dpkg -i taliesin_1.0.12_Debian_stretch_x86_64.deb
```

If there's no package available for your distribution, you can recompile it manually using `CMake` or `Makefile`.

## Docker image

You can install a Docker image based on Alpine Linux 3.7.

### Quickstart with a Taliesin server without authentication and a SQLite3 database

You must map a volume to store the SQLite3 database on the docker host and another volume to map the path to your media files.

Example:

```shell
docker run --rm -it -p 8576:8576 -v /tmp/taliesin/:/var/cache/taliesin -v /media/Music/:/media babelouest/taliesin_x86_64_sqlite_noauth_quickstart
```

### Quickstart with your own configuration file

The folder `taliesin/docker/x86_64_custom` contains a Docker file, sql and configuration files.

You can update the files `config.json` and `taliesin.conf` with your own settings, update the file `Docker` and uncomment some of the following if required:

```shell
#COPY ["taliesin.sqlite3.sql", "/"]
#COPY ["taliesin.mariadb.sql", "/"]
#COPY ["oauth-key.pem", "/var/taliesin/conf/"]
```

Then, build the image:

```shell
$ cd taliesin/docker/x86_64_custom
$ docker build -t my_taliesin .
```

When your docker image is ready, you can run it, don't forget to map the media volume at least, and the sqlite3 database volume if necessary.

#### Run a built docker image configured with a SQLite3 database

```shell
$ docker run -it --rm -p 8576:8576 -v /tmp/taliesin/:/var/cache/taliesin -v /media/Music/:/media my_taliesin
```

#### Run a built docker image configured with a MariaDB/Mysql database

```shell
$ docker run -it --rm -p 8576:8576 -p 3306:3306 -v /media/Music/:/media my_taliesin
```

## Manual install Taliesin and its dependencies

Taliesin requires [libav](https://libav.org/) version 11, or equivalent ffmpeg, available in Debian Stretch or Ubuntu 17.04. It also requires [ulfius](https://github.com/babelouest/ulfius), [hoel](https://github.com/babelouest/hoel), [libjwt](https://github.com/benmcollins/libjwt), [libconfig](http://www.hyperrealm.com/libconfig/libconfig.html) and their dependencies.

```shell
$ # Install libraries
$ apt install -y libjansson-dev libavfilter-dev libavcodec-dev libavformat-dev libavresample-dev libavutil-dev libcurl4-gnutls-dev libgnutls28-dev libgcrypt20-dev libsqlite3-dev libmariadbclient-dev libconfig8-dev
$ git clone https://github.com/benmcollins/libjwt.git
$ cd libjwt
$ autoreconf -a && ./configure --without-openssl && make && sudo make install
$ cd ..
```

### CMake

Download Taliesin from Github, then use the CMake script to build the application:

```shell
# Install Taliesin
$ git clone https://github.com/babelouest/taliesin.git
$ mkdir taliesin/build
$ cd taliesin/build
$ cmake ..
$ make 
$ sudo make install
```

### Good ol' Makefile

Download Taliesin and its dependencies hosted on github, compile and install.

```shell
$ # Install Orcania
$ git clone https://github.com/babelouest/orcania.git
$ cd orcania/src/
$ make
$ sudo make install
$ cd ../..

$ # Install Yder
$ git clone https://github.com/babelouest/yder.git
$ cd yder/src/
$ make
$ sudo make install
$ cd ../..

$ # Install Ulfius
$ git clone https://github.com/babelouest/ulfius.git
$ cd ulfius/src/
$ make
$ sudo make install
$ cd ../..

$ # Install Hoel
$ git clone https://github.com/babelouest/hoel.git
$ cd hoel/src/
$ make DISABLE_POSTGRESQL=1
$ sudo make install
$ cd ../..

$ git clone git@github.com:babelouest/taliesin.git
$ cd taliesin/src/
$ make && sudo make install
```

### Configuration

#### taliesin.conf file

Copy `taliesin.conf.sample` to `/usr/local/etc/taliesin/taliesin.conf`, edit the file `/usr/local/etc/taliesin/taliesin.conf` with your own settings.

Change the value `app_files_path` to your `taliesin/webapp` if necessary.

Also, change the value `server_remote_address` to a remote accessible url if you want to access Taliesin from different devices.

##### Data storage backend initialisation

You can use a MySql/MariaDB database or a SQLite3 database file.
Use the dedicated script, `taliesin.mariadb.sql` or `taliesin.sqlite3.sql` to initialize your database.

```shell
$ # Example to install the database with MariaDB
$ mysql taliesin < taliesin.mariadb.conf 
$ # Example to install the database with SQLite3
$ sqlite3 [path/to/taliesin.db] < taliesin.sqlite3.sql
```

##### Glewlwyd OAuth2 token validation

If you set the config parameter `use_oauth2_authentication` to true, you must set the configuration values to correspond with your OAuth2 Glewlwyd server. The Glewlwyd configuration block is labelled `jwt`.

In this block, you must set the value `use_rsa` to `true` if you use RSA signatures for the tokens, then specify the path to the RSA public key file in the value `rsa_pub_file`. If you use an ECDSA signature, set the parameter `use_ecdsa` to true and set the path to your ECDSA public key file. If you use `sha` digest as signature, set `use_sha` to `true`, then specify the secret used to encode the tokens in the value `sha_secret`.

#### webapp/config.json file

Copy `webapp/config.json.sample` to `webapp/config.json` and edit the file `webapp/config.json` with your own settings.

If you want to setup Taliesin without OAuth2, you can use have a `config.json` like this:

```javascript
{
  "taliesinApiUrl": "http://localhost:8576/api",
  "storageType": "local"
}
```

The web application is located in `webapp`, its source is located in `webapp-src`. Go to [webapp-src/README.md](https://github.com/babelouest/taliesin/blob/master/webapp-src/README.md) if you want more details on the front-end implementation.

You can either use Taliesin built-in static file server or host the web application in another place, e.g. an Apache or nginx instance.

By design, the web application must be accessible on a root path, e.g. `https://taliesin.mydomain.tld/`.

To configure the front-end, rename the file `webapp/config.json.sample` to `webapp/config.json` and modify its content for your configuration.

```javascript
{
  "taliesinApiUrl": "http://localhost:8576/api", // URL to your Taliesin API
  "angharadApiUrl": "http://localhost:2473/api", // URL to your Angharad API (optional)
  "storageType": "local",                        // Storage type to keep local config values like last player used, last stream or last data source
  "oauth2Config": {
    "storageType": "local",                      // local or cookie
    "responseType": "code",                      // code or implicit
    "serverUrl": "http://localhost:4593/api",    // URL to your Glewlwyd API
    "authUrl": "auth", 
    "tokenUrl": "token", 
    "clientId": "taliesin", 
    "redirectUri": "http://localhost:8576/",     // Url to your Taliesin front-end
    "scope": "taliesin taliesin_admin angharad g_profile"
    "profileUrl": "profile"
  }
}
```

#### Install service

The files `taliesin-init` (SysV init) and `taliesin.service` (Systemd) can be used to run taliesin as a daemon. They are fitted for a Raspbian distrbution, but can easily be changed for other systems.

##### Install as a SysV init daemon and run

```shell
$ sudo cp taliesin-init /etc/init.d/taliesin
$ sudo update-rc.d taliesin defaults
$ sudo service taliesin start
```

##### Install as a Systemd daemon and run

```shell
$ sudo cp taliesin.service /etc/systemd/system
$ sudo systemctl enable taliesin
$ sudo systemctl start taliesin
```

#### Setup Taliesin in your Glewlwyd Oauth2 server

If you use a [Glewlwyd](https://github.com/babelouest/glewlwyd) instance as Oauth2 server, you must setup a new client, don't forget to setup properly the new scope, here set to `taliesin` and `taliesin_admin`, the `client_id` and at least one correct `redirect_uri` value.

Taliesin front-end is a React JS application with Redux, it will need a non confidential client_id, and the authorization types `code` and/or `token`.

![glewlwyd client configuration](https://github.com/babelouest/taliesin/raw/master/docs/images/glewlwyd.png)

#### Protect behind Apache mod_proxy

A good practice consists to protect Taliesin behind a http proxy. This way you can add specific security rules, redirect to a standard TCP port, e.g. 443, etc.

Since Taliesin webapp must be executed from a root path (e.g. http://localhost:8576/ and not http://localhost:8576/app/), you must set the apache proxy to the root of a domain or sub-domain.

For the following exemple, it configuration configures a reverse-proxy and encapsulates the http connection in a secured https connection, better for the outside world, the apache server must have the modules `proxy_http.load` (mandatory) and `proxy_wstunnel.load` (optional) enabled:

```
<VirtualHost *:443>
        ServerName taliesin.my-domain.org

        SSLEngine on
        SSLCertificateFile    /path/to/cert.pem
        SSLCertificateKeyFile /path/to/privkey.pem

        ProxyPassMatch ^/api/stream/(.*)/ws$ ws://localhost:8576/api/stream/$1/ws
        ProxyPass / http://localhost:8576/ retry=0 connectiontimeout=30 timeout=300 nocanon

        ProxyPassReverse / http://localhost:8576/
</VirtualHost>
```

### Start the server

Run the application using the service command if you installed the init file:

```shell
$ sudo service taliesin start
```

You can also manually start the application like this:

```shell
$ ./taliesin --config-file=taliesin.conf
```

By default, Taliesin is available on TCP port 8576, the API is located at [http://localhost:8576/api](http://localhost:8576/api) and the web application is located at the url [http://localhost:8576/](http://localhost:8576/).
