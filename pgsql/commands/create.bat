@echo off
..\bin\psql.exe --dbname postgres --file %1 --username postgres
