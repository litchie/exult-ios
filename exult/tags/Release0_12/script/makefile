#
#	Makefile for scripts.
#

OBJECTS = script.o scrlex.o exec.o

CXXFLAGS = -g
CPPFLAGS = -I.. -I../npc
YACC = yacc
YFLAGS = -v -d
LEX = lex
LFLAGS = -L

test1:	libscr.a test1.o
	g++ -g -o test1 test1.o libscr.a libscr.a ../npc/libnpc.a

libscr.a: $(OBJECTS)
	$(AR) r $@ $?

%.o:%.cc
	$(CC) -c $(CPPFLAGS) $(CXXFLAGS) $<

%.cc:%.y
	$(YACC) $(YFLAGS) $*.y
	mv -f y.tab.c $*.cc
	(if cmp y.tab.h $*.h; \
		then rm y.tab.h; \
		else mv -f y.tab.h $*.h; fi)

%.cc:%.l
	$(LEX) $(LFLAGS) $?
	mv -f lex.yy.c $*.cc

script.cc:	script.y
script.o:	script.y
scrlex.cc:	scrlex.l
scrlex.o:	scrlex.l script.h
exec.cc:	exec.h script.h

#libnpc.a(%.o):
#	$(AR) r $@ $*.o

clean:
	-rm *.o *.a
	-rm script.cc script.h scrlex.cc
