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
using std::string;

#include "utils.h"
#include "listfiles.h"

// System Specific Code for Windows
#if defined(WIN32)

// Need this for _findfirst, _findnext, _findclose
#include <io.h>

int U7ListFiles(const char *p, char **& files, int& count)
{
	string			path(get_system_path(string(p)));
	vector<char *>		filelist;
	struct _finddata_t	fileinfo;
	long			handle;
	char			*stripped_path;
	int			i;

	files = 0;
	count = 0;

	handle = _findfirst (path.c_str(), &fileinfo);

	// 
	stripped_path = new char [strlen (path.c_str())+1];
	strcpy (stripped_path, path.c_str());

	for (i = strlen (stripped_path)-1; i; i--)
		if (stripped_path[i] == '\\' || stripped_path[i] == '/')
			break;

	if (stripped_path[i] == '\\' || stripped_path[i] == '/') stripped_path[i+1] = 0;


	// Now search the files
	if (handle != -1) do {
		char *filename = (char *) malloc (strlen (fileinfo.name)+strlen(stripped_path)+1 );
		strcpy (filename, stripped_path);
		strcat (filename, fileinfo.name);
		filelist.push_back(filename);
		count++;

	} while (_findnext( handle, &fileinfo ) != -1);
	else {
		delete [] stripped_path;
		_findclose (handle);
		return 0;
	}

	delete [] stripped_path;
	_findclose (handle);

	// Now Put the files into the array
	files = (char **) malloc (sizeof(char *)*count);

	for (i = 0; i < count; i++) files[i] = filelist[i];

	return 0;
}

#elif defined(MACOS)

int U7ListFiles(const char *p, char **& files, int& count)
{
	// TODO Implement this!
}

#else	// This system has glob.h

#include <glob.h>

int U7ListFiles(const char *p, char **& files, int& count)
{
        glob_t globres;
		string path(get_system_path(string(p)));
        int err = glob(path.c_str(), GLOB_NOSORT, 0, &globres);


        switch (err) {
        case 0:   //OK
                files = globres.gl_pathv;
                count = globres.gl_pathc;
                return 0;               
        case 3:   //no matches
                files = 0; count = 0;
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
