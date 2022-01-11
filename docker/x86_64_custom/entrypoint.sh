#!/bin/bash

# If required, generate database file

if ! [ -f "/var/cache/taliesin/taliesin.db" ]
then
  sqlite3 /var/cache/taliesin/taliesin.db < /taliesin.sqlite3.sql
fi

# Run application
/usr/bin/taliesin --config-file=/etc/taliesin/taliesin.conf
