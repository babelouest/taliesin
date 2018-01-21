FROM babelouest/taliesin_x86_64:latest

# Install required packages
RUN apk add --no-cache \
    sqlite

COPY ["taliesin.conf", "/var/taliesin/conf/"]
COPY ["config.json", "/var/www"]
COPY ["entrypoint.sh", "/"]
#COPY ["taliesin.sqlite3.sql", "/"]
#COPY ["taliesin.mariadb.sql", "/"]
#COPY ["oauth-key.pem", "/var/taliesin/conf/"]

EXPOSE 8576

CMD ["/entrypoint.sh"]
