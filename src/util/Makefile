
include ../oscheck.mk

ifndef ISWINDOWS
  BASE_DIR=../..
else
  BASE_DIR=..\..
endif
SRC_ROOT=..

include ../common.mk

LDFLAGS += -rdynamic -lstdc++

UTILS = wsman wsalias

ifdef WS_PARALLEL
UTILS += waterslide-parallel
else
UTILS += waterslide
endif


all: $(UTILS)

waterslide: waterslide.c $(WS_LIB_DIR)/libwaterslide.a
	$(SHOWFILE)
	$(CC) $(CFLAGS) $(PERF_FLAG) $< -o $@ -Wl,--whole-archive $(WS_LIB_DIR)/libwaterslide.a -Wl,--no-whole-archive $(LDFLAGS)
ifndef ISFREEBSD
	$(CP) -t $(WS_BIN_DIR) $@
else
	$(CP) $@ $(WS_BIN_DIR) 
endif


waterslide-parallel: waterslide-parallel.c $(WS_LIB_DIR)/libwaterslide-parallel.a $(HWLOC_LINK)
	$(SHOWFILE)
	$(CC) $(CFLAGS) $(PERF_FLAG) $< -o $@ -Wl,--whole-archive $(WS_LIB_DIR)/libwaterslide-parallel.a -Wl,--no-whole-archive $(LDFLAGS) $(HWLOC_LINK)
ifndef ISFREEBSD
	$(CP) -t $(WS_BIN_DIR) $@
else
	$(CP) $@ $(WS_BIN_DIR) 
endif


wsman: wsman.c wsman_color.h wsman_map.h wsman_util.h wsman_word_wrap.h
	$(SHOWFILE)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)
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

