
# catch for windows (Mingw32/64)
ifeq ($(OS),Windows_NT)
  OSNAME=WINDOWS
  ISWINDOWS=1
else # expect some form of posix
  OSNAME=$(word 1,$(shell uname))
  ifndef OSNAME
    OSNAME=UNKNOWN
  endif
endif

# special case for BSD
ifeq "$(OSNAME)" "FreeBSD"
  NODL=1
  ISFREEBSD=1
  OSNAME=LINUX
endif

