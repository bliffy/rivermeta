
include ../oscheck.mk

ifndef ISWINDOWS
  BASE_DIR=../..
else
  BASE_DIR=..\..
endif
SRC_ROOT=..

include ../common.mk

#LDFLAGS += -rdynamic -lstdc++
# TODO: fix rpath somehow
LDFLAGS += -lstdc++ -Wl,-rpath=lib/

UTILS = wsman wsalias

ifdef WS_PARALLEL
UTILS += waterslide-parallel
WSCORELIB = -lwaterslide-parallel
else
UTILS += waterslide
WSCORELIB = -lwaterslide
endif


all: $(UTILS)

waterslide: waterslide.c $(WS_LIB_DIR)/libwaterslide.so
	$(SHOWFILE)
	$(CC) $(CFLAGS) $(PERF_FLAG) $< -o $@ $(LDFLAGS) $(WSCORELIB)
ifndef ISFREEBSD
	$(CP) -t $(WS_BIN_DIR) $@
else
	$(CP) $@ $(WS_BIN_DIR) 
endif


waterslide-parallel: waterslide-parallel.c $(WS_LIB_DIR)/libwaterslide-parallel.so $(HWLOC_LINK)
	$(SHOWFILE)
	$(CC) $(CFLAGS) $(PERF_FLAG) $< -o $@ $(LDFLAGS) $(HWLOC_LINK) $(WSCORELIB)
ifndef ISFREEBSD
	$(CP) -t $(WS_BIN_DIR) $@
else
	$(CP) $@ $(WS_BIN_DIR) 
endif


wsman: wsman.c wsman_color.h wsman_map.h wsman_util.h wsman_word_wrap.h $(WS_LIB_DIR)/libwaterslide.so
	$(SHOWFILE)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(WSCORELIB)
ifndef ISFREEBSD
	$(CP) -t $(WS_BIN_DIR) $@
else
	$(CP) $@ $(WS_BIN_DIR)
endif


%: %.c
	$(SHOWFILE)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)
ifndef ISFREEBSD
	$(CP) -t $(WS_BIN_DIR) $@
else
	$(CP) $@ $(WS_BIN_DIR) 
endif


clean:
	$(RM) *.o $(UTILS) $(QUIETOUT)

