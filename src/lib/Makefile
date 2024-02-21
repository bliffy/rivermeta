
include ../oscheck.mk

ifdef ISWINDOWS
  BASE_DIR=..\..
else
  BASE_DIR=../..
endif
SRC_ROOT=..

include ../common.mk

CFLAGS += $(PERF_FLAG) -fPIC
CXXFLAGS += $(PERF_FLAG) -fPIC

FILELIST =  $(patsubst %.c, %.o, $(wildcard *.c))
FILELIST += $(patsubst %.cc, %.o, $(wildcard *.cc))
FILELIST += wscalc.o wscalc.tab.o
FILELIST += graph.tab.o graph.lex.o

FILES = $(FILELIST)

ifdef WS_PARALLEL
  SUFFIX = -p
else
  SUFFIX = -s
endif

MODULES = $(sort $(patsubst %.o, %$(SUFFIX).o, $(FILES)))


all: $(MODULES) $(WS_CORE_LIB)

wsperf.c: 
	$(SHOWFILE)

# Created shared core library.
ifdef ISWINDOWS
# for windows
$(WS_CORE_LIB): $(MODULES)
	$(SHOWFILE)
	$(CXX) $(MODULES) -o $@ -shared $(LDFLAGS) $(WINSTATIC_CXX)
	$(CP) $@ $(WS_BIN_DIR)
else
# for linux
$(WS_CORE_LIB): $(MODULES)
	$(SHOWFILE)
	$(CXX) -shared $(MODULES) -o $@ $(LDFLAGS) $(RELATIVE_ORIGIN)
	$(CP) $@ $(WS_LIB_DIR)/
endif

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
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) *.so .*.swp *.dSYM ._* *.o $(QUIETOUT)
ifndef ISWINDOWS
	$(RM) shared/*.o $(QUIETOUT)
else
	@if EXIST shared $(RM) shared\*.o $(QUIETOUT)
endif
	$(RM) libwaterslide$(LIBEND) $(QUIETOUT)
	$(RM) libwaterslide-parallel$(LIBEND) $(QUIETOUT)

scour: clean
	$(RM) parse_graph.* graph.lex* graph.tab* $(QUIETOUT)
	$(RM) wscalc.c wscalc.tab* wscalc_int.c $(QUIETOUT)
	$(RM) wscalc_int.tab.c wscalc_int.tab.h $(QUIETOUT)

graph.tab$(SUFFIX).o: graph.tab.cc
graph.lex$(SUFFIX).o: graph.lex.cc ast.h
graphBuilder$(SUFFIX).o: graph.tab.hh ast.h
ast$(SUFFIX).o: ast.cc ast.h

