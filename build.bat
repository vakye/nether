
@echo off

if not exist build mkdir build

set CompileFlags=/nologo /FC /Zi /Od /Oi /std:c11 /GS- /Gs999999 /W4 /WX /wd4101 /wd4100 /wd4189
set LinkFlags=/incremental:no /opt:icf /opt:ref /subsystem:windows /nodefaultlib kernel32.lib

pushd build
call cl %CompileFlags% /Fe:nether.exe "..\code\win32_nether.c" /link %LinkFlags%
popd
