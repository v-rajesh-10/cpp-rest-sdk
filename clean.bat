@echo off
pushd boost
rem call clean.bat
popd

if exist .vs rd /Q /S .vs

if exist Debug rd /Q /S Debug
if exist x32_VS_debug rd /Q /S x32_VS_debug
if exist x32_VS_release rd /Q /S x32_VS_release
if exist x64_VS_debug rd /Q /S x64_VS_debug
if exist x64_VS_release rd /Q /S x64_VS_release

if exist client\Debug rd /Q /S client\Debug
if exist client\x32_VS_debug rd /Q /S client\x32_VS_debug
if exist client\x32_VS_release rd /Q /S client\x32_VS_release
if exist client\x64_VS_debug rd /Q /S client\x64_VS_debug
if exist client\x64_VS_release rd /Q /S client\x64_VS_release

if exist common\Debug rd /Q /S common\Debug
if exist common\x32_VS_debug rd /Q /S common\x32_VS_debug
if exist common\x32_VS_release rd /Q /S common\x32_VS_release
if exist common\x64_VS_debug rd /Q /S common\x64_VS_debug
if exist common\x64_VS_release rd /Q /S common\x64_VS_release

if exist server\Debug rd /Q /S server\Debug
if exist server\x32_VS_debug rd /Q /S server\x32_VS_debug
if exist server\x32_VS_release rd /Q /S server\x32_VS_release
if exist server\x64_VS_debug rd /Q /S server\x64_VS_debug
if exist server\x64_VS_release rd /Q /S server\x64_VS_release

if exist tests\Debug rd /Q /S tests\Debug
if exist tests\x32_VS_debug rd /Q /S tests\x32_VS_debug
if exist tests\x32_VS_release rd /Q /S tests\x32_VS_release
if exist tests\x64_VS_debug rd /Q /S tests\x64_VS_debug
if exist tests\x64_VS_release rd /Q /S tests\x64_VS_release

if exist Monitoring.VC.db del Monitoring.VC.db
