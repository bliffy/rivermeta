
# General project Makefile contents

NORE2=1
NOHWLOC=1
NOPB=1

# catch for windows (Mingw32/64)
ifeq ($(OS),Windows_NT)
  NODL=1
  NORT=1
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

QUIET = @
ifdef ISWINDOWS
  QUIETOUT = 2>NUL 1>NUL
else
  QUIETOUT = > /dev/null 2>&1
endif

ifndef ISWINDOWS
  WS_BIN_DIR = $(BASE_DIR)
  WS_LIB_DIR = $(BASE_DIR)/lib
  WS_PROCS_DIR = $(BASE_DIR)/procs
  WS_INC_DIR = $(SRC_ROOT)/include
else
  WS_BIN_DIR = $(BASE_DIR)
  WS_LIB_DIR = $(BASE_DIR)\lib
  WS_PROCS_DIR = $(BASE_DIR)\procs
  WS_INC_DIR = $(SRC_ROOT)\include
endif

ifdef ISWINDOWS
  LIBEND:=.dll
else
  LIBEND:=.a
endif
ifdef WS_PARALLEL
  WS_CORE_LIB = libwaterslide-parallel$(LIBEND)
else
  WS_CORE_LIB = libwaterslide$(LIBEND)
endif

ifndef NOPB
  PBDIR = '$(WS_LIB_DIR)/protobuflib'
  PBBUILD = $(EXT_DIR)/pbtemp
  PBPKG = protobuf-2.5.0*
  PBEXE = $(QUIET)$(PBDIR)/bin/protoc
endif

ifndef NORE2
  RE2DIR = $(WS_LIB_DIR)/re2
  RE2BUILD = $(EXT_DIR)/re2temp
  RE2PKG = re2-20140304.tgz
  RE2INCLUDE = $(RE2DIR)/include
  RE2LIB = $(RE2DIR)/lib
endif

ifndef NOHWLOC
  HWLOC_VER = 1.9
  HWLOC_DIR = $(WS_LIB_DIR)/hwloc-$(HWLOC_VER)
  HWLOC_BUILDROOT = $(EXT_DIR)/hwlocBuildDir
  HWLOC_BUILDDIR = $(HWLOC_BUILDROOT)/hwloc-$(HWLOC_VER)
  HWLOC_PKG = hwloc-$(HWLOC_VER).tar.bz2
  HWLOC_INCLUDE = $(HWLOC_DIR)/include
  HWLOC_LIB = $(HWLOC_DIR)/lib
  HWLOC_LINK = $(HWLOC_LIB)/libhwloc_embedded.a
  HWLOC_CFLAGS = -I$(HWLOC_INCLUDE) -DUSE_HWLOC
endif

# Compiler settings

ifndef OPT_LEVEL
  OPT_LEVEL = -O0
endif

CFLAGS = $(OPT_LEVEL) -Wall -fpic -std=gnu99
ifdef GNU_DEBUG
  CFLAGS += -g
endif
CFLAGS += -D_GNU_SOURCE

WS_INCLUDES = -I$(WS_INC_DIR) -I$(SRC_ROOT) -I.

#TODO the sections thing might break linux
LDFLAGS += -L$(WS_LIB_DIR) -Wl,--no-gc-sections

LDFLAGS += -lm -lz -lpthread -m64
ifndef NODL
  LDFLAGS += -ldl
endif
ifndef NORT
  LDFLAGS += -lrt
endif

ifeq "$(OSNAME)" "Darwin"
  CFLAGS += -bundle -undefined dynamic_lookup
endif

# Tools
CC = $(QUIET)gcc
CPP = $(QUIET)g++
FLEX = $(QUIET)flex
BISON = $(QUIET)bison
CD = $(QUIET)cd
ifdef ISWINDOWS
  RM = $(QUIET)del /Q /S
  MKDIR = $(QUIET)mkdir
  CP = $(QUIET)copy /Y
  RMDIR = $(QUIET)rmdir /S /Q
else
  RM = $(QUIET)rm -f -r
  MKDIR = $(QUIET)mkdir -p
  CP = $(QUIET)cp
  ifdef ISFREEBSD
    RMDIR = -$(QUIET)rmdir 
  else
    RMDIR = $(QUIET)rmdir --ignore-fail-on-non-empty
  endif
endif

TAR = $(QUIET)tar

ifndef INSTALL_EXEC
  INSTALL_EXEC = install
endif
INSTALL = $(QUIET)$(INSTALL_EXEC)
INSTALL_DIR_CMD = $(INSTALL) -d

WSALIAS = $(QUIET)$(WS_BIN_DIR)/wsalias

SHOWFILE = $(QUIET)echo "    $@"
SKIPFILE = $(QUIET)echo "  Skipping $@..."

ifndef ISWINDOWS
  NOERROR = 2>/dev/null || true
endif

CFLAGS += $(WS_INCLUDES)

ifdef HASWSPERF
  PERF_FLAG += -DWS_PERF
endif
ifdef HASSQPERF
  PERF_FLAG += -DSQ_PERF
endif
ifdef HASLOCKDBG
  CFLAGS += -DWS_LOCK_DBG
endif

ATOMIC_STACK_STATE = -DUSE_ATOMICS=1

THREAD_DEFS = -DWS_PTHREADS -DUSE_MUTEX_HOMED_FREE_LIST=1 $(ATOMIC_STACK_STATE) $(HWLOC_CFLAGS)

WS_SFX = .ws_so
ifdef WS_PARALLEL
  CFLAGS += $(THREAD_DEFS)
  SH_FLAGS = -DSHARED_KID
  OWMR_FLAGS = -DOWMR_TABLES
  WS_SFX = .wsp_so
endif

CPPFLAGS = $(filter-out -std=gnu99, $(CFLAGS))

