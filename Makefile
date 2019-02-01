#
# Top-level build for rivermeta
#

BASE_DIR=.
SRC_ROOT=./src

include $(SRC_ROOT)/common.mk


.PHONY: all install uninstall clean scour

all: 
	@echo "Building SERIAL waterslide"
	@$(MAKE) --no-print-directory -C src
	@echo "Building PARALLEL waterslide"
	@$(MAKE) WS_PARALLEL=1 --no-print-directory -C src
	@echo "rivermeta build completed"

install:
	$(MAKE) -C src install

uninstall:
	$(MAKE) -C src uninstall

clean:
	$(MAKE) --no-print-directory -C src clean
	$(RM) waterslide waterslide-parallel $(QUIETOUT)
	$(RM) wsalias wsman $(QUIETOUT)
#	$(RM) $(WS_BIN_DIR)/* $(QUIETOUT)
	$(RM) $(WS_PROCS_DIR)/proc_* $(QUIETOUT)
	$(RM) $(WS_LIB_DIR)/wsdt_* $(QUIETOUT)
	$(RM) $(WS_LIB_DIR)/libwaterslid*.a $(QUIETOUT)

scour: clean
	$(RM) $(WS_LIB_DIR) $(WS_PROCS_DIR) $(QUIETOUT)

