FROM babelouest/taliesin_x86_64:latest

# Install required packages
RUN apk add --no-cache \
    sqlite

COPY ["entrypoint.sh", "taliesin.sqlite3.sql", "/"]
COPY ["taliesin.conf.orig", "/var/taliesin/conf/taliesin.conf"]
COPY ["config.json.orig", "/var/www/config.json"]

EXPOSE 8576

CMD ["/entrypoint.sh"]
