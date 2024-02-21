
include ../oscheck.mk

ifdef ISWINDOWS
  BASE_DIR=..\..
else
  BASE_DIR=../..
endif
SRC_ROOT=..

include ../common.mk

UTILS = wsman$(EXT_EXEC) #wsalias

#LDFLAGS += -L$(WS_LIB_DIR)
ifdef ISWINDOWS
  RTLIB_DIR = $(WS_BIN_DIR)
  LDFLAGS += -L$(RTLIB_DIR) $(WINSTATIC_CXX)
else
 # -rdynamic
  RTLIB_DIR = $(WS_LIB_DIR)
  LDFLAGS += -L$(RTLIB_DIR) -lstdc++ $(RELATIVE_ORIGIN)
endif

ifdef WS_PARALLEL
  UTILS += waterslide-parallel$(EXT_EXEC)
  LDFLAGS += -lwaterslide-parallel
else
  UTILS += waterslide$(EXT_EXEC)
  LDFLAGS += -lwaterslide
endif

all: $(UTILS)

waterslide$(EXT_EXEC): waterslide.c $(RTLIB_DIR)/$(WS_CORE_LIB)
	$(SHOWFILE)
	$(CC) $(CFLAGS) $(PERF_FLAG) $< -o $@ $(LDFLAGS)
	$(CP) $@ $(WS_BIN_DIR) $(QUIETOUT)

waterslide-parallel$(EXT_EXEC): waterslide-parallel.c $(RTLIB_DIR)/$(WS_CORE_LIB) $(HWLOC_LINK)
	$(SHOWFILE)
	$(CC) $(CFLAGS) $(PERF_FLAG) $< -o $@ $(LDFLAGS)
	$(CP) $@ $(WS_BIN_DIR) $(QUIETOUT)

wsman$(EXT_EXEC): wsman.c wsman_color.h wsman_map.h wsman_util.h wsman_word_wrap.h $(RTLIB_DIR)/$(WS_CORE_LIB)
	$(SHOWFILE)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)
	$(CP) $@ $(WS_BIN_DIR) $(QUIETOUT)

#%: %.c
#	$(SHOWFILE)
#	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)
#	$(CP) $@ $(WS_BIN_DIR) $(QUIETOUT)


clean:
	$(RM) *.o $(UTILS) $(QUIETOUT)

