/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Gamedat.cc - Create gamedat files from a savegame.
 **
 **	Written: 2/4/00 - JSF
 **/

/*
Copyright (C) 2000-2001 The Exult Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
#include "utils.h"
#include "gump_utils.h"
#include "game.h"
#include "Flex.h"
#include "databuf.h"
#include "Newfile_gump.h"
#include "actors.h"
#include "ucmachine.h"

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
using std::snprintf;
using std::strchr;
using std::strcmp;
using std::strcpy;
using std::strlen;
using std::strncpy;
using std::time_t;
using std::tm;
using std::time_t;

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
	ifstream in;
	U7open(in, fname);		// Open file; throws an exception 
					// in case of an error.
	U7mkdir(GAMEDAT, 0755);		// Create dir. if not already there.

	U7remove (USEDAT);
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

	cout.flush();

	in.seekg(0x54);			// Get to where file count sits.
	int numfiles = Read4(in);
	in.seekg(0x80);			// Get to file info.
					// Read pos., length of each file.
	long *finfo = new long[2*numfiles];
	int i;
	for (i = 0; i < numfiles; i++)
		{
		finfo[2*i] = Read4(in);	// The position, then the length.
		finfo[2*i + 1] = Read4(in);
		}
	for (i = 0; i < numfiles; i++)	// Now read each file.
		{
					// Get file length.
		int len = finfo[2*i + 1] - 13;
		if (len <= 0)
			continue;
		in.seekg(finfo[2*i]);	// Get to it.
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
		}
	delete [] finfo;
	cout.flush();
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
static const char *savefiles[] = {
	GSCRNSHOT,	GSAVEINFO,	// MUST BE FIRST!!
	NPC_DAT,	MONSNPCS,
	IDENTITY,	USEDAT,
	FLAGINIT,	GWINDAT,
	GSCHEDULE
	};
static const int numsavefiles = sizeof(savefiles)/sizeof(savefiles[0]);

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
	ifstream in;
	U7open(in, fname);
	in.seekg(0, ios::end);		// Get to end so we can get length.
	long len = in.tellg();
	in.seekg(0, ios::beg);
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
	if (!in.good())
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
	ofstream out;
	U7open(out, fname);
					// Doing all IREG's + what's listed.
	int count = 12*12 + numsavefiles;
					// Use samename for title.
	Flex::write_header(out, savename, count);
					// Create table.
	unsigned char *table = new unsigned char[2*count*4];
	uint8 *tptr = table;
	int i;				// Start with listed files.
	for (i = 0; i < numsavefiles; i++)
		{
		Write4(tptr, out.tellp());
		long len = Savefile(out, savefiles[i]);
		Write4(tptr, len);
		}
					// Now the Ireg's.
	for (int schunk = 0; schunk < 12*12; schunk++, i++)
		{
		Write4(tptr, out.tellp());
		char iname[80];
		long len = Savefile(out, get_schunk_file_name(U7IREG,
							schunk, iname));
		Write4(tptr, len);
		}
	out.seekp(0x80, ios::beg);	// Write table.
	out.write((char*)table, 2*count*4);
	delete [] table;
	out.flush();
	bool result = out.good();
	if (!result)			// ++++Better error system needed??
		throw file_write_exception(fname);
	out.close();
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
		catch(...)
		{
			save_names[i] = newstrdup("");
		}
	}
}


void Game_window::write_saveinfo()
{
	ofstream out;
	SaveGame_Details	details;
	SaveGame_Party		party[8];

	details.save_count = 0;

	try
	{
		ifstream in;
		U7open(in, GSAVEINFO);		// Open file; throws an exception 
		in.read((char *) &details, sizeof (details));
		in.close();
	}
	catch(...)
	{
	}

	Usecode_machine *uc = get_usecode();

	details.party_size = uc->get_party_count()+1;
	details.save_count++;

	details.game_day = clock.get_day();
	details.game_hour = clock.get_hour();
	details.game_minute = clock.get_minute();

	time_t t = std::time(0);
	struct tm *timeinfo = localtime (&t);	

	details.real_day = timeinfo->tm_mday;
	details.real_hour = timeinfo->tm_hour;
	details.real_minute = timeinfo->tm_min;
	details.real_month = timeinfo->tm_mon+1;
	details.real_year = timeinfo->tm_year + 1900;
	details.real_second = timeinfo->tm_sec;

	U7open(out, GSAVEINFO);		// Open file; throws an exception - Don't care
	out.write((char *) &details, sizeof(details));

	int	i;
	for (i=0; i<8 && i<details.party_size ; i++)
	{
		Actor *npc;
		if (i == 0)
			npc = main_actor;
		else
			npc = (Npc_actor *) get_npc( uc->get_party_member(i-1));

		strncpy (party[i].name, npc->get_npc_name().c_str(), 18);
		party[i].shape = npc->get_shapenum();

		party[i].dext = npc->get_property(Actor::dexterity);
		party[i].str = npc->get_property(Actor::strength);
		party[i].intel = npc->get_property(Actor::intelligence);
		party[i].health = npc->get_property(Actor::health);
		party[i].combat = npc->get_property(Actor::combat);
		party[i].mana = npc->get_property(Actor::mana);
		party[i].magic = npc->get_property(Actor::magic);
		party[i].training = npc->get_property(Actor::training);
		party[i].exp = npc->get_property(Actor::exp);
		party[i].food = npc->get_property(Actor::food_level);
		party[i].flags = npc->get_flags();
		party[i].flags2 = npc->get_flags2();
	}

	out.write((char *) party, sizeof(SaveGame_Party)*details.party_size );
	out.close();

	// Save Shape
	Shape_file *map = create_mini_screenshot();
	U7open(out, GSCRNSHOT);		// Open file; throws an exception - Don't care
	StreamDataSource ds(&out);
	map->save(ds);
	out.close();
	delete map;
}

void Game_window::get_saveinfo(int num, char *&name, Shape_file *&map, SaveGame_Details *&details, SaveGame_Party *& party)
{
	char fname[50];			// Set up name.
	snprintf(fname, 50, SAVENAME, num, 
		Game::get_game_type() == BLACK_GATE ? "bg" : "si");

	ifstream in;
	U7open(in, fname);		// Open file; throws an exception 
					// in case of an error.

	// Read Name
	char buf[0x50];
	memset(buf, 0, sizeof(buf));
	in.read(buf, sizeof(buf) - 1);
	name = new char [strlen (buf)+1];
	strcpy (name, buf);

	// Now get dir info
	in.seekg(0x54);			// Get to where file count sits.
	int numfiles = Read4(in);
	in.seekg(0x80);			// Get to file info.
					// Read pos., length of each file.
	long *finfo = new long[2*numfiles];
	int i;
	for (i = 0; i < numfiles; i++)
	{
		finfo[2*i] = Read4(in);	// The position, then the length.
		finfo[2*i + 1] = Read4(in);
	}

	// Always first two entires
	for (i = 0; i < 2; i++)	// Now read each file.
	{
					// Get file length.
		int len = finfo[2*i + 1] - 13;
		if (len <= 0)
			continue;
		in.seekg(finfo[2*i]);	// Get to it.
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
			map = new Shape_file(ds);
			delete [] buf;
		}
		else if (!strcmp (fname, GSAVEINFO))
		{
			details = new SaveGame_Details;
			in.read((char *) details, sizeof (SaveGame_Details));
			party = new SaveGame_Party[details->party_size];
			in.read((char *) party, sizeof (SaveGame_Party)*details->party_size);
		}

	}
	in.close();

	delete [] finfo;
}

void Game_window::get_saveinfo(Shape_file *&map, SaveGame_Details *&details, SaveGame_Party *& party)
{
	try
	{
		ifstream in;
		U7open(in, GSAVEINFO);		// Open file; throws an exception 
		details = new SaveGame_Details;
		in.read((char *) details, sizeof (SaveGame_Details));
		party = new SaveGame_Party[details->party_size];
		in.read((char *) party, sizeof (SaveGame_Party)*details->party_size);
		in.close();
	}
	catch(...)
	{
		details = NULL;
		party = NULL;
	}

	try
	{
		ifstream in;
		U7open(in, GSCRNSHOT);		// Open file; throws an exception 
		StreamDataSource ds(&in);
		map = new Shape_file(ds);
		in.close();
	}
	catch(...)
	{
		// yes, this is weird, but seems to work-around a compiler
		// problem... (gcc-2.95.2-12mdk)    -wjp
		map = 0; map = 0;
	}
}
