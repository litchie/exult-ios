/*
 *	gamedat.cc - Create gamedat files from a savegame.
 *
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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef WIN32
#include <io.h>
#endif

#ifndef ALPHA_LINUX_CXX
#  include <fstream>
#  include <cstdio>
#  include <cstdlib>
#  include <cstring>
#  include <ctime>
#endif

#if (defined(XWIN) || defined(BEOS))
#include <sys/stat.h>
#elif defined(MACOS)
#include <stat.h>
#endif

#include "exceptions.h"
#include "fnames.h"
#include "gamewin.h"
#include "gameclk.h"
#include "gamemap.h"
#include "utils.h"
#include "gump_utils.h"
#include "game.h"
#include "Flex.h"
#include "databuf.h"
#include "Newfile_gump.h"
#include "Yesno_gump.h"
#include "actors.h"
#include "party.h"
#include "version.h"

using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::localtime;
using std::memset;
using std::ofstream;
using std::ostream;
using std::size_t;
using std::strchr;
using std::strcmp;
using std::strcpy;
using std::strlen;
using std::strncpy;
using std::time_t;
using std::tm;
using std::time_t;

// Save game compression level
extern int save_compression;

/*
 *	Write out the gamedat directory from a saved game.
 *
 *	Output: Aborts if error.
 */

void Game_window::restore_gamedat
	(
	const char *fname		// Name of savegame file.
	)
	{
					// Check IDENTITY.
	char *id = get_game_identity(fname);
	char *static_identity = get_game_identity(INITGAME);
					// Note: "*" means an old game.
	if(!id || (*id != '*' && strcmp(static_identity, id) != 0))
		{
		std::string msg("Wrong identity '");
		msg += id; msg += "'.  Open anyway?";
		int ok = Yesno_gump::ask(msg.c_str());
		if (!ok)
			return;
		}
	// Check for a ZIP file first
#ifdef HAVE_ZIP_SUPPORT
	if (restore_gamedat_zip(fname) != false)
		return;
#endif

	ifstream in_stream;

#ifdef RED_PLASMA
	// Display red plasma during load...
	setup_load_palette();
#endif
									
	U7mkdir("<GAMEDAT>", 0755);		// Create dir. if not already there. Don't
									// use GAMEDAT define cause that's got a
									// trailing slash
	try
		{
		U7open(in_stream, fname);	// Open file; throws an exception 
		}
	catch(const file_exception & f)
		{
		if (!Game::is_editing())	// Ok if map-editing.
			throw f;
		std::cerr << "Warning (map-editing): Couldn't open '" << 
							fname << "'" << endl;
		return;
		}

	StreamDataSource in(&in_stream);

	U7remove (USEDAT);
	U7remove (USEVARS);
	U7remove (U7NBUF_DAT);
	U7remove (NPC_DAT);
	U7remove (MONSNPCS);
	U7remove (FLAGINIT);
	U7remove (GWINDAT);
	U7remove (IDENTITY);
	U7remove (GSCHEDULE);
	U7remove ("<STATIC>/flags.flg");
	U7remove (GSCRNSHOT);
	U7remove (GSAVEINFO);
	U7remove (GNEWGAMEVER);
	U7remove (GEXULTVER);
	U7remove (KEYRINGDAT);

	cout.flush();

	in.seek(0x54);			// Get to where file count sits.
	int numfiles = in.read4();
	in.seek(0x80);			// Get to file info.
					// Read pos., length of each file.
	long *finfo = new long[2*numfiles];
	int i;
	for (i = 0; i < numfiles; i++)
		{
		finfo[2*i] = in.read4();	// The position, then the length.
		finfo[2*i + 1] = in.read4();
		}
	for (i = 0; i < numfiles; i++)	// Now read each file.
		{
					// Get file length.
		int len = finfo[2*i + 1] - 13;
		if (len <= 0)
			continue;
		in.seek(finfo[2*i]);	// Get to it.
		char fname[50];		// Set up name.
		strcpy(fname, GAMEDAT);
		in.read(&fname[sizeof(GAMEDAT) - 1], 13);
		int namelen = strlen(fname);
					// Watch for names ending in '.'.
		if (fname[namelen - 1] == '.')
			fname[namelen - 1] = 0;
		ofstream out;
		U7open(out, fname);
					// Now read the file.
		char *buf = new char[len];
		in.read(buf, len);
		out.write(buf, len);	// Then write it out.
		delete [] buf;
		if (!out.good())
			abort("Error writing '%s'.", fname);
		out.close();
		CYCLE_RED_PLASMA();
		}
	delete [] finfo;
	cout.flush();

#ifdef RED_PLASMA
	load_palette_timer = 0;
#endif
	}

