#/bin/sh
set -e
USER=pi
GROUP=pi
ORCANIA_VERSION=1.1.1
YDER_VERSION=1.1.1
ULFIUS_VERSION=2.2.4
HOEL_VERSION=1.3.1
CARLEON_VERSION=1.1.0

echo "###############"
echo "Install dependencies packages"
echo "###############"
apt update
apt upgrade -y
apt install -y libjansson-dev libavfilter-dev libavcodec-dev libavformat-dev libavresample-dev libavutil-dev libcurl4-gnutls-dev libsqlite3-dev libconfig8-dev libmpdclient-dev libgnutls28-dev libgcrypt11-dev wget sqlite3

echo "###############"
echo "Install libmicrohttpd"
echo "###############"
wget https://ftp.gnu.org/gnu/libmicrohttpd/libmicrohttpd-0.9.58.tar.gz
tar xvf libmicrohttpd-0.9.58.tar.gz
cd libmicrohttpd-0.9.58 && ./configure
make
make install
cd .. && rm -rf libmicrohttpd-0.9.58*

echo "###############"
echo "Install Orcania"
echo "###############"
wget https://github.com/babelouest/orcania/archive/v$ORCANIA_VERSION.tar.gz
tar xvf v$ORCANIA_VERSION.tar.gz
cd orcania-$ORCANIA_VERSION/src
make
make install
cd ../.. && rm -rf orcania-$ORCANIA_VERSION v$ORCANIA_VERSION.tar.gz

echo "###############"
echo "Install Yder"
echo "###############"
wget https://github.com/babelouest/yder/archive/v$YDER_VERSION.tar.gz
tar xvf v$YDER_VERSION.tar.gz
cd yder-$YDER_VERSION/src
make
make install
cd ../.. && rm -rf yder-$YDER_VERSION xvf v$YDER_VERSION.tar.gz

echo "###############"
echo "Install Ulfius"
echo "###############"
wget https://github.com/babelouest/ulfius/archive/v$ULFIUS_VERSION.tar.gz
tar xvf v$ULFIUS_VERSION.tar.gz
cd ulfius-$ULFIUS_VERSION/src
make
make install
cd ../.. && rm -rf ulfius-$ULFIUS_VERSION xvf v$ULFIUS_VERSION.tar.gz

echo "###############"
echo "Install Hoel"
echo "###############"
wget https://github.com/babelouest/hoel/archive/v$HOEL_VERSION.tar.gz
tar xvf v$HOEL_VERSION.tar.gz
cd hoel-$HOEL_VERSION/src
make DISABLE_POSTGRESQL=1 DISABLE_MARIADB=1
make install
cd ../.. && rm -rf hoel-$HOEL_VERSION xvf v$HOEL_VERSION.tar.gz

echo "###############"
echo "Install Taliesin"
echo "###############"
cd ../../src
make DISABLE_OAUTH2=1
make install clean DISABLE_OAUTH2=1
cd ../docs/minimal

echo "###############"
echo "Configure Taliesin"
echo "###############"
mkdir -p /usr/local/etc/taliesin /var/cache/taliesin /var/www/taliesin
chown $USER:$GROUP /var/cache/taliesin
cp taliesin.conf /usr/local/etc/taliesin
cp ../../taliesin.service /etc/systemd/system
su -c "sqlite3 /var/cache/taliesin/taliesin.db < ../../taliesin.sqlite3.sql" $USER
cp -R ../../webapp/* /var/www/taliesin/
cp config.json /var/www/taliesin/
systemctl enable taliesin
service taliesin start

if [ "$1" = "-with-mpd" ];
then
	echo "###############"
	echo "Install Carleon"
	echo "###############"
	apt install -y mpd
  wget https://github.com/babelouest/carleon/archive/v$CARLEON_VERSION.tar.gz
  tar xvf v$CARLEON_VERSION.tar.gz
	cd carleon-$CARLEON_VERSION/service-modules/
	make libservicempd.so
	cd ..
	make release-standalone install-standalone
	cd ..
  rm -rf carleon-$CARLEON_VERSION v$CARLEON_VERSION.tar.gz
  
	echo "###############"
	echo "Configure Carleon"
	echo "###############"
	mkdir -p /usr/local/etc/carleon /var/cache/carleon
	cp carleon.conf /usr/local/etc/carleon
	cp carleon.service /etc/systemd/system
	su -c "sqlite3 /var/cache/taliesin/carleon.db < carleon.sqlite3.sql" $USER
	su -c "sqlite3 /var/cache/taliesin/taliesin.db < taliesin-init.sqlite3.sql" $USER
	systemctl enable carleon
	service carleon start
fi

echo "Installation complete, visit the page http://localhost:8576 to start using Taliesin"
