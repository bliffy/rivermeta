#
# Top-level build for rivermeta
#

include src/oscheck.mk

BASE_DIR=.
SRC_ROOT=./src

include $(SRC_ROOT)/common.mk


.PHONY: all install uninstall clean scour

all: 
	@echo "Building SERIAL waterslide"
	@$(MAKE) --no-print-directory -C src
	@echo "Building PARALLEL waterslide"
	@$(MAKE) WS_PARALLEL=1 --no-print-directory -C src
	@echo "Rivermeta build completed"

install:
	$(MAKE) -C src install

uninstall:
	$(MAKE) -C src uninstall

clean:
	$(MAKE) --no-print-directory -C src clean
	$(RM) waterslide waterslide-parallel $(QUIETOUT)
	$(RM) wsalias wsman $(QUIETOUT)
ifndef ISWINDOWS
	$(RM) $(WS_LIB_DIR)/wsdt_* $(QUIETOUT)
	$(RM) $(WS_LIB_DIR)/libwaterslid*.so $(QUIETOUT)
	$(RM) $(WS_PROCS_DIR)/proc_*
else
	@if EXIST lib $(RM) lib\wsdt_*
	@if EXIST lib $(RM) lib\libwaterslide*.so
	@if EXIST procs $(RM) procs\proc_*
endif

scour: clean
	$(RM) $(WS_LIB_DIR) $(WS_PROCS_DIR) $(QUIETOUT)

