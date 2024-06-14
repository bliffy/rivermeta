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
	$(RM) waterslide$(EXT_EXEC) $(QUIETOUT)
	$(RM) waterslide-parallel$(EXT_EXEC) $(QUIETOUT)
	$(RM) wsalias$(EXT_EXEC) wsman$(EXT_EXEC) $(QUIETOUT)
ifndef ISWINDOWS
	$(RM) $(WS_DATATYPES_DIR)/wsdt_* $(QUIETOUT)
	$(RM) $(WS_LIB_DIR)/libwaterslid*.so $(QUIETOUT)
	#$(RM) $(WS_PROCS_DIR)/proc_*
else
	@if EXIST $(WS_DATATYPES_DIR)\wsdt_* $(RM) $(WS_DATATYPES_DIR)\wsdt_*
	@if EXIST $(WS_LIB_DIR)\*.dll $(RM) $(WS_LIB_DIR)\libwaterslide*.dll
	@if EXIST $(WS_PROCS_DIR)\proc_* $(RM) $(WS_PROCS_DIR)\proc_*
endif

#scour: clean
#	$(RM) $(WS_LIB_DIR) $(WS_PROCS_DIR) $(QUIETOUT)

