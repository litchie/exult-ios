# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to now our path anyway.
LPATH := audio/midi_drivers

LSRC := $(wildcard $(LPATH)/*.cc)
LPRODUCTS := 

midi_drivers_OBJ := $(patsubst %.cc,../%.o,$(filter %.cc,$(LSRC)))

# Common rules
include common.mk
