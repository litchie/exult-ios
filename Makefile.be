# Exult makefile for use in BeOS
# It may require a little tweaking. (paths)

# Base of the exult source
SRC=.
VPATH=$(SRC):$(SRC)/files:$(SRC)/pathfinder:$(SRC)/flic:$(SRC)/conf:$(SRC)/audio:$(SRC)/audio/midi_drivers

### Modify these paths
SDL_INCLUDES=-I/boot/develop/tools/gnupro/include/SDL
SDL_LIBS=-L/boot/develop/tools/gnupro/lib -lSDLmain -lSDL

CPPFLAGS=-DVERSION=\"0.90alpha1\" -DBEOS -DDEBUG -DEXULT_DATADIR=\"data\" \
	-DNO_INTRO -DAUTOCONFIGURED -I$(SRC)/files \
	-I$(SRC) -I$(SRC)/audio -I$(SRC)/conf -I$(SRC)/pathfinder \
	$(SDL_INCLUDES)
CXXFLAGS=-g -Wall

LFLAGS=-g
LIBS=$(SDL_LIBS) -lmidi -lbe

EXEC=exult
MAIN_OBJS=actions.o actorio.o actors.o alloc.o animate.o \
	args.o barge.o bggame.o bodies.o browser.o cheat.o\
	combat.o delobjs.o dir.o drag.o effects.o egg.o exult.o font.o \
	game.o gameclk.o gamedat.o gamerend.o gametxt.o gamewin.o \
	gumps.o imagescl.o imagewin.o items.o lists.o menulist.o mouse.o \
	npcnear.o npctime.o objs.o palette.o paths.o readnpcs.o scale.o schedule.o \
	segfile.o sigame.o spells.o tqueue.o txtscroll.o usecode.o \
    useval.o vgafile.o virstone.o
# unused: npctest.o
PATH_OBJS=Astar.o PathFinder.o Zombie.o path.o
CONF_OBJS=Configuration.o XMLEntity.o xmldump.o xmlparse.o
# unused: xmain.o
MIDI_DRV_OBJS=be_midi.o
# unused: KMIDI.o Timidity_binary.o forked_player.o win_MCI.o
AUDIO_OBJS=Audio.o Midi.o Mixer.o SDL_mapping.o pcb.o xmidi.o $(MIDI_DRV_OBJS)
# unused: test.o u7audiotool.o
FLIC_OBJS=playfli.o
FILE_OBJS=U7file.o Flex.o IFF.o Table.o Flat.o utils.o
# unused: test.o
OBJS=$(MAIN_OBJS) $(PATH_OBJS) $(CONF_OBJS) $(AUDIO_OBJS) $(FLIC_OBJS) $(FILE_OBJS)

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
	data/sfx_conversion.shp

$(EXEC) : Makefile data/exult.flx $(OBJS)
	$(CXX) $(LFLAGS) -o $@ $(OBJS) $(LIBS)

tools/expack : tools/expack.o $(FILE_OBJS)
	$(CXX) $(LFLAGS) -o tools/expack tools/expack.o $(FILE_OBJS) $(LIBS)

data/exult.flx: tools/expack $(EXULT_FLX_OBJECTS)
	tools/expack -c data/exult.flx $(EXULT_FLX_OBJECTS)

imagescl.o: imagescl.cc scale.cc
	$(CXX) $(CPPFLAGS) -O3 -c imagescl.cc -o imagescl.o

Makefile: Makefile.be
	cp Makefile.be Makefile

clean:
	rm -f $(OBJS) $(EXEC) data/exult.flx tools/expack tools/expack.o

install:
	strip $(EXEC)

run:
	./$(EXEC)