/*
 *	Write out the gamedat directory from a saved game.
 *
 *	Output: Aborts if error.
 */

void Game_window::restore_gamedat
	(
	int num				// 0-9, currently.
	)
	{
	char fname[50];			// Set up name.
	snprintf(fname, 50, SAVENAME, num, 
		Game::get_game_type() == BLACK_GATE ? "bg" : "si");
	restore_gamedat(fname);
	}

/*
 *	List of 'gamedat' files to save (in addition to 'iregxx'):
 */
static const char *bgsavefiles[] = {
	GSCRNSHOT,	GSAVEINFO,	// MUST BE FIRST!!
	IDENTITY,			// MUST BE #2
	GEXULTVER,	GNEWGAMEVER,
	NPC_DAT,	MONSNPCS,
	USEVARS,	USEDAT,
	FLAGINIT,	GWINDAT,
	GSCHEDULE
	};
static const int bgnumsavefiles = sizeof(bgsavefiles)/sizeof(bgsavefiles[0]);

static const char *sisavefiles[] = {
	GSCRNSHOT,	GSAVEINFO,	// MUST BE FIRST!!
	IDENTITY,			// MUST BE #2
	GEXULTVER,	GNEWGAMEVER,
	NPC_DAT,	MONSNPCS,
	USEVARS,	USEDAT,
	FLAGINIT,	GWINDAT,
	GSCHEDULE,	KEYRINGDAT
	};
static const int sinumsavefiles = sizeof(sisavefiles)/sizeof(sisavefiles[0]);

/*
 *	Save a single file into an IFF repository.
 *
 *	Output:	Length of data saved.
 *		Errors reported.
 */

static long Savefile
	(
	ostream& out,			// Write here.
	const char *fname			// Name of file to save.
	)
	{
	ifstream in_stream;
	try {
		U7open(in_stream, fname);
	} catch (exult_exception& e) {
		if (Game::is_editing())
			return 0;	// Newly developed game.
		throw e;
	}
	StreamDataSource in(&in_stream);
	long len = in.getSize();
	in.seek(0);
	char namebuf[13];		// First write 13-byte name.
	memset(namebuf, 0, sizeof(namebuf));
	const char *base = strchr(fname, '/');// Want the base name.
	if (!base)
		base = strchr(fname, '\\');
	if (base)
		base++;
	else
		base = fname;
	strncpy(namebuf, base, sizeof(namebuf));
	out.write(namebuf, sizeof(namebuf));
	char *buf = new char[len];	// Get it all at once.
	in.read(buf, len);
	out.write(buf, len);
	delete [] buf;
	if (!in_stream.good())
		throw file_read_exception(fname);
	return len + 13;		// Include filename.
	}

static long SavefileFromDataSource(
	ostream& out,		// write here
	DataSource& source,	// read from here
	const char *fname	// store data using this filename
)
{
	long len = source.getSize();
	char namebuf[13];
	memset(namebuf, 0, sizeof(namebuf));
	strncpy(namebuf, fname, sizeof(namebuf));
	out.write(namebuf, sizeof(namebuf));
	char *buf = new char[len];
	source.read(buf, len);
	out.write(buf, len);
	delete [] buf;
	return len + 13;
}

/*
 *	Save 'gamedat' into a given file.
 *
 *	Output:	0 if error (reported).
 */

void Game_window::save_gamedat
	(
	const char *fname,			// File to create.
	const char *savename			// User's savegame name.
	)
	{

		// First check for compressed save game
#ifdef HAVE_ZIP_SUPPORT
	if (save_compression > 0 && save_gamedat_zip(fname, savename) != false)
		return;
#endif

	// setup correct file list 
	int numsavefiles = (Game::get_game_type() == BLACK_GATE) ?
			bgnumsavefiles : sinumsavefiles;
	const char **savefiles = (Game::get_game_type() == BLACK_GATE) ?
			bgsavefiles : sisavefiles;	

	ofstream out;
	U7open(out, fname);
					// Doing all IREG's + what's listed.
	int count = 12*12 + numsavefiles;
					// Use samename for title.
	Flex_writer flex(out, savename, count);
	int i;				// Start with listed files.
	for (i = 0; i < numsavefiles; i++)
		{
		Savefile(out, savefiles[i]);
		flex.mark_section_done();
		}
					// Now the Ireg's.
	for (int schunk = 0; schunk < 12*12; schunk++, i++)
		{
		char iname[80];
		Savefile(out, map->get_schunk_file_name(U7IREG,
							schunk, iname));
		flex.mark_section_done();
		}
	bool result = flex.close();	// Write it all out.
	if (!result)			// ++++Better error system needed??
		throw file_write_exception(fname);
	return;
	}

