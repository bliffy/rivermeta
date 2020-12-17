
include ../oscheck.mk

ifdef ISWINDOWS
  BASE_DIR=..\..
else
  BASE_DIR=../..
endif
SRC_ROOT=..

include ../common.mk

UTILS = wsman$(EXT_EXEC) #wsalias

LDFLAGS += -L$(WS_LIB_DIR)
ifdef ISWINDOWS
  LDFLAGS += -lwaterslide $(WINSTATIC_CXX)
else
 # -rdynamic
  LDFLAGS += -lwaterslide -lstdc++ $(RELATIVE_ORIGIN)
endif

ifdef WS_PARALLEL
  UTILS += waterslide-parallel$(EXT_EXEC)
else
  UTILS += waterslide$(EXT_EXEC)
endif


all: $(UTILS)

waterslide$(EXT_EXEC): waterslide.c $(WS_LIB_DIR)/$(WS_CORE_LIB)
	$(SHOWFILE)
	$(CC) $(CFLAGS) $(PERF_FLAG) $< -o $@ $(LDFLAGS)
	$(CP) $@ $(WS_BIN_DIR) $(QUIETOUT)

waterslide-parallel$(EXT_EXEC): waterslide-parallel.c $(WS_LIB_DIR)/$(WS_CORE_LIB) $(HWLOC_LINK)
	$(SHOWFILE)
	$(CC) $(CFLAGS) $(PERF_FLAG) $< -o $@ $(LDFLAGS)
	$(CP) $@ $(WS_BIN_DIR) $(QUIETOUT)

wsman$(EXT_EXEC): wsman.c wsman_color.h wsman_map.h wsman_util.h wsman_word_wrap.h $(WS_LIB_DIR)/$(WS_CORE_LIB)
	$(SHOWFILE)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)
	$(CP) $@ $(WS_BIN_DIR) $(QUIETOUT)

#%: %.c
#	$(SHOWFILE)
#	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)
#	$(CP) $@ $(WS_BIN_DIR) $(QUIETOUT)


clean:
	$(RM) *.o $(UTILS) $(QUIETOUT)

