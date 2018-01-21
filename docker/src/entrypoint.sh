#!/bin/bash

# All backups will be copied in the folder /var/taliesin/host

# Backup files in /usr/local/lib
tar cvz /usr/local/lib/ /usr/local/bin/ /var/www/ -f /var/taliesin/host/taliesin.tar.gz
