# Never ever change the order here: Exult depends on it
EXULT_FLX_OBJECTS = \
	exult_quotes.shp	\
	exult_credits.shp	\
	quotes.txt		\
	credits.txt		\
	exult_logo.shp		\
	exult0.pal		\
	black_gate.shp		\
	serpent_isle.shp	\
	meditown.mid		\
	font.shp		\
	setup.shp		\
	play_intro.shp		\
	full_screen.shp		\
	cheating.shp		\
	ok.shp			\
	cancel.shp		\
	pointers.shp		\
	exit.shp		\
	play_1st_scene.shp	\
	extras.shp		\
	midi_conversion.shp	\
	sfx_conversion.shp	\
	palette_fades.shp	\
	defaultkeys.txt		\
	scaling_method.shp	\
	hoe_of_destruction.shp  \
	caddellite_helmet.shp	\
	great_dagger.shp	\
	magic_boomerang.shp	\
	gorget.shp		\
	magicgorget.shp		\
	cleaver.shp		\
	faces.shp

flexdir = $(datadir)/exult

if DATA_FILES
EXULT_FLX = exult.flx
else
EXULT_FLX =
endif

flex_DATA = $(EXULT_FLX) midisfx.flx

CLEANFILES = exult.flx *~

EXTRA_DIST = flx.in $(EXULT_FLX_OBJECTS) midisfx.flx

exult.flx: $(EXULT_FLX_OBJECTS)
	../tools/expack -i $(srcdir)/flx.in
