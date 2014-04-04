REM Store transform tool in variable so we can modify it
SET "transformtool=%~dp0..\tools\gnu2msdev.exe"

REM Turn backslashes into forward slashes that bash understand
SET "transformtool=%transformtool:\=/%

REM Change C:/ into /cygdrive/c
SET "transformtool=%transformtool:c:=/cygdrive/c%"

REM Call make inside bash to compile our code, redirecting the output via gnu2msdev for
REM Pretty, clickable error messages in Visual Studio
echo c:/cygwin64/bin/bash.exe --login -c "cd /cygdrive/c/git/PiOS/ && make %1 1>&2 %transformtool%"
c:/cygwin64/bin/bash.exe --login -c "cd /cygdrive/c/git/PiOS/ && make %1 2<&1 | sed -e 's|/cygdrive/\([a-z]\)/|\1:/|' -e 's/\.\([ch]\):\([0-9]*\)/.\1 (\2)/' ; exit ${PIPESTATUS[0]}