/*
 *	Save to one of the numbered savegame files (and update save_names).
 *
 *	Output:	false if error (reported).
 */

void Game_window::save_gamedat
	(
	int num,			// 0-9, currently.
	const char *savename			// User's savegame name.
	)
	{
	char fname[50];			// Set up name.
	snprintf(fname, 50, SAVENAME, num,
		Game::get_game_type() == BLACK_GATE ? "bg" : "si");
	save_gamedat(fname, savename);
	if (num >=0 && num < 10)
	{
		delete [] save_names[num];	// Update name
		save_names[num] = newstrdup(savename);
	}
	}

/*
 *	Read in the saved game names.
 */
void Game_window::read_save_names
	(
	)
{
	for (size_t i = 0; i < sizeof(save_names)/sizeof(save_names[0]); i++)
	{
		char fname[50];		// Set up name.
		snprintf(fname, 50, SAVENAME, i,
			Game::get_game_type() == BLACK_GATE ? "bg" : "si");
		ifstream in;
		try
		{
			U7open(in, fname);
			char buf[0x50];		// It's at start of file.
			memset(buf, 0, sizeof(buf));
			in.read(buf, sizeof(buf) - 1);
			if (in.good())		// Okay if file not there.
				save_names[i] = newstrdup(buf);
			else
				save_names[i] = newstrdup("");
			in.close();
		}
		catch(const file_exception & f)
		{
			save_names[i] = newstrdup("");
		}
	}
}


void Game_window::write_saveinfo()
{
	ofstream out_stream;
	int	i, j;

	int save_count = 1;

	try
	{
		ifstream in;
		U7open(in, GSAVEINFO);		// Open file; throws an exception 

		StreamDataSource ds(&in);
		ds.skip(10);	// Skip 10 bytes.
		save_count += ds.read2();

		in.close();
	}
	catch(const file_exception & f)
	{
	}

	int party_size = party_man->get_count()+1;

	time_t t = std::time(0);
	struct tm *timeinfo = localtime (&t);	

	U7open(out_stream, GSAVEINFO);		// Open file; throws an exception - Don't care
	StreamDataSource out(&out_stream);

	// This order must match struct SaveGame_Details

	// Time that the game was saved
	out.write1(timeinfo->tm_min);
	out.write1(timeinfo->tm_hour);
	out.write1(timeinfo->tm_mday);
	out.write1(timeinfo->tm_mon+1);
	out.write2(timeinfo->tm_year + 1900);

	// The Game Time that the save was done at
	out.write1(clock->get_minute());
	out.write1(clock->get_hour());
	out.write2(clock->get_day());

	out.write2(save_count);
	out.write1(party_size);

	out.write1(0);			// Unused

	out.write1(timeinfo->tm_sec);	// 15

	// Packing for the rest of the structure
	for (j = reinterpret_cast<long>(&(((SaveGame_Details *)0)->reserved0)); j < sizeof(SaveGame_Details); j++)
		out.write1(0);

	for (i=0; i<party_size ; i++)
	{
		Actor *npc;
		if (i == 0)
			npc = main_actor;
		else
			npc = (Npc_actor *)get_npc(party_man->get_member(i-1));

		char name[18];
		std::string namestr = npc->get_npc_name();
		strncpy (name, namestr.c_str(), 18);
		out.write(name, 18);
		out.write2(npc->get_shapenum());

		out.write4(npc->get_property(Actor::exp));
		out.write4(npc->get_flags());
		out.write4(npc->get_flags2());

		out.write1(npc->get_property(Actor::food_level));
		out.write1(npc->get_property(Actor::strength));
		out.write1(npc->get_property(Actor::combat));
		out.write1(npc->get_property(Actor::dexterity));
		out.write1(npc->get_property(Actor::intelligence));
		out.write1(npc->get_property(Actor::magic));
		out.write1(npc->get_property(Actor::mana));
		out.write1(npc->get_property(Actor::training));

		out.write2(npc->get_property(Actor::health));
		out.write2(npc->get_shapefile());

		// Packing for the rest of the structure
		for (j = reinterpret_cast<long>(&(((SaveGame_Party *)0)->reserved1)); j < sizeof(SaveGame_Party); j++)
			out.write1(0);
	}

	out_stream.close();

	// Save Shape
	Shape_file *map = create_mini_screenshot();
	U7open(out_stream, GSCRNSHOT);		// Open file; throws an exception - Don't care
	map->save(&out);
	out_stream.close();
	delete map;

	// Current Exult version

	U7open(out_stream, GEXULTVER);
	getVersionInfo(out_stream);
	out_stream.close();

	// Exult version that started this game
	if (!U7exists(GNEWGAMEVER)) {
		U7open(out_stream, GNEWGAMEVER);
		out_stream << "Unknown" << endl;
		out_stream.close();
	}
}


