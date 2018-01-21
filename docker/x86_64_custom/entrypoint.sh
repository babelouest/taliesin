#!/bin/bash

# If required, generate config file

if ! [ -f "/var/cache/taliesin/taliesin.db" ]
then
  sqlite3 /var/cache/taliesin/taliesin.db < /taliesin.sqlite3.sql
fi

# Run application
/usr/local/bin/taliesin --config-file=/var/taliesin/conf/taliesin.conf
