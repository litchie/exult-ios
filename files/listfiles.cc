/*
Copyright (C) 2001 The Exult Team

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

#ifndef ALPHA_LINUX_CXX
#  include <cstdlib>
#  include <cctype>
#  include <cstdio>
#endif

#include <vector>
#include <string>
#include <iostream>

using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using std::free;
using std::malloc;
using std::string;
using std::strcat;
using std::strcpy;
using std::strlen;

#include "utils.h"
#include "listfiles.h"

// System Specific Code for Windows
#if defined(WIN32)

// Need this for _findfirst, _findnext, _findclose
#include <io.h>

int U7ListFiles(const std::string mask, FileList& files)
{
	string			path(get_system_path(mask));
	struct _finddata_t	fileinfo;
	long			handle;
	char			*stripped_path;
	int			i;

	handle = _findfirst (path.c_str(), &fileinfo);

	// 
	stripped_path = new char [path.length()+1];
	strcpy (stripped_path, path.c_str());

	for (i = strlen (stripped_path)-1; i; i--)
		if (stripped_path[i] == '\\' || stripped_path[i] == '/')
			break;

	if (stripped_path[i] == '\\' || stripped_path[i] == '/')
		stripped_path[i+1] = 0;


	// Now search the files
	if (handle != -1)
	{
		do
		{
			char *filename = (char *) malloc (strlen (fileinfo.name)+strlen(stripped_path)+1 );
			strcpy (filename, stripped_path);
			strcat (filename, fileinfo.name);
			files.push_back(filename);
		} while (_findnext( handle, &fileinfo ) != -1);
	}

	delete [] stripped_path;
	_findclose (handle);
	return 0;
}

#elif defined(MACOS)

#include <Files.h>
#include <TextUtils.h>

/*
 *	Match a string with a given pattern (DOS like syntax, using * and ?)
 */

bool	MatchString( const char *str, const std::string& inPat );
bool	MatchString( const char *str, const std::string& inPat )
{
	const char *pat = inPat.c_str();

	const char *p = NULL;
	const char *q = NULL;
	
	for(;;)
	{
		switch(*pat)
		{
		case '*':
			p = ++pat;
			q = str;
			break;

		default:
			if(*pat != *str)
			{
				if(p)
				{
					pat = p;
					str = ++q;
					break;
				}
				else
					return false;
			}
			// fallthrough
		case '?':
			if(!*str)
				return !*pat;
			pat++;
			str++;
		}
	}
}


int U7ListFiles(const std::string mask, FileList& files)
{
	CInfoPBRec		cPB;			// the parameter block used for PBGetCatInfo calls
	Str63			itemName;		// the name of the current item
	OSErr			err;			// temporary holder of results - saves 2 bytes of stack each level
	short			index = 0;
	short			vRefNum;
	long			dirID;
	char			filename[256];
	
	err = HGetVol( NULL, &vRefNum, &dirID );
	if (err != noErr)
		return err;

	itemName[0] = 0;
	cPB.hFileInfo.ioNamePtr = (StringPtr)&itemName;
	cPB.hFileInfo.ioVRefNum = vRefNum;

	do
	{
		// Get next source item at the current directory level

		++index;
		cPB.dirInfo.ioFDirIndex = index;
		cPB.dirInfo.ioDrDirID = dirID;
		err = PBGetCatInfoSync(&cPB);		

		if ( err == noErr )
		{
			// Is it a file (i.e. not a directory)?
			if ( (cPB.hFileInfo.ioFlAttrib & kioFlAttribDirMask) == 0 )
			{
				CopyPascalStringToC(itemName, filename);
				if (MatchString(filename, mask))
				{
					cout << "File name: " << filename << endl;
					files.push_back(filename);
				}
			}
		}

	} while (err == noErr );

	if ( (err == fnfErr) ||	// fnfErr is OK - it only means we hit the end of this level
		 (err == afpAccessDenied) ) // afpAccessDenied is OK, too - it only means we cannot see inside a directory
	{
		err = noErr;
	}
	
	return err;
}

#else	// This system has glob.h

#include <glob.h>

int U7ListFiles(const std::string mask, FileList& files)

{
        glob_t globres;
		string path(get_system_path(mask));
        int err = glob(path.c_str(), GLOB_NOSORT, 0, &globres);


        switch (err) {
        case 0:   //OK
				for (int i=0; i<globres.gl_pathc; i++)
				{
					files.push_back(globres.gl_pathv[i]);
					free(globres.gl_pathv[i]);
				}
		        free(globres.gl_pathv);
                return 0;               
        case 3:   //no matches
                return 0;
        default:  //error
                cerr << "Glob error " << err << endl;
                return err;
        }
}

#endif

void U7FreeFileList(char **files, int count)
{
        if (files == 0) return;


        for (int i=0; i<count; i++) {
                free(files[i]);
        }
        free(files);
}
