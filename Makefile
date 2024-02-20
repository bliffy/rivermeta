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
ifndef ISWINDOWS
	$(RM) $(WS_LIB_DIR)/wsdt_* $(QUIETOUT)
	$(RM) $(WS_LIB_DIR)/libwaterslid*.so $(QUIETOUT)
	$(RM) $(WS_PROCS_DIR)/proc_* $(QUIETOUT)
	$(RM) $(WS_BIN_DIR)/waterslide $(WS_BIN_DIR)/waterslide-parallel $(QUIETOUT)
	$(RM) $(WS_BIN_DIR)/wsalias $(WS_BIN_DIR)/wsman $(QUIETOUT)
else
	@if EXIST lib $(RM) $(WS_LIB_DIR)\wsdt_*
	@if EXIST lib $(RM) $(WS_LIB_DIR)\libwaterslide.*
	@if EXIST procs $(RM) $(WS_PROCS_DIR)\proc_*
	@if EXIST $(WS_BIN_DIR)\waterslide.exe $(RM) $(WS_BIN_DIR)\waterslide.exe
	@if EXIST $(WS_BIN_DIR)\waterslide-parallel.exe $(RM) $(WS_BIN_DIR)\waterslide-parallel.exe
	@if EXIST $(WS_BIN_DIR)\wsman.exe $(RM) $(WS_BIN_DIR)\wsman.exe
	@if EXIST $(WS_BIN_DIR)\wsalias.exe $(RM) $(WS_BIN_DIR)\wsalias.exe
endif

#scour: clean
#	$(RM) $(WS_LIB_DIR) $(WS_PROCS_DIR) $(QUIETOUT)

