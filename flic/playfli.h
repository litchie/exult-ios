/** 
 **	playfli.cc - Play Autodesk Animator FLIs
 **
 **	Written: 5/5/2000 - TST
 **/

/*
Copyright (C) 2000  Tristan Tarrant

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

#include <fstream.h>
#include "imagewin.h"

class playfli {
 private:
    ifstream fli_stream;
    int fli_size;
    int fli_magic;
    int fli_frames;
    int fli_width;
    int fli_height;
    int fli_depth;
    int fli_flags;
    int fli_speed;
 public:
    playfli(const char *fli_name);
    ~playfli();
    void info();
    void play(Image_window *win);
};
