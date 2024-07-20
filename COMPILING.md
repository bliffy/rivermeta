# Compiling RiverMeta

RiverMeta can be compiled for Linux or Windows x64.<br>
It is compiled and tested using the following methods.

## For Linux

Tested using WSL [**Windows Ubuntu Kernel**](https://ubuntu.com/desktop/wsl).

Simply run 'make'. Autoconf script is not currently included.

## For Windows

> *_NOTE:_* 
You do **not** need Cygwin or Bash-for-Windows.<br>

Compiled using [**MingW64**](https://www.mingw-w64.org/).<br>
Add your MingW64 installation's bin folder to your PATH.<br>
You may need to create an alias for MingW's 'make'.<br>
Here is an example batch-file script to create a friendly make console. Adjust to match your installation path and distribution.

```
@echo off
rem Directory to find mingw commands, such as g++.
set mingwDir=C:\Program Files\mingw-w64\x86_64-13.2.0-release-posix-seh-msvcrt-rt_v11-rev1\mingw64\bin        
echo MingW-w64 Path set.
rem Alias for make.
doskey make=mingw32-make $*
path %PATH%;%mingwDir%
call cmd.exe
```

Then, to compile, simply run 'make'.