void Game_window::read_saveinfo(DataSource *in,
		SaveGame_Details *&details,
		SaveGame_Party *& party)
{
	int i;
	details = new SaveGame_Details;

	// This order must match struct SaveGame_Details
	// Time that the game was saved
	details->real_minute = in->read1();
	details->real_hour = in->read1();
	details->real_day = in->read1();
	details->real_month = in->read1();
	details->real_year = in->read2();
	

	// The Game Time that the save was done at
	details->game_minute = in->read1();
	details->game_hour = in->read1();
	details->game_day = in->read2();

	details->save_count = in->read2();
	details->party_size = in->read1();

	details->unused = in->read1();	// Unused

	details->real_second = in->read1();	// 15

	// Packing for the rest of the structure
	in->skip(sizeof(SaveGame_Details) - reinterpret_cast<long>(&(((SaveGame_Details *)0)->reserved0)));

	party = new SaveGame_Party[details->party_size];
	for (i=0; i<8 && i<details->party_size ; i++)
	{
		in->read(party[i].name, 18);
		party[i].shape = in->read2();

		party[i].exp = in->read4();
		party[i].flags = in->read4();
		party[i].flags2 = in->read4();

		party[i].food = in->read1();
		party[i].str = in->read1();
		party[i].combat = in->read1();
		party[i].dext = in->read1();
		party[i].intel = in->read1();
		party[i].magic = in->read1();
		party[i].mana = in->read1();
		party[i].training = in->read1();

		party[i].health = in->read2();
		party[i].shape_file = in->read2();

		// Packing for the rest of the structure
		in->skip (sizeof(SaveGame_Party) -  reinterpret_cast<long>(&(((SaveGame_Party *)0)->reserved1)));
	}
}

bool Game_window::get_saveinfo(int num, char *&name, Shape_file *&map, SaveGame_Details *&details, SaveGame_Party *& party)
{
	char fname[50];			// Set up name.
	snprintf(fname, 50, SAVENAME, num, 
		Game::get_game_type() == BLACK_GATE ? "bg" : "si");

	// First check for compressed save game
#ifdef HAVE_ZIP_SUPPORT
	if (get_saveinfo_zip(fname, name, map, details, party) != false)
		return true;
#endif

	ifstream in_stream;
	U7open(in_stream, fname);		// Open file; throws an exception 
	StreamDataSource in(&in_stream);
					// in case of an error.
	// Always try to Read Name
	char buf[0x50];
	memset(buf, 0, sizeof(buf));
	in.read(buf, sizeof(buf) - 1);
	name = new char [strlen (buf)+1];
	strcpy (name, buf);

	// Isn't a flex, can't actually read it
	if (!Flex::is_flex(&in)) return false;

	// Now get dir info
	in.seek(0x54);			// Get to where file count sits.
	int numfiles = in.read4();
	in.seek(0x80);			// Get to file info.
					// Read pos., length of each file.
	long *finfo = new long[2*numfiles];
	int i;
	for (i = 0; i < numfiles; i++)
	{
		finfo[2*i] = in.read4();	// The position, then the length.
		finfo[2*i + 1] = in.read4();
	}

	// Always first two entires
	for (i = 0; i < 2; i++)	// Now read each file.
	{
					// Get file length.
		int len = finfo[2*i + 1] - 13;
		if (len <= 0)
			continue;
		in.seek(finfo[2*i]);	// Get to it.
		char fname[50];		// Set up name.
		strcpy(fname, GAMEDAT);
		in.read(&fname[sizeof(GAMEDAT) - 1], 13);
		int namelen = strlen(fname);
					// Watch for names ending in '.'.
		if (fname[namelen - 1] == '.')
			fname[namelen - 1] = 0;

		if (!strcmp (fname, GSCRNSHOT))
		{
			char *buf = new char[len];
			in.read(buf, len);
			BufferDataSource ds(buf, len);
			map = new Shape_file(&ds);
			delete [] buf;
		}
		else if (!strcmp (fname, GSAVEINFO))
		{
			read_saveinfo (&in, details, party);
		}

	}
	in_stream.close();

	delete [] finfo;

	return true;
}

