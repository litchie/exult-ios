/**
 **	Fnames.h - Names of data files for Exult.
 **
 **	Written: 6/4/99 - JSF
 **/

#ifndef INCL_FNAMES
#define INCL_FNAMES	1

// This will get prepended with different things at runtime
// depending on the OS
#define	USER_CONFIGURATION_FILE	"exult.cfg"

#if (defined(XWIN) || defined(DOS))
/*
 *	Here are the files we use:
 */
#define GAMEDAT		"gamedat/"
#define SHAPES_VGA	"static/shapes.vga"
#define FACES_VGA	"static/faces.vga"
#define GUMPS_VGA	"static/gumps.vga"
#define FONTS_VGA	"static/fonts.vga"
#define SHPDIMS		"static/shpdims.dat"
#define TFA		"static/tfa.dat"
#define WGTVOL		"static/wgtvol.dat"		
#define U7CHUNKS	"static/u7chunks"
#define U7MAP		"static/u7map"
#define TEST_FLX	"static/text.flx"
#define AVATAR_TTF	"avatar.ttf"
#define U7IFIX		"static/u7ifix"
#define U7IREG		"gamedat/u7ireg"
#define PALETTES_FLX	"static/palettes.flx"
#define U7NBUF_DAT	"gamedat/u7nbuf.dat"
#define NPC_DAT		"gamedat/npc.dat"
#define SCHEDULE_DAT	"static/schedule.dat"
#define SHPDIMS_DAT	"static/shpdims.dat"
#define INITGAME	"static/initgame.dat"
#define USECODE		"static/usecode"
#define POINTERS	"static/pointers.shp"

#else

#define GAMEDAT		"gamedat\\"
#define SHAPES_VGA	"static\\shapes.vga"
#define FACES_VGA	"static\\faces.vga"
#define GUMPS_VGA	"static\\gumps.vga"
#define FONTS_VGA	"static\\fonts.vga"
#define SHPDIMS		"static\\shpdims.dat"
#define TFA		"static\\tfa.dat"
#define WGTVOL		"static\\wgtvol.dat"		
#define U7CHUNKS	"static\\u7chunks"
#define U7MAP		"static\\u7map"
#define TEST_FLX	"static\\text.flx"
#define AVATAR_TTF	"avatar.ttf"
#define U7IFIX		"static\\u7ifix"
#define U7IREG		"gamedat\\u7ireg"
#define PALETTES_FLX	"static\\palettes.flx"
#define U7NBUF_DAT	"gamedat\\u7nbuf.dat"
#define NPC_DAT		"gamedat\\npc.dat"
#define SCHEDULE_DAT	"static\\schedule.dat"
#define SHPDIMS_DAT	"static\\shpdims.dat"
#define INITGAME	"static\\initgame.dat"
#define USECODE		"static\\usecode"
#define POINTERS	"static\\pointers.shp"

#endif

#endif



