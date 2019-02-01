
BASE_DIR=../..
SRC_ROOT=..

include $(SRC_ROOT)/common.mk


MODULES = $(patsubst %.c, %$(WS_SFX), $(wildcard wsdt_*.c))

CFLAGS += $(PERF_FLAG)
CFLAGS += -shared 

DESTDIR = $(WS_LIB_DIR)


all: $(DESTDIR) $(MODULES) 

$(DESTDIR):
	$(MKDIR) $(DESTDIR)

%$(WS_SFX): %.c
	$(SHOWFILE)
	$(CC) $(CFLAGS) $< -o $@
ifndef ISFREEBSD
	$(CP) -t $(DESTDIR) $@
else
	$(CP) $@ $(DESTDIR)
endif

clean:
	$(RM) .*.swp *.dSYM ._* *_so $(QUIETOUT)

