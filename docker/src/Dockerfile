FROM alpine:3.7

# Install required packages
RUN apk add --no-cache \
    autoconf \
		automake \
		libcurl \
    curl-dev \
    libconfig-dev \
    jansson-dev \
    gnutls-dev \
		libgcrypt-dev \
    libmicrohttpd-dev \
    sqlite-dev \
    libtool \
    make \
		gcc \
		g++ \
    wget \
		ffmpeg-dev \
    zlib-dev \
		bash

ARG LIBJWT_VERSION=1.9.0
ARG ORCANIA_VERSION=1.1.1
ARG YDER_VERSION=1.1.1
ARG ULFIUS_VERSION=2.2.4
ARG HOEL_VERSION=1.3.1
ARG TALIESIN_VERSION=1.0.10

# libtool and autoconf may be required, install them with 'sudo apt-get install libtool autoconf'
RUN cd /tmp && \
    wget https://github.com/benmcollins/libjwt/archive/v${LIBJWT_VERSION}.tar.gz && \
    tar -zxvf v${LIBJWT_VERSION}.tar.gz && \
    rm v${LIBJWT_VERSION}.tar.gz && \
    cd libjwt-${LIBJWT_VERSION}/ && \
    autoreconf -i && \
    ./configure --without-openssl && \
    make && \
    make install && \

# Install Orcania
    cd /tmp && \
    wget https://github.com/babelouest/orcania/archive/${ORCANIA_VERSION}.tar.gz && \
    tar -zxvf ${ORCANIA_VERSION}.tar.gz && \
    rm ${ORCANIA_VERSION}.tar.gz && \
    cd orcania-${ORCANIA_VERSION}/src/ && \
    make && \
		cp liborcania.so.${ORCANIA_VERSION} /usr/local/lib/ && \
		ln -s /usr/local/lib/liborcania.so.${ORCANIA_VERSION} /usr/local/lib/liborcania.so && \
		cp orcania.h /usr/local/include && \

# Install Yder
    cd /tmp && \
    wget https://github.com/babelouest/yder/archive/v${YDER_VERSION}.tar.gz && \
    tar -zxvf v${YDER_VERSION}.tar.gz && \
    rm v${YDER_VERSION}.tar.gz && \
    cd yder-${YDER_VERSION}/src/ && \
    make && \
		cp libyder.so.${YDER_VERSION} /usr/local/lib/ && \
		ln -s /usr/local/lib/libyder.so.${YDER_VERSION} /usr/local/lib/libyder.so && \
		cp yder.h /usr/local/include/ && \

# Install Hoel
    cd /tmp && \
    wget https://github.com/babelouest/hoel/archive/v${HOEL_VERSION}.tar.gz && \
    tar -zxvf v${HOEL_VERSION}.tar.gz && \
    rm v${HOEL_VERSION}.tar.gz && \
    cd hoel-${HOEL_VERSION}/src/ && \
    make DISABLE_MARIADB=1 DISABLE_POSTGRESQL=1 && \
		cp libhoel.so.${HOEL_VERSION} /usr/local/lib/ && \
		ln -s /usr/local/lib/libhoel.so.${HOEL_VERSION} /usr/local/lib/libhoel.so && \
		cp hoel.h /usr/local/include/ && \

# Install Ulfius
    cd /tmp && \
    wget https://github.com/babelouest/ulfius/archive/v${ULFIUS_VERSION}.tar.gz && \
    tar -zxvf v${ULFIUS_VERSION}.tar.gz && \
    rm v${ULFIUS_VERSION}.tar.gz && \
    cd ulfius-${ULFIUS_VERSION}/src/ && \
    make && \
		cp libulfius.so.${ULFIUS_VERSION} /usr/local/lib/ && \
		ln -s /usr/local/lib/libulfius.so.${ULFIUS_VERSION} /usr/local/lib/libulfius.so && \
		cp ulfius.h /usr/local/include && \

# Install Taliesin
    cd /tmp && \
    wget https://github.com/babelouest/taliesin/archive/v${TALIESIN_VERSION}.tar.gz && \
    tar -zxvf v${TALIESIN_VERSION}.tar.gz && \
    rm v${TALIESIN_VERSION}.tar.gz && \
    cd taliesin-${TALIESIN_VERSION}/src && \
    make debug && \
    make install && \
		cp -R ../webapp /var/www && \

# Clean /tmp folder
		rm -rf /tmp/libjwt-${LIBJWT_VERSION} /tmp/orcania-${ORCANIA_VERSION} /tmp/yder-${YDER_VERSION} /tmp/hoel-${HOEL_VERSION} /tmp/ulfius-${ULFIUS_VERSION} /tmp/taliesin-${TALIESIN_VERSION}

# Configuration required for shared objects
RUN mkdir -p /var/taliesin/conf && \
    mkdir -p /var/cache/taliesin/

WORKDIR /var/taliesin

COPY ["entrypoint.sh", "/"]

EXPOSE 8576

CMD ["/entrypoint.sh"]
