
include ../oscheck.mk

ifndef ISWINDOWS
  BASE_DIR=../..
else
  BASE_DIR=..\..
endif
SRC_ROOT=..

include ../common.mk


MODULES = $(patsubst %.c, %$(WS_SFX), $(wildcard wsdt_*.c))

CFLAGS += $(PERF_FLAG)

CFLAGS += -shared
#CFLAGS += -Wl,--no-allow-shlib-undefined,--no-undefined

DESTDIR = $(WS_LIB_DIR)


all: $(DESTDIR) $(MODULES) 

$(DESTDIR):
	echo want to make $(DESTDIR)
	$(MKDIR) $(DESTDIR)

%$(WS_SFX): %.c
	$(SHOWFILE)
	$(CC) $(CFLAGS) $< -o $@ -L$(WS_LIB_DIR) -lwaterslide
	$(CP) $@ $(DESTDIR) $(QUIETOUT)

clean:
	$(RM) .*.swp *.dSYM ._* *_so $(QUIETOUT)

