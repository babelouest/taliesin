FROM alpine:latest

# Install required packages
RUN apk add --no-cache \
		libcurl \
		jansson \
		gcc \
		gnutls \
		libconfig \
		libgcrypt \
		libmicrohttpd \
		sqlite-libs \
		mariadb-connector-c \
		wget \
		ffmpeg-libs \
		zlib \
		bash

ARG TALIESIN_VERSION
ARG ALPINE_VERSION

COPY ["taliesin-full_${TALIESIN_VERSION}_alpine_${ALPINE_VERSION}_x86_64.tar.gz", "/opt"]

RUN cd /opt && \
		tar -xf taliesin-full_${TALIESIN_VERSION}_alpine_${ALPINE_VERSION}_x86_64.tar.gz && \
		rm taliesin-full_${TALIESIN_VERSION}_alpine_${ALPINE_VERSION}_x86_64.tar.gz && \
		tar xf liborcania_*.tar.gz && \
		mv liborcania_*/lib/lib*.so* /usr/lib && \
		tar xf libyder_*.tar.gz && \
		mv libyder_*/lib/lib*.so* /usr/lib && \
		tar xf libulfius_*.tar.gz && \
		mv libulfius_*/lib/lib*.so* /usr/lib && \
		tar xf libhoel_*.tar.gz && \
		mv libhoel_*/lib/lib*.so* /usr/lib && \
		tar xf librhonabwy_*.tar.gz && \
		mv librhonabwy_*/lib/lib*.so* /usr/lib && \
		tar xf libiddawc_*.tar.gz && \
		mv libiddawc_*/lib/lib*.so* /usr/lib && \
		tar xf taliesin_*.tar.gz && \
		mv taliesin_*/bin/taliesin /usr/bin && \
    mkdir -p /var/cache/taliesin/ /usr/share/taliesin/ /etc/taliesin/ && \
		mv taliesin_*/etc/taliesin/taliesin.conf /etc/taliesin/ && \
		mv taliesin_*/share/taliesin/webapp/ /usr/share/taliesin/ && \
		mv /usr/share/taliesin/webapp/config.json /etc/taliesin/ && \
		cp /etc/taliesin/config.json /usr/share/taliesin/ && \
		rm -rf *

COPY ["entrypoint.sh", "/"]

EXPOSE 8576

CMD ["/entrypoint.sh"]
