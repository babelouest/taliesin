FROM babelouest/taliesin_x86_64:latest

# Install required packages
RUN apk add --no-cache \
    sqlite

COPY ["entrypoint.sh", "taliesin.sqlite3.sql", "/"]
COPY ["taliesin.conf", "/var/taliesin/conf/"]
COPY ["config.json", "/var/www"]

EXPOSE 8576

CMD ["/entrypoint.sh"]
