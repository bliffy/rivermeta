
include ../oscheck.mk

ifndef WINDOWS
  BASE_DIR=../..
else
  BASE_DIR=..\..
endif
SRC_ROOT=..

include ../common.mk

CFLAGS += $(PERF_FLAG)

CFLAGS += -fPIC
CPPFLAGS += -fPIC

FILELIST =  $(patsubst %.c, %.o, $(wildcard *.c))
FILELIST += $(patsubst %.cc, %.o, $(wildcard *.cc))
FILELIST += wscalc.o wscalc.tab.o
FILELIST += graph.tab.o graph.lex.o

FILES = $(FILELIST)

# NOTE: These 'SUFFIX' are just tagged onto the filenames
ifdef WS_PARALLEL
  SUFFIX = -p
  WSARCHIVEFILE = libwaterslide-parallel.so
else
  SUFFIX = -s
  WSARCHIVEFILE = libwaterslide.so
endif

MODULES = $(sort $(patsubst %.o, %$(SUFFIX).o, $(FILES)))


all: $(MODULES) $(WSARCHIVEFILE)

wsperf.c: 
	$(SHOWFILE)

$(WSARCHIVEFILE): $(MODULES)
	$(SHOWFILE)
#	$(RM) $(WSARCHIVEFILE)  # (ar appends)
#	$(AR) rcs $@ $(MODULES)
	$(CC) -shared $(MODULES) -o $@ $(LDFLAGS) -lstdc++
	$(CP) $@ $(WS_LIB_DIR) 

graph.lex.cc: graph.l graph.tab.hh
	$(SHOWFILE)
	$(FLEX) -Ppg -ograph.lex.cc graph.l

graph.tab.cc graph.tab.hh: graph.y
	$(SHOWFILE)
	$(BISON) $(BISON_FLAGS) -d -ppg graph.y -o graph.tab.cc

wscalc.c: wscalc.l wscalc.tab.h $(WS_INC_DIR)/wscalc.h
	$(SHOWFILE)
	$(FLEX) -owscalc.c -Pwscalc wscalc.l

wscalc.tab.c wscalc.tab.h: wscalc.y $(WS_INC_DIR)/wscalc.h
	$(SHOWFILE)
	$(BISON) -d -pwscalc wscalc.y -o wscalc.tab.c

%$(SUFFIX).o: %.c
	$(SHOWFILE)
	$(CC) $(CFLAGS) -c $< -o $@

%$(SUFFIX).o: %.cc
	$(SHOWFILE)
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean:
	$(RM) *.so .*.swp *.dSYM ._* *.o $(QUIETOUT)
ifndef ISWINDOWS
	$(RM) shared/*.o $(QUIETOUT)
else
	@if EXIST shared $(RM) shared\*.o $(QUIETOUT)
endif
	$(RM) parse_graph.* graph.lex* graph.tab* $(QUIETOUT)
	$(RM) wscalc.c wscalc.tab* wscalc_int.c $(QUIETOUT)
	$(RM) wscalc_int.tab.c wscalc_int.tab.h $(QUIETOUT)
	$(RM) libwaterslide.dylib libwaterslide.a $(QUIETOUT)
	$(RM) libwaterslide-parallel.a $(QUIETOUT)

graph.tab$(SUFFIX).o: graph.tab.cc
graph.lex$(SUFFIX).o: graph.lex.cc ast.h
graphBuilder$(SUFFIX).o: graph.tab.hh ast.h
ast$(SUFFIX).o: ast.cc ast.h

