# Postgresql
ShadeWatcher supports storing/loading log data to/from a relational database (Postgresql).

## Setup
Install Postgresql on your database server.
```bash
sudo apt install postgresql postgresql-contrib
```

## Log in:
```bash
# start the db
service postgresql start

# Log in the db
sudo su postgres
psql -d postgres -U postgres -h 127.0.0.1

# Display schema
SELECT schema_name FROM information_schema.schemata; or /dn

# Set Schema
SET search_path to XX

# Delete Schema
DROP SCHEMA [IF EXISTS] schema_name 
```