void Game_window::get_saveinfo(Shape_file *&map, SaveGame_Details *&details, SaveGame_Party *& party)
{
	try
	{
		ifstream in;
		U7open(in, GSAVEINFO);		// Open file; throws an exception 
		StreamDataSource ds(&in);
		read_saveinfo (&ds, details, party);
		in.close();
	}
	catch(const file_exception & f)
	{
		details = NULL;
		party = NULL;
	}

	try
	{
		ifstream in;
		U7open(in, GSCRNSHOT);		// Open file; throws an exception 
		StreamDataSource ds(&in);
		map = new Shape_file(&ds);
		in.close();
	}
	catch(const file_exception & f)
	{
		// yes, this is weird, but seems to work-around a compiler
		// problem... (gcc-2.95.2-12mdk)    -wjp
		map = 0; map = 0;
	}
}

/*
 *	Return string from IDENTITY in a savegame.
 *
 *	Output:	->identity if found.
 *		0 if error (or may throw exception).
 *		"*" if older savegame.
 */
char *Game_window::get_game_identity(const char *savename)
{
    char *game_identity = 0;
#ifdef HAVE_ZIP_SUPPORT
    game_identity = get_game_identity_zip(savename);
    if (game_identity)
	return game_identity;
#endif
    ifstream in_stream;
    try {
        U7open(in_stream, savename);		// Open file.
    } catch (const exult_exception &e) {
	if (Game::is_editing()) {	// Okay if creating a new game.
		std::string titlestr = Game::get_gametitle();
		return newstrdup(titlestr.c_str());
	}
	throw e;
    }
	StreamDataSource in(&in_stream);

    in.seek(0x54);			// Get to where file count sits.
    int numfiles = in.read4();
    in.seek(0x80);			// Get to file info.
    // Read pos., length of each file.
    sint32 *finfo = new sint32[2*numfiles];
    int i;
    for (i = 0; i < numfiles; i++)
      {
	finfo[2*i] = in.read4();	// The position, then the length.
	finfo[2*i + 1] = in.read4();
      }
    for (i = 0; i < numfiles; i++)	// Now read each file.
      {
	// Get file length.
	int len = finfo[2*i + 1] - 13;
	if (len <= 0)
	  continue;
	in.seek(finfo[2*i]);	// Get to it.
	char fname[50];		// Set up name.
	in.read(fname, 13);
	if (!strcmp("identity",fname))
	    {
      	      game_identity = new char[len];
	      in.read(game_identity, len);
	      // Truncate identity
	      char *ptr = game_identity;
	      for(; (*ptr!=0x1a && *ptr!=0x0d); ptr++)
	      	;
	      *ptr = 0;
	      break;
	    }
      }
    delete [] finfo;
    return game_identity;
}

// Zip file support
#ifdef HAVE_ZIP_SUPPORT

#include "files/zip/unzip.h"
#include "files/zip/zip.h"

static const char *remove_dir(const char *fname)
{
	const char *base = strchr(fname, '/');// Want the base name.
	if (!base)
		base = strchr(fname, '\\');
	if (base)
		return base+1;

	return fname;
}


