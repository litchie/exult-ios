# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to now our path anyway.
LPATH := audio

LSRC := $(wildcard $(LPATH)/*.cc)
LPRODUCTS := 

#u7audiotool_SRC := u7audiotool.o

# Common rules
#include common.mk
