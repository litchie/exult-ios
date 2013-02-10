# This is an automatically generated file; please do not edit it manually.
# Instead, run makefile_builder.sh from the parent directory.

# Base of the exult source
SRC=../..

UCCDIR=$(SRC)/usecode/compiler
UCC=$(UCCDIR)/ucc

EXPACKDIR=$(SRC)/tools
EXPACK=$(EXPACKDIR)/expack

USECODE_OBJECTS = \
	src/cutscenes/fawn_storm.uc	\
	src/cutscenes/fawn_trial.uc	\
	src/cutscenes/monitor_banquet.uc	\
	src/cutscenes/wall_of_lights.uc	\
	src/header/constants.uc	\
	src/header/functions.uc	\
	src/header/si/si_externals.uc	\
	src/header/si/si_gflags.uc	\
	src/header/si/si_npcs.uc	\
	src/header/si/si_shapes.uc	\
	src/items/bucket_cure.uc	\
	src/items/hourglass.uc	\
	src/items/pillar.uc	\
	src/items/potion.uc	\
	src/items/scroll.uc	\
	src/items/time_tellers.uc	\
	src/misc/egg_bane_holocaust.uc	\
	src/misc/egg_gorlab_swamp_sleep.uc	\
	src/misc/egg_starting_hints.uc	\
	src/misc/exchanged_item_list.uc	\
	src/misc/fawn_tower_cleanup.uc	\
	src/misc/inn_keys.uc	\
	src/misc/location_ids.uc	\
	src/misc/luther_return_shield.uc	\
	src/misc/resurrect.uc	\
	src/npcs/baiyanda.uc	\
	src/npcs/cantra.uc	\
	src/npcs/dupre.uc	\
	src/npcs/frigidazzi.uc	\
	src/npcs/ghost.uc	\
	src/npcs/goblin_simon.uc	\
	src/npcs/gwenno.uc	\
	src/npcs/iolo.uc	\
	src/npcs/shamino.uc	\
	src/npcs/thoxa.uc	\
	src/spells/spells.uc	\
	src/usecode.uc	

MAINSHP_FLX_OBJECTS = \
	src/graphics/mainshp.in	\
	src/graphics/mainshp/title.shp	

SHAPES_VGA_OBJECTS = \
	src/graphics/shapes.in	\
	src/graphics/shapes/sails.shp	

sifixesdir = $(datadir)/exult/silverseed/mods

sifixes_DATA = \
	sifixes.cfg

sifixesdatadir = $(sifixesdir)/sifixes/data

sifixesdata_DATA = \
	data/usecode	\
	data/mainshp.flx	\
	data/shapes.vga	\
	data/initgame.dat	

CLEANFILES = \
	data/usecode	\
	data/mainshp.flx	\
	data/mainshp_flx.h	\
	data/shapes.vga	\
	data/shapes_vga.h	

$(EXPACK):
	+(cd $(EXPACKDIR);$(MAKE) expack)

$(UCC):
	+(cd $(UCCDIR);$(MAKE))


data/usecode: $(UCC) $(USECODE_OBJECTS)
	$(UCC) -I src -o data/usecode src/usecode.uc

data/mainshp.flx: $(EXPACK) $(MAINSHP_FLX_OBJECTS)
	$(EXPACK) -i $(srcdir)/src/graphics/mainshp.in

data/shapes.vga: $(EXPACK) $(SHAPES_VGA_OBJECTS)
	$(EXPACK) -i $(srcdir)/src/graphics/shapes.in

