
# This section builds the individual modules for
# data processing. Separate bindings are created
# for serial and parallel.

include ../oscheck.mk

BASE_DIR=../..
SRC_ROOT=..

include ../common.mk

CFLAGS += $(PERF_FLAG)

# Switches for various build options

# HASMAGIC = 1	     # if you have magic.h
# NOPB = 1           # to skip the wsproto build
# NORE2 = 1          # to skip the re2 build

ifndef NOPB
  CFLAGS += -DPBMETA
endif
CFLAGS += -Wno-unknown-pragmas -Wno-strict-aliasing

ifdef ISWINDOWS
  LDFLAGS += -shared $(WINSTATIC_CXX)
else
  LDFLAGS += -shared $(RELATIVE_ORIGIN)
endif

ifdef WS_PARALLEL
  LDFLAGS += -lwaterslide-parallel
else
  LDFLAGS += -lwaterslide
endif

MODULES = $(patsubst %.c, $(WS_PROCS_DIR)/%$(WS_SFX), $(wildcard *.c))
MODULES += $(patsubst %.cc, $(WS_PROCS_DIR)/%$(WS_SFX), $(wildcard *.cc))


all: $(WS_PROCS_DIR) $(MODULES) 

$(WS_PROCS_DIR): 
	$(MKDIR) $(WS_PROCS_DIR)

###########################################

$(WS_PROCS_DIR)/proc_calc$(WS_SFX): proc_calc.c $(WS_INC_DIR)/wscalc.h

$(WS_PROCS_DIR)/proc_tcpcatch$(WS_SFX): proc_tcpcatch.c $(WS_INC_DIR)/tcp_rw.h
	$(SHOWFILE) 
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(WS_PROCS_DIR)/proc_tcpthrow$(WS_SFX): proc_tcpthrow.c $(WS_INC_DIR)/tcp_rw.h
	$(SHOWFILE) 
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

ifdef HASMAGIC
$(WS_PROCS_DIR)/proc_filemagic$(WS_SFX): proc_filemagic.c
	$(SHOWFILE) 
	$(CC) $(CFLAGS) $< -o $@ -lmagic $(LDFLAGS)
else
$(WS_PROCS_DIR)/proc_filemagic$(WS_SFX): proc_filemagic.c
	$(SKIPFILE)

endif

ifndef NOPB
  ifdef PBNONLOCAL
    PBFLAGS += -rdynamic -Wl,-rpath,`pkg-config protobuf --variable=libdir`
    PBLIB+= `pkg-config --cflags --libs protobuf-lite`
  else
    PBFLAGS += -rdynamic -I$(PBDIR)/include -pthread
    PBLIB+= -L$(PBDIR)/lib -lprotobuf-lite -pthread -lpthread
  endif

protobuf/wsserial.pb.cc: 
	$(QUIET)$(MAKE) --no-print-directory -C protobuf

$(WS_PROCS_DIR)/proc_wsproto_in$(WS_SFX):proc_wsproto_in.cc protobuf/wsserial.pb.cc protobuf/wsproto.pb.cc
	$(SHOWFILE) 
	$(CPP) $(PBFLAGS) $(CPPFLAGS) $^ -o $@ $(PBLIB) $(LDFLAGS)
$(WS_PROCS_DIR)/proc_wsproto_out$(WS_SFX): proc_wsproto_out.cc protobuf/wsserial.pb.cc protobuf/wsproto.pb.cc
	$(SHOWFILE) 
	$(CPP) $(PBFLAGS) $(CPPFLAGS) $^ -o $@ $(PBLIB) $(LDFLAGS)
$(WS_PROCS_DIR)/proc_lastn$(WS_SFX): proc_lastn.c protobuf/wsproto.pb.cc
	$(SHOWFILE) 
	$(CPP) $(PBFLAGS) $(CPPFLAGS) $^ -o $@ $(PBLIB) $(LDFLAGS)

else
$(WS_PROCS_DIR)/proc_lastn$(WS_SFX): proc_lastn.c
	$(SHOWFILE) 
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)
$(WS_PROCS_DIR)/proc_wsproto_in$(WS_SFX): proc_wsproto_in.cc
	$(SKIPFILE)
$(WS_PROCS_DIR)/proc_wsproto_out$(WS_SFX): proc_wsproto_out.cc
	$(SKIPFILE)
endif

ifndef NORE2
$(WS_PROCS_DIR)/proc_re2$(WS_SFX): proc_re2.cc
	$(SHOWFILE) 
	$(CPP) $(CPPFLAGS) -I$(RE2INCLUDE) $< -o $@ $(RE2LIB)/libre2.a $(LDFLAGS)
else
$(WS_PROCS_DIR)/proc_re2$(WS_SFX): proc_re2.cc
	$(SKIPFILE)
endif

ifdef HASDPU
DPUFLAGS += `pkg-config --cflags funnycar`
DPULIB   += `pkg-config --libs funnycar`
$(WS_PROCS_DIR)/proc_dpu$(WS_SFX): proc_dpu.c
	$(SHOWFILE)
	$(CC) $(CFLAGS) $(DPUFLAGS) $< -o $@ $(DPULIB) $(LDFLAGS)
else
$(WS_PROCS_DIR)/proc_dpu$(WS_SFX): proc_dpu.c
	$(SKIPFILE)
endif

ifdef HASRDKAFKA
$(WS_PROCS_DIR)/proc_kafka_in$(WS_SFX): proc_kafka_in.c
	$(SHOWFILE) 
	$(CC) $(CFLAGS) $< -o $@ -lrdkafka $(LDFLAGS)
$(WS_PROCS_DIR)/proc_kafka_multi_in$(WS_SFX): proc_kafka_multi_in.c
	$(SHOWFILE) 
	$(CC) $(CFLAGS) $< -o $@ -lrdkafka $(LDFLAGS)
else
$(WS_PROCS_DIR)/proc_kafka_in$(WS_SFX): proc_kafka_in.c
	$(SKIPFILE)
$(WS_PROCS_DIR)/proc_kafka_multi_in$(WS_SFX): proc_kafka_multi_in.c
	$(SKIPFILE)
endif

ifdef HASREDIS
$(WS_PROCS_DIR)/proc_redis$(WS_SFX): proc_redis.c
	$(SHOWFILE) 
	$(CC) $(CFLAGS) $< -o $@ -lhiredis $(LDFLAGS)
else
$(WS_PROCS_DIR)/proc_redis$(WS_SFX): proc_redis.c
	$(SKIPFILE)
endif


ifndef WS_PARALLEL
$(WS_PROCS_DIR)/proc_workbalance$(WS_SFX): proc_workbalance.c
	$(QUIET)echo "$@" $(QUIETOUT)
$(WS_PROCS_DIR)/proc_workreceive$(WS_SFX): proc_workreceive.c
	$(QUIET)echo "$@" $(QUIETOUT)
endif

#********
#*****
#********

# Default processing

%$(WS_SFX): %.c
	$(SHOWFILE)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)
$(WS_PROCS_DIR)/%$(WS_SFX): %.c
	$(SHOWFILE)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

%$(WS_SFX): %.cc
	$(SHOWFILE)
	$(CPP) $(CPPFLAGS) $< -o $@ $(LDFLAGS)
$(WS_PROCS_DIR)/%$(WS_SFX): %.cc
	$(SHOWFILE)
	$(CPP) $(CPPFLAGS) $< -o $@ $(LDFLAGS)

clean:
	$(RM) *.o .*.swp *.dSYM ._* *.8 *_so  $(QUIETOUT)
	$(QUIET)$(MAKE) --no-print-directory -C protobuf clean

