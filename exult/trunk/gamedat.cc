/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Gamedat.cc - Create gamedat files from a savegame.
 **
 **	Written: 2/4/00 - JSF
 **/
/*
Copyright (C) 2000  Jeffrey S. Freedman

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

#ifdef WIN32
#include <io.h>
#endif

#include <fstream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if (defined(XWIN) || defined(BEOS))
#include <sys/stat.h>
#elif defined(MACOS)
#include <stat.h>
#endif

#include "exceptions.h"
#include "fnames.h"
#include "gamewin.h"
#include "utils.h"

using std::cerr;
using std::cout;
using std::endl;
using std::ios;
using std::ostream;

/*
 *	Write out the gamedat directory from a saved game.
 *
 *	Output: Aborts if error.
 */

void Game_window::restore_gamedat
	(
	const char *fname			// Name of savegame file.
	)
	{
	ifstream in;
	U7open(in, fname);		// Open file; throws an exception in case of an error.
#ifdef WIN32
	mkdir("gamedat");
#else
	mkdir("gamedat", 0755);		// Create dir. if not already there.
#endif

	U7remove (USEDAT);
	U7remove (U7NBUF_DAT);
	U7remove (NPC_DAT);
	U7remove (MONSNPCS);
	U7remove (FLAGINIT);
	U7remove (GWINDAT);
	U7remove (IDENTITY);
	U7remove ("<STATIC>/flags.flg");

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
	sprintf(fname, SAVENAME, num);
	restore_gamedat(fname);
	}

/*
 *	List of 'gamedat' files to save (in addition to 'iregxx'):
 */
static const char *savefiles[] = {
	NPC_DAT,	MONSNPCS,
	IDENTITY,	USEDAT,
	FLAGINIT,	GWINDAT
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
	char title[0x50];		// Use savename for title.
	memset(title, 0, sizeof(title));
	strncpy(title, savename, sizeof(title) - 1);
	out.write(title, sizeof(title));
	Write4(out, 0xFFFF1A00);	// Magic number.
					// Doing all IREG's + what's listed.
	int count = 12*12 + numsavefiles;
	Write4(out, count);
	Write4(out, 0x000000CC);	// 2nd magic number.
					// Create table.
	unsigned char *table = new unsigned char[2*count*4];
	unsigned char *tptr = table;
	long pos = out.tellp();		// Fill to data (past table at 0x80).
	long fill = 0x80 + 8*count - pos;
	while (fill--)
		out.put((char) 0);
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
		long len = Savefile(out, get_ireg_name(schunk, iname));
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
	sprintf(fname, SAVENAME, num);
	save_gamedat(fname, savename);
	free((void*) save_names[num]);		// Update name.
	save_names[num] = strdup(savename);
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
		sprintf(fname, SAVENAME, i);
		ifstream in;
		try
		{
			U7open(in, fname);
			char buf[0x50];		// It's at start of file.
			memset(buf, 0, sizeof(buf));
			in.read(buf, sizeof(buf) - 1);
			if (in.good())		// Okay if file not there.
				save_names[i] = strdup(buf);
			else
				save_names[i] = strdup("");
			in.close();
		}
		catch(...)
		{
			save_names[i] = strdup("");
		}
	}
}
