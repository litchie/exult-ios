EXULT_FLX_OBJECTS = \
	flx.in			\
	quotes.txt		\
	credits.txt		\
	exult_logo.shp		\
	exult0.pal		\
	meditown.mid		\
	font.shp		\
	fonton.shp		\
	navfont.shp		\
	navfonton.shp		\
	pointers.shp		\
	extras.shp		\
	notebook.shp		\
	stats_extra.shp		\
	savegump.shp		\
	sav_downdown.shp	\
	sav_down.shp		\
	sav_up.shp		\
	sav_upup.shp		\
	sav_slider.shp		\
	sav_selected.shp	\
	gameplayoptions.shp	\
	gamemenu.shp		\
	audiooptions.shp	\
	videooptions.shp	\
	hp_bar.shp		\
	sfx_icon.shp		\
	mtgm.mid

EXULT_BG_FLX_OBJECTS = \
	bg/flx.in \
	bg/BGmap.shp \
	bg/defaultkeys.txt	\
	bg/mr_faces.shp \
	bg/u7menupal.pal \
	bg/bg_paperdol.vga \
	bg/bg_mr_faces.vga \
	bg/bodies.txt \
	bg/paperdol_info.txt \
	bg/shape_files.txt \
	bg/avatar_data.txt \
	bg/shape_info.txt \
	bg/blends.dat

EXULT_SI_FLX_OBJECTS = \
	si/flx.in \
	si/SImap.shp \
	si/defaultkeys.txt \
	si/bodies.txt \
	si/paperdol_info.txt \
	si/shape_files.txt \
	si/avatar_data.txt \
	si/shape_info.txt \
	si/blends.dat

EXULT_BG_PAPERDOL_VGA_OBJECTS = \
	bg/amulets.shp \
	bg/bgfiredoom.shp \
	bg/bluedress.shp \
	bg/caddellite_helmet.shp \
	bg/cleaver.shp \
	bg/custom_sword.shp \
	bg/death_scythe.shp \
	bg/dupre_face.shp \
	bg/fellowshipstaff.shp \
	bg/fem_av_face.shp \
	bg/firesword.shp \
	bg/firewand.shp \
	bg/gorget.shp \
	bg/great_dagger.shp \
	bg/greaves.shp \
	bg/greencloak.shp \
	bg/greentop.shp \
	bg/greycloak.shp \
	bg/hawk.shp \
	bg/hoe_of_destruction.shp \
	bg/hoods.shp \
	bg/iolo_face.shp \
	bg/jaana_face.shp \
	bg/julia_face.shp \
	bg/katrina_face.shp \
	bg/knife.shp \
	bg/lightning_whip.shp \
	bg/magic_boomerang.shp \
	bg/magic_helmet.shp \
	bg/main_gauche.shp \
	bg/male_av_face.shp \
	bg/musket.shp \
	bg/musket_ammo.shp \
	bg/orangedress.shp \
	bg/pants.shp \
	bg/ring_of_protection.shp \
	bg/sentri_face.shp \
	bg/shamino_face.shp \
	bg/shoes.shp \
	bg/spark_face.shp \
	bg/spiked_shield.shp \
	bg/starburst.shp \
	bg/throwing_axe.shp \
	bg/triple_xbow.shp \
	bg/tseramed_face.shp \
	bg/swamp_boots.shp \
	bg/spellbook.shp \
	bg/av_costume.shp \
	bg/oil_flask.shp \
	bg/love_arrows.shp \
	bg/magebane.shp \
	bg/shieldbacks.shp \
	bg/bg_paperdol.in
EXULT_BG_MR_FACES_VGA_OBJECTS = \
	bg/mr_faces.shp \
	bg/u7menupal.pal \
	bg/bg_mr_faces.in

ESTUDIO_NEW_FILES = \
	estudio/new/combos.flx \
	estudio/new/faces.vga \
	estudio/new/gumps.vga \
	estudio/new/palettes.flx \
	estudio/new/paperdol.vga \
	estudio/new/shapes.vga \
	estudio/new/sprites.vga \
	estudio/new/text.flx \
	estudio/new/fonts.vga \
	estudio/new/pointers.shp \
	estudio/new/blends.dat

flexdir = $(datadir)/exult
estudionewdir = $(datadir)/exult/estudio/new

if DATA_FILES
EXULT_FLX = exult.flx
EXULT_BG_FLX = exult_bg.flx
EXULT_SI_FLX = exult_si.flx
EXULT_BG_PAPERDOL_VGA = bg/bg_paperdol.vga
EXULT_BG_MR_FACES_VGA = bg/bg_mr_faces.vga
else
EXULT_FLX =
EXULT_BG_FLX =
EXULT_SI_FLX =
EXULT_BG_PAPERDOL_VGA =
EXULT_BG_MR_FACES_VGA =
endif

flex_DATA = make_expack \
		$(EXULT_FLX) $(EXULT_BG_FLX) $(EXULT_SI_FLX) midisfx.flx \
		exultmsg.txt $(EXULT_BG_PAPERDOL_VGA) $(EXULT_BG_MR_FACES_VGA)

if BUILD_STUDIO
estudionew_DATA = $(ESTUDIO_NEW_FILES)
else
estudionew_DATA =
endif

CLEANFILES = exult.flx exult_bg.flx exult_si.flx bg/bg_paperdol.vga bg/bg_mr_faces.vga *~

EXTRA_DIST = flx.in $(EXULT_FLX_OBJECTS) $(EXULT_BG_FLX_OBJECTS) \
	$(EXULT_SI_FLX_OBJECTS) $(ESTUDIO_NEW_FILES) midisfx.flx \
	exultmsg.txt bginclude.uc \
	$(EXULT_BG_PAPERDOL_VGA_OBJECTS) $(EXULT_BG_MR_FACES_VGA_OBJECTS)

../tools/expack: make_expack

make_expack:
	+(cd ../tools;$(MAKE) expack)

bg/bg_paperdol.vga: ../tools/expack $(EXULT_BG_PAPERDOL_VGA_OBJECTS)
	../tools/expack -i $(srcdir)/bg/bg_paperdol.in

bg/bg_mr_faces.vga: ../tools/expack $(EXULT_BG_MR_FACES_VGA_OBJECTS)
	../tools/expack -i $(srcdir)/bg/bg_mr_faces.in

exult.flx: ../tools/expack $(EXULT_FLX_OBJECTS)
	../tools/expack -i $(srcdir)/flx.in

exult_bg.flx: ../tools/expack $(EXULT_BG_FLX_OBJECTS)
	../tools/expack -i $(srcdir)/bg/flx.in

exult_si.flx: ../tools/expack $(EXULT_SI_FLX_OBJECTS)
	../tools/expack -i $(srcdir)/si/flx.in
