
include ../../oscheck.mk

ifndef ISWINDOWS
  BASE_DIR=../../..
  SRC_ROOT=../..
else
  BASE_DIR=..\..\..
  SRC_ROOT=..\..
endif

include $(SRC_ROOT)/common.mk

# Makefile for waterslide/waterslide-threads build
# All C files as sources, and chop off the .c for targets
SOURCES = $(wildcard *.proto)
MODULES = $(patsubst %.proto, %.pb.cc, $(SOURCES))

all: $(MODULES)

# All targets without an extension depend on their .c files
%.pb.cc: %.proto 
	@echo " building protoc    $<"
	$(PBEXE) --cpp_out=./. $<

clean:
	$(RM) *.pb.cc *.pb.h  $(QUIETOUT)

