# Exult makefile for use in BeOS
# It may require a little tweaking. (paths)

# Base of the exult source
SRC=.
VPATH=$(SRC):$(SRC)/files:$(SRC)/gumps:$(SRC)/pathfinder:$(SRC)/flic:$(SRC)/conf:$(SRC)/audio:$(SRC)/audio/midi_drivers:$(SRC)/imagewin:$(SRC)/usecode:$(SRC)/shapes:$(SRC)/objs

VERSION=0.92cvs

### Modify these paths
SDL_INCLUDES=-I/boot/develop/tools/gnupro/include/SDL
SDL_LIBS=-L/boot/develop/tools/gnupro/lib -lSDLmain -lSDL

CPPFLAGS=-DVERSION=\"$(VERSION)\" -DBEOS -DDEBUG -DEXULT_DATADIR=\"data\" \
	-I$(SRC)/imagewin -I$(SRC)/shapes \
	-I$(SRC)/files -I$(SRC) -I$(SRC)/audio -I$(SRC)/conf \
	-I$(SRC)/gumps -I$(SRC)/objs -I$(SRC)/pathfinder -I$(SRC)/usecode \
	$(SDL_INCLUDES)
CXXFLAGS=-O2 -Wall

LFLAGS=
LIBS=$(SDL_LIBS) -lmidi -lbe

EXEC=exult
MAIN_OBJS=actions.o actorio.o actors.o alloc.o \
	args.o bggame.o browser.o cheat.o \
	combat.o delobjs.o dir.o drag.o effects.o exult.o exultmenu.o \
	game.o gameclk.o gamedat.o gamerend.o gamewin.o keys.o keyactions.o \
	menulist.o mouse.o npcnear.o npctime.o palette.o \
	paths.o readnpcs.o schedule.o segfile.o sigame.o tqueue.o txtscroll.o \
	cheat_screen.o
# unused: npctest.o
PATH_OBJS=Astar.o PathFinder.o Zombie.o path.o
CONF_OBJS=Configuration.o XMLEntity.o xmldump.o xmlparse.o
# unused: xmain.o
MIDI_DRV_OBJS=be_midi.o
# unused: KMIDI.o Timidity_binary.o forked_player.o win_MCI.o
AUDIO_OBJS=Audio.o Midi.o Mixer.o SDL_mapping.o pcb.o xmidi.o soundtest.o $(MIDI_DRV_OBJS)
# unused: test.o u7audiotool.o
FLIC_OBJS=playfli.o
FILE_OBJS=U7file.o Flex.o IFF.o Table.o Flat.o utils.o
GUMPS_OBJS= Actor_gump.o  Book_gump.o  File_gump.o  Gump.o  Gump_button.o \
	gump_utils.o  Gump_widget.o  misc_buttons.o  Paperdoll_gump.o  \
	Paperdoll_gump_info.o Scroll_gump.o Sign_gump.o  Slider_gump.o \
	Spellbook_gump.o Stats_gump.o Text_gump.o  Yesno_gump.o
IMAGEWIN_OBJS=imagebuf.o imagewin.o iwin8.o ibuf8.o ibuf16.o imagescl.o \
	savepcx.o
SHAPES_OBJS=bodies.o items.o shapeinf.o shapevga.o vgafile.o font.o fontvga.o
OBJS_OBJS=animate.o barge.o chunks.o contain.o egg.o iregobjs.o objs.o \
	spellbook.o virstone.o
# unused: test.o
USECODE_OBJS=ucmachine.o ucsched.o intrinsics.o useval.o conversation.o
OBJS=$(MAIN_OBJS) $(PATH_OBJS) $(CONF_OBJS) $(AUDIO_OBJS) $(FLIC_OBJS) $(FILE_OBJS) $(GUMPS_OBJS) $(OBJS_OBJS) $(SHAPES_OBJS) $(IMAGEWIN_OBJS) $(USECODE_OBJS)

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
	data/defaultkeys.txt \
	data/hoe_of_destruction.shp \
	data/caddellite_helmet.shp \
	data/great_dagger.shp \
	data/magic_boomerang.shp \
	data/gorget.shp \
	data/magicgorget.shp \
	data/cleaver.shp \
	data/faces.shp \
	data/faces2.shp \
	data/amulets.shp \
	data/bgfiredoom.shp \
	data/flx.in

$(EXEC) : Makefile data/exult.flx $(OBJS) beos/exult.rsrc
	$(CXX) $(LFLAGS) -o $@ $(OBJS) $(LIBS)
	xres -o $(EXEC) beos/exult.rsrc

tools/expack : tools/expack.o $(FILE_OBJS)
	$(CXX) $(LFLAGS) -o tools/expack tools/expack.o $(FILE_OBJS) $(LIBS)

data/exult.flx: tools/expack data/flx.in $(EXULT_FLX_OBJECTS)
	tools/expack -i data/flx.in

imagescl.o: imagewin/imagescl.cc scale.cc
	$(CXX) $(CPPFLAGS) -O3 -c imagewin/imagescl.cc -o imagescl.o

Makefile: Makefile.be
	cp Makefile.be Makefile

clean:
	rm -f $(OBJS) $(EXEC) data/exult.flx tools/expack tools/expack.o

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
	cp data/exult.flx data/midisfx.flx exult-$(VERSION)/data
	cp $(HOME)/lib/libSDL.so $(HOME)/lib/README-SDL.txt exult-$(VERSION)/lib
	rm -f exult-$(VERSION).x86.beos.zip
	zip -r exult-$(VERSION).x86.beos.zip exult-$(VERSION)
	rm -rf exult-$(VERSION) 
