/**
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

#include <iostream.h>
#include <string.h>
#include <sys/stat.h>
#if !defined(XWIN)
#include <dir.h>
#endif
#include "gamewin.h"
#include "fnames.h"
#include "utils.h"

/*
 *	Write out the gamedat directory from a saved game.
 *
 *	Output: Aborts if error.
 */

void Game_window::write_gamedat
	(
	char *savename			// Name of savegame file.
	)
	{
	ifstream in;
	u7open(in, savename);		// Open file & abort if error.
#if defined(XWIN)
	mkdir("gamedat", 0755);		// Create dir. if not already there.
#else
	mkdir("gamedat");
#endif
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
		ofstream out(fname, ios::out + ios::trunc + ios::binary);
					// Now read the file.
		char *buf = new char[len];
		in.read(buf, len);
		out.write(buf, len);	// Then write it out.
		delete buf;
		if (!out.good())
			abort("Error writing '%s'.", fname);
		out.close();
		}
	delete [] finfo;
	}


char *Game_window::get_game_identity
		 (
		  char *savename
		  )
{
    ifstream in;
    u7open(in, savename);		// Open file & abort if error.
    in.seekg(0x54);			// Get to where file count sits.
    int numfiles = Read4(in);
    char *game_identity = 0;
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
	in.read(&fname, 13);
	int namelen = strlen(fname);
	if (!strcmp("identity",fname))
	    {
      	      game_identity = new char[len];
	      in.read(game_identity, len);
	      // Truncate identity
	      char *ptr = game_identity;
	      for(; (*ptr!=0x1a && *ptr!=0x0d); ptr++);
	      *ptr = 0;
	      break;
	    }
      }
    delete [] finfo;
    return game_identity;
}
