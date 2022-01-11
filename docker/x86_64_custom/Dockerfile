FROM babelouest/taliesin_x86_64:latest

# Install required packages
RUN apk add --no-cache \
		sqlite

COPY ["taliesin.conf", "/etc/taliesin/"]
COPY ["config.json", "/usr/share/taliesin/webapp/"]
COPY ["entrypoint.sh", "/"]
COPY ["taliesin.sqlite3.sql", "/"]

EXPOSE 8576

CMD ["/entrypoint.sh"]
