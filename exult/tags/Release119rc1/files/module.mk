# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to now our path anyway.
LPATH := files

LSRC := $(wildcard $(LPATH)/*.cc)
LPRODUCTS := rwregress

rwregress_OBJ := rwregress.o

# Common rules
include common.mk
