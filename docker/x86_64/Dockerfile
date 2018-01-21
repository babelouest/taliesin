FROM alpine:3.7

# Install required packages
RUN apk add --no-cache \
		libcurl \
    libconfig \
    jansson \
    gnutls \
		libgcrypt \
    libmicrohttpd \
    sqlite-libs \
    libtool \
		ffmpeg-libs \
    zlib

COPY ["taliesin.tar.gz", "/tmp"]

# Configuration required for shared objects
RUN tar xvf /tmp/taliesin.tar.gz -C /
