@SETLOCAL
@SET PATHEXT=%PATHEXT:;.JS;=;%

SET "PATH=%~dp0tools\win32\binary\nodejs;%PATH%"

SET "NODE_EXE=%~dp0\tools\win32\binary\nodejs\node.exe"
IF NOT EXIST "%NODE_EXE%" (
  SET "NODE_EXE=node"
)

npm install && "%NODE_EXE%" "%~dp0\out\cli.js" installation %*