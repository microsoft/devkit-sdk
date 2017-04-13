@SETLOCAL
@SET PATHEXT=%PATHEXT:;.JS;=;%

SET "NODE_EXE=%~dp0\tools\win32\binary\nodejs\node.exe"
IF NOT EXIST "%NODE_EXE%" (
  SET "NODE_EXE=node"
)

SET "NPM=%~dp0\tools\win32\binary\nodejs\npm.cmd"
IF NOT EXIST "%NPM%" (
  SET "NPM=npm"
)

"%NPM%" install && "%NODE_EXE%" "%~dp0\out\cli.js" installation %*
