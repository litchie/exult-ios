AUTOMAKE_OPTIONS=foreign

all-local:	libflic.a

INCDIRS += -I.. $(SDL_INCLUDES)
CXXFLAGS += $(INCDIRS) -Wall
CXXFLAGS += $(WINDOWING_SYSTEM)
CXXFLAGS += $(DEBUG_FLAGS)


OFILES=	playfli.o

SOURCES= playfli.cc playfli.h

libflic.a:	$(OFILES)
	ar rs libflic.a $(OFILES)

clean:
	rm -f libflic.a core $(OFILES)

test:	libflic.a play.o
	g++ $(CXXFLAGS) -o playflic play.o ../utils.o -L. -lflic

%.o : %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<

rebuild_master_dependencies:
	makedepend -f Makefile.am -Y $(CXXFLAGS) -I. -I.. -I../conf -I../audio -I../pathfinder -I../files - -- *.cc


# DO NOT DELETE

play.o: playfli.h ../imagewin.h /usr/include/SDL/SDL.h
play.o: /usr/include/SDL/SDL_main.h /usr/include/SDL/SDL_types.h
play.o: /usr/include/SDL/SDL_error.h /usr/include/SDL/begin_code.h
play.o: /usr/include/SDL/close_code.h /usr/include/SDL/SDL_rwops.h
play.o: /usr/include/SDL/SDL_timer.h /usr/include/SDL/SDL_audio.h
play.o: /usr/include/SDL/SDL_byteorder.h /usr/include/SDL/SDL_cdrom.h
play.o: /usr/include/SDL/SDL_events.h /usr/include/SDL/SDL_active.h
play.o: /usr/include/SDL/SDL_keyboard.h /usr/include/SDL/SDL_keysym.h
play.o: /usr/include/SDL/SDL_mouse.h /usr/include/SDL/SDL_video.h
play.o: /usr/include/SDL/SDL_mutex.h /usr/include/SDL/SDL_quit.h
play.o: /usr/include/SDL/SDL_version.h
playfli.o: playfli.h ../imagewin.h /usr/include/SDL/SDL.h
playfli.o: /usr/include/SDL/SDL_main.h /usr/include/SDL/SDL_types.h
playfli.o: /usr/include/SDL/SDL_error.h /usr/include/SDL/begin_code.h
playfli.o: /usr/include/SDL/close_code.h /usr/include/SDL/SDL_rwops.h
playfli.o: /usr/include/SDL/SDL_timer.h /usr/include/SDL/SDL_audio.h
playfli.o: /usr/include/SDL/SDL_byteorder.h /usr/include/SDL/SDL_cdrom.h
playfli.o: /usr/include/SDL/SDL_events.h /usr/include/SDL/SDL_active.h
playfli.o: /usr/include/SDL/SDL_keyboard.h /usr/include/SDL/SDL_keysym.h
playfli.o: /usr/include/SDL/SDL_mouse.h /usr/include/SDL/SDL_video.h
playfli.o: /usr/include/SDL/SDL_mutex.h /usr/include/SDL/SDL_quit.h
playfli.o: /usr/include/SDL/SDL_version.h ../utils.h
