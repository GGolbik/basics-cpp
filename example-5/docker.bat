@echo on

:: "Debug" or empty for release build
SET SCRIPT_DIR=%~dp0

SET IMAGE_NAME=ggolbik/cpp-build
SET BUILD_DIR="%SCRIPT_DIR%build"

for /f %%i in ('docker images -q %IMAGE_NAME%') do SET VAR=%%i

IF %VAR% == "" (
  docker build --tag %IMAGE_NAME% %SCRIPT_DIR%/../
)

for /f %%i in ('docker run --interactive --tty --detach %IMAGE_NAME%') do SET CONTAINER_ID=%%i

MKDIR %BUILD_DIR%

docker cp %SCRIPT_DIR%/. %CONTAINER_ID%:/app

docker exec --interactive --tty %CONTAINER_ID% rm -r /app/build

docker exec --interactive --tty %CONTAINER_ID% bash /app/build.sh

docker exec --interactive --tty %CONTAINER_ID% bash /app/install.sh

docker cp %CONTAINER_ID%:/app/build/. %BUILD_DIR%

docker stop %CONTAINER_ID%

docker rm %CONTAINER_ID%

