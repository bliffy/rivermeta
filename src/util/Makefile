
include ../oscheck.mk

ifndef ISWINDOWS
  BASE_DIR=../..
else
  BASE_DIR=..\..
endif
SRC_ROOT=..

include ../common.mk

UTILS = wsman wsalias

ifndef ISWINDOWS
  LDFLAGS += -rdynamic -lstdc++
else
  #TODO replace rpath
  LDFLAGS += -lstdc++ -Wl,-rpath=lib/
endif

ifdef WS_PARALLEL
  UTILS += waterslide-parallel
else
  UTILS += waterslide
endif


all: $(UTILS)

waterslide: waterslide.c $(WS_LIB_DIR)/$(WS_CORE_LIB)
	$(SHOWFILE)
ifdef ISWINDOWS
	$(CC) $(CFLAGS) $(PERF_FLAG) $< -o $@ $(LDFLAGS) -lwaterslide
else
	$(CC) $(CFLAGS) $(PERF_FLAG) $< -o $@ -Wl,--whole-archive -lwaterslide -Wl,--no-whole-archive $(LDFLAGS)
endif
	$(CP) $@ $(WS_BIN_DIR) $(QUIETOUT)


waterslide-parallel: waterslide-parallel.c $(WS_LIB_DIR)/$(WS_CORE_LIB) $(HWLOC_LINK)
	$(SHOWFILE)
ifdef ISWINDOWS
	$(CC) $(CFLAGS) $(PERF_FLAG) $< -o $@ $(LDFLAGS) $(HWLOC_LINK) -lwaterslide-parallel
else
	$(CC) $(CFLAGS) $(PERF_FLAG) $< -o $@ -Wl,--whole-archive -lwaterslide-parallel -Wl,--no-whole-archive $(LDFLAGS)
endif
	$(CP) $@ $(WS_BIN_DIR) $(QUIETOUT)


wsman: wsman.c wsman_color.h wsman_map.h wsman_util.h wsman_word_wrap.h $(WS_LIB_DIR)/$(WS_CORE_LIB)
	$(SHOWFILE)
ifdef ISWINDOWS
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) -lwaterslide
else
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)
endif
	$(CP) $@ $(WS_BIN_DIR) $(QUIETOUT)


%: %.c
	$(SHOWFILE)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)
	$(CP) $@ $(WS_BIN_DIR) $(QUIETOUT)


clean:
	$(RM) *.o $(UTILS) $(QUIETOUT)

