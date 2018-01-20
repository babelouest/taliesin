# Taliesin minimal automatic installation

This folder contains an `install.sh` script that will install a *non-secure* installation of [Taliesin](https://github.com/babelouest/taliesin) and [Carleon](https://github.com/babelouest/carleon) (if specified) on the device, the database backed is SQLite3, there is no authentication and no SSL support.

## WARNING

This is not a universal installer! This script is to install Taliesin and control a local MPD, on a fresh Raspbian Stretch installed on a Raspberry PI. It can be adapted to other distibutions or specific installations but th script `install.sh` must be changed then.

A complete documentation on how to install Taliesin with all options and configurations detailed can be found at [INSTALL.md](https://github.com/babelouest/taliesin/blob/master/docs/INSTALL.md)

## Run automatic install

Run the script `install.sh` as root:

```shell
$ sudo ./install.sh
```

### Install with a local MPD instance configured

Add the option `-with-mpd` to the install command:

```shell
$ sudo ./install.sh -with-mpd
```

### Post-installation

Taliesin should be available at the following address [http://localhost:8576/](http://localhost:8576/). If you want to access Taliesin from another device, you MUST change the following files `/var/www/taliesin/config.json` and `/usr/local/etc/taliesin/taliesin.conf`. You must chenge the address `localhost` to the address of the device in your network. For exampe, if your device is called `taliesin` in your networkm follow these instructions.

#### /var/www/taliesin/config.json

Edit the file `/var/www/taliesin/config.json` and change both lines where are set the values `taliesinApiUrl` and `angharadApiUrl` (if you have installed MPD on the device). The result should look like:

```javascript
{
	"taliesinApiUrl": "http://taliesin:8576/api",
	"angharadApiUrl": "http://taliesin:2756/api",
	"storageType": "local", 
	"useWebsocket": true
}
```

#### /usr/local/etc/taliesin/taliesin.conf

Edit the vile `/usr/local/etc/taliesin/taliesin.conf` and change the settings `server_remote_address` with your device name. The result should look like:

```
server_remote_address = "http://taliesin:8576/"
```

Then open the address [http://taliesin:8576/](http://taliesin:8576/) in your browser to start using Talesin.
