INCLUDES = -I. -I../files $(DEBUG_FLAGS) $(CPPFLAGS)

if LEXYACC
UCC= ucc
else
UCC=
endif

if HAVE_GIMPTOOL
GIMP_PLUGINS=u7shpload
else
GIMP_PLUGINS=
endif

noinst_PROGRAMS = expack wuc wud mklink rip ucdump $(UCC) $(GIMP_PLUGINS)

expack_SOURCES = \
	expack.cc

expack_LDADD = \
	../files/libu7file.la

wuc_SOURCES = \
	wuc.c \
	uctools.h

wud_SOURCES = wud.c

mklink_SOURCES = mklink.c

rip_SOURCES = rip.c

ucdump_SOURCES = ucdump.c

ucc_SOURCES = \
	ucclex.l \
	uccparse.y

ucclex.c: ucclex.l
	$(LEX) $< && mv lex.yy.c $@
uccparse.c: uccparse.y
	$(YACC) $< && mv y.tab.c $@
	
uccparse.o: ucclex.c uccparse.c
	$(COMPILE) -o $@ -c uccparse.c

ucc: uccparse.o
	$(LINK) uccparse.o

u7shpload: u7shpload.c
	gimptool --build $<

if HAVE_GIMPTOOL
install-exec-local:
	gimptool --install-admin-bin u7shpload
endif

EXTRA_DIST = \
	intrins1.txt \
	intrins2.txt \
	README \
	u7bgflag.txt \
	u7siflag.txt \
	ucformat.txt

CLEANFILES = \
	u7shpload
