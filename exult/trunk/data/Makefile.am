# Never ever change the ordere here: Exult depends on it
EXULT_FLX_OBJECTS = \
	exult_quotes.shp \
	exult_credits.shp \
	quotes.txt \
	credits.txt \
	exult_logo.shp \
	exult0.pal

CLEANFILES = \
	exult.flx *~

EXTRA_DIST = $(EXULT_FLX_OBJECTS)

all-local:
	../tools/expack -c exult.flx $(EXULT_FLX_OBJECTS)
