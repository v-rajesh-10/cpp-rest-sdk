@echo off
set path=C:\Windows\System32
if exist %TEMP%\b2_msvc_14.0_vcvarsall_amd64.cmd del %TEMP%\b2_msvc_14.0_vcvarsall_amd64.cmd
if exist %TEMP%\b2_msvc_14.0_vcvarsall_x86.cmd del %TEMP%\b2_msvc_14.0_vcvarsall_x86.cmd
if exist %TEMP%\b2_msvc_14.0_vcvarsall_x86_arm.cmd del %TEMP%\b2_msvc_14.0_vcvarsall_x86_arm.cmd
b2.exe --stagedir=stage32 --with-date_time --with-log --with-regex --with-system address-model=32 runtime-link=static