bool Game_window::get_saveinfo_zip(const char *fname, char *&name, Shape_file *&map, SaveGame_Details *&details, SaveGame_Party *& party)
{
	// If a flex, so can't read it
	if (Flex::is_flex(fname)) return false;

	std::string filestr = get_system_path(fname);
	unzFile unzipfile = unzOpen(filestr.c_str());
	if (!unzipfile) return false;

	// Name comes from comment
	char namebuf[0x50];
	if (unzGetGlobalComment(unzipfile, namebuf, 0x50) <= 0) strncpy (namebuf, "UNNAMED", 0x50);
	name = new char [strlen (namebuf)+1];
	strcpy (name, namebuf);


	// Things we need
	unz_file_info file_info;
	char *buf = 0;

	// Get the screenshot first
	if (unzLocateFile(unzipfile, remove_dir(GSCRNSHOT), 2) == UNZ_OK)
	{
		unzGetCurrentFileInfo(unzipfile, &file_info, NULL, 0, NULL, 0, NULL, 0);
		buf = new char[file_info.uncompressed_size];

		unzOpenCurrentFile(unzipfile);
		unzReadCurrentFile(unzipfile, buf, file_info.uncompressed_size);
		if (unzCloseCurrentFile(unzipfile) == UNZ_OK)
		{
			BufferDataSource ds(buf, file_info.uncompressed_size);
			map = new Shape_file(&ds);
		}

		delete [] buf;
	}

	// Now saveinfo
	if (unzLocateFile(unzipfile, remove_dir(GSAVEINFO), 2) == UNZ_OK)
	{
		unzGetCurrentFileInfo(unzipfile, &file_info, NULL, 0, NULL, 0, NULL, 0);
		buf = new char[file_info.uncompressed_size];

		unzOpenCurrentFile(unzipfile);
		unzReadCurrentFile(unzipfile, buf, file_info.uncompressed_size);
		if (unzCloseCurrentFile(unzipfile) == UNZ_OK)
		{
			BufferDataSource ds(buf, file_info.uncompressed_size);
			read_saveinfo (&ds, details, party);
		}

		delete [] buf;
	}

	unzClose (unzipfile);

	return true;
}


// Level 2 Compression
bool Game_window::Restore_level2 (void *uzf)
{
	unzFile unzipfile = static_cast<unzFile>(uzf);

	char oname[50];		// Set up name.
	char *oname2 = oname+sizeof(GAMEDAT) - 1;		// Set up name.
	char size_buffer[4];
	int size;
	strcpy(oname, GAMEDAT);

	if (unzOpenCurrentFile(unzipfile) != UNZ_OK)
	{
		std::cerr << "Couldn't open current file" << std::endl;
		return false;
	}

	while (!unzeof(unzipfile))
	{
		// Read Filename
		oname2[12] = 0;
		if (unzReadCurrentFile(unzipfile, oname2, 12) != 12)
		{
			std::cerr << "Couldn't read for filename" << std::endl;
			return false;
		}

		// Check to see if was are at the end of the list
		if (*oname2 == 0) break;

		// Get file length.
		if (unzReadCurrentFile(unzipfile, size_buffer, 4) != 4)
		{
			std::cerr << "Couldn't read for size" << std::endl;
			return false;
		}
		BufferDataSource ds(size_buffer, 4);
		size = ds.read4();

		if (size)
		{
			// Watch for names ending in '.'.
			int namelen = strlen(oname);
			if (oname[namelen - 1] == '.')
				oname[namelen - 1] = 0;


			// Now read the file.
			char *buf = new char[size];
			if (unzReadCurrentFile(unzipfile, buf, size) != size)
			{
				delete [] buf;
				std::cerr << "Couldn't read for buf" << std::endl;
				return false;
			}

			// Then write it out.
			ofstream out;
			U7open(out, oname);
			out.write(buf, size);

			delete [] buf;
			if (!out.good())
			{
				std::cerr << "out was bad" << std::endl;
				return false;
			}
			out.close();
			CYCLE_RED_PLASMA();
		}
	}

	return unzCloseCurrentFile(unzipfile) == UNZ_OK;
}


/*
 *	Write out the gamedat directory from a saved game.
 *
 *	Output: Aborts if error.
 */

