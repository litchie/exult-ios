# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to now our path anyway.
LPATH := conf

LSRC := $(wildcard $(LPATH)/*.cc)
LPRODUCTS := confregress

CONF_COMMON := ../conf/Configuration.o ../conf/XMLEntity.o


confregress_OBJ := xmain.o $(CONF_COMMON) ../files/utils.o

# Common rules
include common.mk
