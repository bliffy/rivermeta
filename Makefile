#
# Top-level build for waterslide
#
#   Invokes the next layer twice:  first for serial WS,
#   second for parallel WS
#

include ./src/common.mk

DIR_ERROR=0
ifeq "$(WS_BIN_DIR)" ""
  DIR_ERROR = 1
endif
ifeq "$(WS_LIB_DIR)" ""
  DIR_ERROR = 1
endif

.PHONY: all install uninstall clean
all: 
#	@export WS_HOME
	@echo "Building SERIAL waterslide"
	@unset WS_PARALLEL ; $(MAKE) --no-print-directory -C src
	@echo "Building PARALLEL waterslide"
	@WS_PARALLEL=1 $(MAKE) --no-print-directory -C src
	$(CP) bin/waterslide waterslide
	$(CP) bin/waterslide-parallel waterslide-parallel
	$(CP) bin/wsalias wsalias
	$(CP) bin/wsman wsman

	@echo "waterslide build completed"

install:
	$(MAKE) -C src install

uninstall:
	$(MAKE) -C src uninstall

clean:
	@unset WS_PARALLEL ; $(MAKE) --no-print-directory -C src clean
	$(RM) waterslide waterslide-parallel wsalias wsman
	$(RM) $(WS_BIN_DIR)/* $(WS_PROCS_DIR)/proc_*
	$(RM) $(WS_LIB_DIR)/wsdt_*
	$(RM) $(WS_LIB_DIR)/libwaterslid*.a

.PHONY: scour
scour: clean
	-$(RM) -r $(WS_LIB_DIR) $(WS_PROCS_DIR)