bool Game_window::restore_gamedat_zip
	(
	const char *fname		// Name of savegame file.
	)
	{
	// If a flex, so can't read it
	try
		{
		if (Flex::is_flex(fname)) return false;
		}
	catch(const file_exception & f)
		{
		return false;		// Ignore if not found.
		}
#ifdef RED_PLASMA
	// Display red plasma during load...
	setup_load_palette();
#endif
	std::string filestr = get_system_path(fname);
	unzFile unzipfile = unzOpen(filestr.c_str());
	if (!unzipfile) return false;

	U7mkdir("<GAMEDAT>", 0755);		// Create dir. if not already there. Don't
									// use GAMEDAT define cause that's got a
									// trailing slash
	U7remove (USEDAT);
	U7remove (USEVARS);
	U7remove (U7NBUF_DAT);
	U7remove (NPC_DAT);
	U7remove (MONSNPCS);
	U7remove (FLAGINIT);
	U7remove (GWINDAT);
	U7remove (IDENTITY);
	U7remove (GSCHEDULE);
	U7remove ("<STATIC>/flags.flg");
	U7remove (GSCRNSHOT);
	U7remove (GSAVEINFO);
	U7remove (GNEWGAMEVER);
	U7remove (GEXULTVER);
	U7remove (KEYRINGDAT);

	cout.flush();

	unz_global_info	global;
	unzGetGlobalInfo(unzipfile, &global);

	// Now read each file.
	char oname[50];		// Set up name.
	char *oname2 = oname + sizeof(GAMEDAT) - 1;
	strcpy(oname, GAMEDAT);

	do
	{
		unz_file_info	file_info;
	
		unzGetCurrentFileInfo(unzipfile, &file_info,
			oname2, 13,
			NULL, 0,
			NULL, 0);

		// Get file length.
		int len = file_info.uncompressed_size;
		if (len <= 0)
			continue;

		// Level 2 compression handling
		if (!std::strcmp("GAMEDAT", oname2))
		{
			if (Restore_level2(unzipfile) == false)
				abort("Error reading level2 from zip '%s'.", fname);

			continue;
		}

					// Watch for names ending in '.'.
		int namelen = strlen(oname);
		if (oname[namelen - 1] == '.')
			oname[namelen - 1] = 0;


					// Open the file in the zip
		if (unzOpenCurrentFile(unzipfile) != UNZ_OK)
			abort("Error opening current from zipfile '%s'.", fname);

					// Now read the file.
		char *buf = new char[len];
		if (unzReadCurrentFile(unzipfile, buf, len) != len)
			abort("Error reading current from zip '%s'.", fname);

					// now write it out.
		ofstream out;
		U7open(out, oname);
		out.write(buf, len);
		if (!out.good()) abort("Error writing to '%s'.", oname);
		out.close();

					// Close the file in the zip
		if (unzCloseCurrentFile(unzipfile) != UNZ_OK)
			abort("Error closing current in zip '%s'.", fname);
		delete [] buf;

		CYCLE_RED_PLASMA();
	}
	while (unzGoToNextFile(unzipfile) == UNZ_OK);

	unzClose(unzipfile);

	cout.flush();

#ifdef RED_PLASMA
	load_palette_timer = 0;
#endif

	return true;
	}


// Level 1 Compression
static bool Save_level1 (zipFile zipfile, const char *fname)
{
	ifstream in;
	try {
		U7open (in, fname);
	} catch (exult_exception& e) {
		if (Game::is_editing())
			return false;	// Newly developed game.
		throw e;
	}


	StreamDataSource ds(&in);

	unsigned int size = ds.getSize();
	char *buf = new char[size];
	ds.read(buf, size);
	

	zipOpenNewFileInZip (zipfile, remove_dir(fname), NULL, NULL, 0,
				NULL, 0, NULL, Z_DEFLATED, Z_BEST_COMPRESSION);

	zipWriteInFileInZip (zipfile, buf, size);
	delete [] buf;

	return zipCloseFileInZip (zipfile) == ZIP_OK;
}

// Level 2 Compression
static bool Begin_level2 (zipFile zipfile)
{
	return zipOpenNewFileInZip (zipfile, "GAMEDAT", NULL, NULL, 0,
				NULL, 0, NULL, Z_DEFLATED, Z_BEST_COMPRESSION) == ZIP_OK;
}

static bool Save_level2 (zipFile zipfile, const char *fname)
{
	ifstream in;
	try {
		U7open (in, fname);
	} catch (exult_exception& e) {
		if (Game::is_editing())
			return false;	// Newly developed game.
		throw e;
	}

	StreamDataSource ds(&in);

	uint32 size = ds.getSize();
	char *buf = new char[size<13?13:size]; // We want at least 13 bytes
	
	// Filename first
	memset (buf, 0, 13);
	strncpy (buf, remove_dir(fname), 13);
	int err = zipWriteInFileInZip (zipfile, buf, 12);

	// Size of the file
	if (err == ZIP_OK)
	{
		// Must be platform independant
		BufferDataSource bds(buf, 4);
		bds.write4(size);
		err = zipWriteInFileInZip (zipfile, buf, 4);
	}

	// Now the actual file
	if (err == ZIP_OK)
	{
		ds.read(buf, size);
		err = zipWriteInFileInZip (zipfile, buf, size);
	}

	delete [] buf;

	return err == ZIP_OK;
}

