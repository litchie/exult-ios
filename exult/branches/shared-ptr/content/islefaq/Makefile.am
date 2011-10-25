# This is an automatically generated file; please do not edit it manually.
# Instead, run makefile_builder.sh from the parent directory.

# Base of the exult source
SRC=../..

UCCDIR=$(SRC)/usecode/compiler
UCC=$(UCCDIR)/ucc

EXPACKDIR=$(SRC)/tools
EXPACK=$(EXPACKDIR)/expack

USECODE_OBJECTS = \
	src/usecode.uc	

FACES_VGA_OBJECTS = \
	src/graphics/faces.in	\
	src/graphics/faces/dominus.shp	

SHAPES_VGA_OBJECTS = \
	src/graphics/shapes.in	\
	src/graphics/shapes/penguin.shp	

islefaqdir = $(datadir)/exult/forgeofvirtue/mods

islefaq_DATA = \
	islefaq.cfg

islefaqpatchdir = $(islefaqdir)/islefaq/patch

islefaqpatch_DATA = \
	patch/usecode	\
	patch/faces.vga	\
	patch/shapes.vga	\
	patch/initgame.dat	\
	patch/ready.dat	\
	patch/shpdims.dat	\
	patch/tfa.dat	\
	patch/u7ifix0d	\
	patch/u7ifix64	\
	patch/u7ifix65	\
	patch/u7map	\
	patch/wihh.dat	

CLEANFILES = \
	patch/usecode	\
	patch/faces.vga	\
	patch/faces_vga.h	\
	patch/shapes.vga	\
	patch/shapes_vga.h	

$(EXPACK):
	+(cd $(EXPACKDIR);$(MAKE) expack)

$(UCC):
	+(cd $(UCCDIR);$(MAKE))


patch/usecode: $(UCC) $(USECODE_OBJECTS)
	$(UCC) -I src -o patch/usecode src/usecode.uc

patch/faces.vga: $(EXPACK) $(FACES_VGA_OBJECTS)
	$(EXPACK) -i $(srcdir)/src/graphics/faces.in

patch/shapes.vga: $(EXPACK) $(SHAPES_VGA_OBJECTS)
	$(EXPACK) -i $(srcdir)/src/graphics/shapes.in

