# Exult makefile for use in BeOS
# It may require a little tweaking. (paths)

# Base of the exult source
SRC=.
VPATH=$(SRC):$(SRC)/files:$(SRC)/gumps:$(SRC)/pathfinder:$(SRC)/flic:$(SRC)/conf:$(SRC)/audio:$(SRC)/audio/midi_drivers:$(SRC)/imagewin:$(SRC)/usecode:$(SRC)/shapes:$(SRC)/objs:$(SRC)/files/zip

VERSION=0.98rc1

### Modify these paths
SDL_CFLAGS=-I/boot/develop/tools/gnupro/include/SDL
SDL_LIBS=-L/boot/develop/tools/gnupro/lib -lSDLmain -lSDL

CPPFLAGS=-DVERSION=\"$(VERSION)\" -DBEOS -DDEBUG -DEXULT_DATADIR=\"data\" \
	-DHAVE_ZIP_SUPPORT -I$(SRC)/files/zip \
	-I$(SRC)/imagewin -I$(SRC)/shapes -I$(SRC)/data -I$(SRC)/tools \
	-I$(SRC)/files -I$(SRC) -I$(SRC)/audio -I$(SRC)/conf \
	-I$(SRC)/gumps -I$(SRC)/objs -I$(SRC)/pathfinder -I$(SRC)/usecode \
	-DHAVE_SNPRINTF $(SDL_CFLAGS)
CXXFLAGS=-O2 -Wall

LFLAGS=
LIBS=$(SDL_LIBS) -lmidi -lbe -lz

EXEC=exult
MAIN_OBJS=actions.o actorio.o actors.o alloc.o \
	args.o bggame.o browser.o cheat.o \
	combat.o delobjs.o devgame.o \
	dir.o drag.o effects.o exult.o exultmenu.o \
	game.o gameclk.o gamedat.o gamemap.o \
	gamerend.o gamewin.o keys.o keyactions.o \
	menulist.o monsters.o mouse.o npcnear.o npctime.o palette.o \
	paths.o readnpcs.o schedule.o segfile.o sigame.o tqueue.o txtscroll.o \
	cheat_screen.o shapeid.o version.o
# unused: npctest.o
PATH_OBJS=Astar.o PathFinder.o Zombie.o path.o
CONF_OBJS=Configuration.o XMLEntity.o
# unused: xmain.o
MIDI_DRV_OBJS=be_midi.o
# unused: KMIDI.o Timidity_binary.o forked_player.o win_MCI.o
AUDIO_OBJS=Audio.o conv.o Midi.o Mixer.o SDL_mapping.o pcb.o xmidi.o soundtest.o $(MIDI_DRV_OBJS)
# unused: test.o u7audiotool.o
FLIC_OBJS=playfli.o
FILE_OBJS=U7file.o Flex.o IFF.o Table.o Flat.o utils.o listfiles.o crc.o
FILE_ZIP_OBJS=zip.o unzip.o
GUMPS_OBJS= Actor_gump.o  Book_gump.o  File_gump.o  Gump.o  Gump_button.o \
	gump_utils.o  Gump_widget.o  misc_buttons.o  Paperdoll_gump.o  \
	Paperdoll_gump_info.o Scroll_gump.o Sign_gump.o  Slider_gump.o \
	Spellbook_gump.o Stats_gump.o Text_gump.o  Yesno_gump.o \
	Gamemenu_gump.o Newfile_gump.o Gump_ToggleButton.o \
	AudioOptions_gump.o Face_button.o CombatStats_gump.o \
	Jawbone_gump.o VideoOptions_gump.o Face_stats.o Gump_manager.o \
	Text_button.o GameplayOptions_gump.o Enabled_button.o
IMAGEWIN_OBJS=imagebuf.o imagewin.o iwin8.o ibuf8.o ibuf16.o imagescl.o \
	savepcx.o
SHAPES_OBJS=bodies.o items.o shapeinf.o shapevga.o vgafile.o font.o fontvga.o \
	monstinf.o
OBJS_OBJS=animate.o barge.o chunks.o chunkter.o \
	contain.o egg.o iregobjs.o mappatch.o objs.o \
	spellbook.o virstone.o jawbone.o
# unused: test.o
USECODE_OBJS=ucinternal.o ucmachine.o ucsched.o intrinsics.o useval.o \
	conversation.o keyring.o ucdisasm.o
OBJS=$(MAIN_OBJS) $(PATH_OBJS) $(CONF_OBJS) $(AUDIO_OBJS) $(FLIC_OBJS) $(FILE_OBJS) $(GUMPS_OBJS) $(OBJS_OBJS) $(SHAPES_OBJS) $(IMAGEWIN_OBJS) $(USECODE_OBJS) $(FILE_ZIP_OBJS)

