# Bash script to config auditbeat in Ubuntu 16.04

# !/bin/bash

folder=$1

echo "stop system auditing"
service auditbeat stop

echo "copying audit records from /var/log/auditbeat to ${folder}"
cp /var/log/auditbeat/* $folder

echo "change access permissions"
chmod -R 555 $folder