static bool End_level2 (zipFile zipfile)
{
	uint32 zeros = 0;

	// Write a terminator (12 zeros)
	int err = zipWriteInFileInZip (zipfile, &zeros, 4);
	if (err == ZIP_OK) err = zipWriteInFileInZip (zipfile, &zeros, 4);
	if (err == ZIP_OK) err = zipWriteInFileInZip (zipfile, &zeros, 4);

	return zipCloseFileInZip (zipfile) == ZIP_OK;
}


bool Game_window::save_gamedat_zip
	(
	const char *fname,			// File to create.
	const char *savename			// User's savegame name.
	)
{
	// If no compression return
	if (save_compression < 1) return false;

	// setup correct file list 
	int numsavefiles = (Game::get_game_type() == BLACK_GATE) ?
			bgnumsavefiles : sinumsavefiles;
	const char **savefiles = (Game::get_game_type() == BLACK_GATE) ?
			bgsavefiles : sisavefiles;	

	// Name
	{
		ofstream out;
		char title[0x50];
		memset (title, 0, 0x50);
		std::strncpy (title, savename, 0x50);
		U7open(out, fname);
		out.write(title, 0x50);
		out.close();
	}
	
	std::string filestr = get_system_path(fname);
	zipFile zipfile = zipOpen(filestr.c_str(), 1);

	// Level 1 Compression
	if (save_compression != 2)
	{
		for (int i = 0; i < numsavefiles; i++)
			Save_level1(zipfile, savefiles[i]);

		// Now the Ireg's.
		for (int schunk = 0; schunk < 12*12; schunk++)
		{
			char iname[80];
			Save_level1(zipfile, 
			    map->get_schunk_file_name(U7IREG, schunk, iname));
		}
	}
	// Level 2 Compression
	else
	{
		// Keep saveinfo, screenshot, identity using normal compression
		// There are always files 0 - 2
		Save_level1(zipfile, GSCRNSHOT);
		Save_level1(zipfile, GSAVEINFO);
		Save_level1(zipfile, IDENTITY);

		Begin_level2(zipfile);

		for (int i = 3; i < numsavefiles; i++)
			Save_level2(zipfile, savefiles[i]);

		// Now the Ireg's.
		for (int schunk = 0; schunk < 12*12; schunk++)
		{
			char iname[80];
			Save_level2(zipfile, map->get_schunk_file_name(
						U7IREG, schunk, iname));
		}

		End_level2(zipfile);
	}

	// ++++Better error system needed??
	if (zipClose(zipfile, savename) != ZIP_OK)
		throw file_write_exception(fname);

	return true;
}

/*
 *	Return string from IDENTITY in a savegame.
 *
 *	Output:	->identity string.
 *		0 if error.
 *		"*" if not found.
 */
char *Game_window::get_game_identity_zip
	(
	const char *savename
	)
	{
	// If a flex, so can't read it
	try
		{
		if (Flex::is_flex(savename)) 
			return 0;
		}
	catch(const file_exception & f)
		{
		return 0;		// Ignore if not found.
		}
	unzFile unzipfile = unzOpen(get_system_path(savename).c_str());
	if (!unzipfile) 
		return 0;
					// Find IDENTITY, ignoring case.
	if (unzLocateFile(unzipfile, "identity", 2) != UNZ_OK)
		{
		unzClose(unzipfile);
		return "*";		// Old game.  Return wildcard.
		}
					// Open the file in the zip
	if (unzOpenCurrentFile(unzipfile) != UNZ_OK)
		{
		unzClose(unzipfile);
		throw file_read_exception(savename);
		}
					// Now read the file.
	char buf[256];
	int cnt = unzReadCurrentFile(unzipfile, buf, sizeof(buf) - 1);
	if (cnt <= 0)
		{
		unzCloseCurrentFile(unzipfile);
		unzClose(unzipfile);
		throw file_read_exception(savename);
		}
	buf[cnt] = 0;			// 0-delimit.
	unzCloseCurrentFile(unzipfile);
	unzClose(unzipfile);
	char *ptr = buf;
	for(; (*ptr != 0 && *ptr!=0x1a && *ptr!=0x0d); ptr++)
	      	;
	*ptr = 0;
	return newstrdup(buf);
	}

#endif