EXULT_FLX_OBJECTS = \
	data/exult_quotes.shp \
	data/exult_credits.shp \
	data/quotes.txt \
	data/credits.txt \
	data/exult_logo.shp \
	data/exult0.pal \
	data/black_gate.shp \
	data/serpent_isle.shp \
	data/meditown.mid \
	data/font.shp \
	data/setup.shp \
	data/play_intro.shp \
	data/full_screen.shp \
	data/cheating.shp \
	data/ok.shp \
	data/cancel.shp \
	data/pointers.shp \
	data/exit.shp \
	data/play_1st_scene.shp \
	data/extras.shp \
	data/midi_conversion.shp \
	data/sfx_conversion.shp \
	data/palette_fades.shp \
	data/scaling_method.shp \
	data/savegump.shp \
	data/sav_downdown.shp \
	data/sav_down.shp \
	data/sav_up.shp \
	data/sav_upup.shp \
	data/sav_slider.shp \
	data/sav_selected.shp \
	data/gameplayoptions.shp \
	data/gamemenu.shp \
	data/audiooptions.shp \
	data/videooptions.shp \
	data/hp_bar.shp \
	data/sfx_icon.shp
	data/flx.in

EXULT_BG_FLX_OBJECTS = \
	data/bg/hoe_of_destruction.shp \
	data/bg/caddellite_helmet.shp \
	data/bg/great_dagger.shp \
	data/bg/magic_boomerang.shp \
	data/bg/gorget.shp \
	data/bg/magicgorget.shp \
	data/bg/cleaver.shp \
	data/bg/faces.shp \
	data/bg/faces2.shp \
	data/bg/amulets.shp \
	data/bg/bgfiredoom.shp \
	data/bg/fellowshipstaff.shp \
	data/bg/BGmap.shp \
	data/bg/defaultkeys.txt \
	data/bg/dupre_face.shp \
	data/bg/fem_av_face.shp \
	data/bg/iolo_face.shp \
	data/bg/male_av_face.shp \
	data/bg/shamino_face.shp \
	data/bg/greaves.shp \
	data/bg/spiked_shield.shp \
	data/bg/mr_faces.shp \
	data/bg/mr_intro.shp \
	data/bg/flx.in

EXULT_SI_FLX_OBJECTS = \
	data/si/SImap.shp \
	data/si/defaultkeys.txt \
	data/si/flx.in


$(EXEC) : Makefile data/exult.flx data/exult_bg.flx data/exult_si.flx $(OBJS) beos/exult.rsrc
	$(CXX) $(LFLAGS) -o $@ $(OBJS) $(LIBS)
	xres -o $(EXEC) beos/exult.rsrc

tools/expack : tools/expack.o $(FILE_OBJS)
	$(CXX) $(LFLAGS) -o tools/expack tools/expack.o $(FILE_OBJS) $(LIBS)

data/exult.flx: tools/expack data/flx.in $(EXULT_FLX_OBJECTS)
	tools/expack -i data/flx.in

data/exult_bg.flx: tools/expack $(EXULT_BG_FLX_OBJECTS)
	tools/expack -i data/bg/flx.in

data/exult_si.flx: tools/expack $(EXULT_SI_FLX_OBJECTS)
	tools/expack -i data/si/flx.in

imagescl.o: imagewin/imagescl.cc scale.cc
	$(CXX) $(CPPFLAGS) -O3 -c imagewin/imagescl.cc -o imagescl.o

Makefile: Makefile.be
	cp Makefile.be Makefile

clean:
	rm -f $(OBJS) $(EXEC) data/exult.flx data/exult_bg.flx data/exult_si.flx tools/expack tools/expack.o

finishedbinary: $(EXEC)
	strip $(EXEC)
	xres -o $(EXEC) beos/exult.rsrc

run: $(EXEC)
	./$(EXEC)

bindist: finishedbinary
	rm -rf exult-$(VERSION)
	mkdir exult-$(VERSION)
	mkdir exult-$(VERSION)/data
	mkdir exult-$(VERSION)/doc
	mkdir exult-$(VERSION)/lib
	cp $(EXEC) exult-$(VERSION)
	cp README FAQ README.BeOS COPYING NEWS AUTHORS ChangeLog exult-$(VERSION)/doc
	cp data/exult.flx data/exult_bg.flx data/exult_si.flx exult-$(VERSION)/data
	cp $(HOME)/lib/libSDL.so $(HOME)/lib/README-SDL.txt exult-$(VERSION)/lib
	rm -f exult-$(VERSION).x86.beos.zip
	zip -r exult-$(VERSION).x86.beos.zip exult-$(VERSION)
	rm -rf exult-$(VERSION) 
