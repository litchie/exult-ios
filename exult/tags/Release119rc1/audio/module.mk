# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to now our path anyway.
LPATH := audio

LSRC := $(wildcard $(LPATH)/*.cc)
LPRODUCTS := 

#u7audiotool_OBJ := u7audiotool.o

#midi_drivers_SRC := $(wildcard $(LPATH)/midi_drivers/*.cc)
#midi_drivers_OBJ := $(patsubst %.cc,../%.o,$(filter %.cc,$(midi_drivers_SRC)))

# Common rules
include common.mk
