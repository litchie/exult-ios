INCLUDES = -I. $(DEBUG_FLAGS) $(CPPFLAGS) -DRELNUM=$(RELNUM)

noinst_PROGRAMS = wuc wud mklink rip ucdump ucc

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


EXTRA_DIST = \
	intrins1.txt \
	intrins2.txt \
	README \
	u7bgflag.txt \
	u7siflag.txt \
	ucformat.txt
