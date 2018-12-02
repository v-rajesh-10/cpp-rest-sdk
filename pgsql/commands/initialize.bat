@echo off
set TZ=UTC
..\bin\initdb.exe --auth md5 --encoding UTF8 --locale C --pgdata ..\data --pwfile password.txt --username postgres
