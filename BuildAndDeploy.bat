@echo off

if "%1"==""	GOTO INVALIDARGUMENTS
if "%2"==""	GOTO INVALIDARGUMENTS

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
echo Building...
c:/cygwin64/bin/bash.exe --login -c "cd f:;cd git/pios;make"

echo Deploying kernel...
start "Deployer" %DEPLOYERPATH%\PiOSDeployer.exe %KERNELPATH%\kernel.img
echo Waiting for Deployer...

GOTO DONE

:INVALIDARGUMENTS
echo Invalid arguments to deploy script, have you set up Visual Studio correctly?

:DONE
