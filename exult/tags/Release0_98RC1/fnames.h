/*
 *	fnames.h - Names of data files for Exult.
 *
 *  Copyright (C) 1999  Jeffrey S. Freedman
 *  Copyright (C) 2000-2001  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef FNAMES_H
#define FNAMES_H	1

// This will get prepended with different things at runtime
// depending on the OS
#if defined(__MORPHOS__) || defined(AMIGA)
#define USER_CONFIGURATION_FILE "PROGDIR:exult.cfg"
#else
#define USER_CONFIGURATION_FILE "exult.cfg"
#endif

#if (defined(MACOS) || defined(MACOSX)) && !defined(EXULT_DATADIR)
#define EXULT_DATADIR	"data"
#endif

/*
 *	Here are the files we use:
 */
#define GAMEDAT		"<GAMEDAT>/"
#define STATICDAT	"<STATIC>/"
#define PATCHDAT	"<PATCH>/"
#define SHAPES_VGA	"<STATIC>/shapes.vga"
#define PATCH_SHAPES	"<PATCH>/shapes.vga"
#define FACES_VGA	"<STATIC>/faces.vga"
#define PATCH_FACES	"<PATCH>/faces.vga"
#define GUMPS_VGA	"<STATIC>/gumps.vga"
#define FONTS_VGA	"<STATIC>/fonts.vga"
#define SPRITES_VGA     "<STATIC>/sprites.vga"
#define MAINSHP_FLX     "<STATIC>/mainshp.flx"
#define ENDSHAPE_FLX    "<STATIC>/endshape.flx"
#define SHPDIMS		"<STATIC>/shpdims.dat"
#define PATCH_SHPDIMS	"<PATCH>/shpdims.dat"
#define TFA		"<STATIC>/tfa.dat"
#define PATCH_TFA	"<PATCH>/tfa.dat"
#define WGTVOL		"<STATIC>/wgtvol.dat"		
#define PATCH_WGTVOL	"<PATCH>/wgtvol.dat"		
#define U7CHUNKS	"<STATIC>/u7chunks"
#define PATCH_U7CHUNKS	"<PATCH>/u7chunks"
#define U7MAP		"<STATIC>/u7map"
#define PATCH_U7MAP	"<PATCH>/u7map"
#define TEXT_FLX	"<STATIC>/text.flx"
#define U7IFIX		"<STATIC>/u7ifix"
#define PATCH_U7IFIX	"<PATCH>/u7ifix"
#define U7IREG		"<GAMEDAT>/u7ireg"
#define PALETTES_FLX	"<STATIC>/palettes.flx"
#define U7NBUF_DAT	"<GAMEDAT>/u7nbuf.dat"
#define NPC_DAT		"<GAMEDAT>/npc.dat"
#define MONSNPCS	"<GAMEDAT>/monsnpcs.dat"
#define USEDAT		"<GAMEDAT>/usecode.dat"
#define FLAGINIT	"<GAMEDAT>/flaginit"
#define GWINDAT		"<GAMEDAT>/gamewin.dat"
#define GSCHEDULE	"<GAMEDAT>/schedule.dat"
#define SCHEDULE_DAT	"<STATIC>/schedule.dat"
#define SHPDIMS_DAT	"<STATIC>/shpdims.dat"
#define INITGAME	"<STATIC>/initgame.dat"
#define PATCH_INITGAME	"<PATCH>/initgame.dat"
#define USECODE		"<STATIC>/usecode"
#define PATCH_USECODE	"<PATCH>/usecode"
#define POINTERS	"<STATIC>/pointers.shp"
#define MAINMUS		"<STATIC>/mt32mus.dat"
#define INTROMUS	"<STATIC>/intrordm.dat"
#define	XMIDI_MT	"<STATIC>/xmidi.mt"
#define U7SPEECH	"<STATIC>/u7speech.spc"
#define SISPEECH	"<STATIC>/sispeech.spc"
#define XFORMTBL       	"<STATIC>/xform.tbl"
#define MONSTERS	"<STATIC>/monsters.dat"
#define PATCH_MONSTERS	"<PATCH>/monsters.dat"
#define EQUIP		"<STATIC>/equip.dat"
#define PATCH_EQUIP	"<PATCH>/equip.dat"
#define READY		"<STATIC>/ready.dat"
#define PATCH_READY	"<PATCH>/ready.dat"
#define WIHH		"<STATIC>/wihh.dat"
#define PATCH_WIHH	"<PATCH>/wihh.dat"
#define IDENTITY	"<GAMEDAT>/identity"
#define ENDGAME		"<STATIC>/endgame.dat"
#define ENDSCORE_XMI	"<STATIC>/endscore.xmi"
#define MIDITMPFILE     "u7midi"
#define MIDISFXFILE     "u7sfx"
#define SAVENAME	"<SAVEGAME>/exult%02d%s.sav"
#define SAVENAME2	"<SAVEGAME>/exult*%s.sav"
#define INTROSND	"<STATIC>/introsnd.dat"
#define PATCH_ARMOR	"<PATCH>/armor.dat"
#define ARMOR		"<STATIC>/armor.dat"
#define WEAPONS		"<STATIC>/weapons.dat"
#define PATCH_WEAPONS	"<PATCH>/weapons.dat"
#define AMMO		"<STATIC>/ammo.dat"
#define PATCH_AMMO	"<PATCH>/ammo.dat"
#define PAPERDOL	"<STATIC>/paperdol.vga"
#define OCCLUDE		"<STATIC>/occlude.dat"
#define PATCH_OCCLUDE	"<PATCH>/occlude.dat"

#define GSCRNSHOT	"<GAMEDAT>/scrnshot.shp"
#define GSAVEINFO	"<GAMEDAT>/saveinfo.dat"
#define GEXULTVER	"<GAMEDAT>/exult.ver"
#define GNEWGAMEVER	"<GAMEDAT>/newgame.ver"
#define KEYRINGDAT	"<GAMEDAT>/keyring.dat"

#define NUM_FONTS	(20)

#endif

