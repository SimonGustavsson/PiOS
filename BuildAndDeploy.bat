@echo off

SET DEPLOYERPATH=%1
SET KERNELPATH=%2

REM Deployer path is relative, make absolute
pushd %DEPLOYERPATH%
SET DEPLOYERPATH=%CD%
popd

REM Kernel path is relative, make absolute
pushd %KERNELPATH%
SET KERNELPATH=%CD%
popd

REM Build
c:/cygwin64/bin/bash.exe --login -c "cd f:;cd git/pios;make"

echo Deployer path: %DEPLOYERPATH%\PiOSDeployer.exe
echo Kernel path: %KERNELPATH%\kernel.img

REM And launch the deployer
REM %DEPLOYERPATH%\PiOSDeployer.exe %KERNELPATH%kernel.img

start "Deployer" %DEPLOYERPATH%\PiOSDeployer.exe %KERNELPATH%\kernel.img
echo Waiting for Deployer...
