/*
 *	playfli.cc - Play Autodesk Animator FLIs
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


#ifndef PLAYFLI_H
#define PLAYFLI_H

#include "databuf.h"
#include "imagewin.h"
class Palette;

class playfli {
 public:
 struct fliinfo {
    int frames;
    int width;
    int height;
    int depth;
    int speed;
 };
 private:
    DataSource *fli_data;
    int fli_size;
    int fli_magic;
    int fli_frames;
    int fli_width;
    int fli_height;
    int fli_depth;
    int fli_flags;
    int fli_speed;
    Image_buffer *fli_buf;
    int streamstart;
    int streampos;
    int frame;
    Palette	*palette;
 public:
    playfli(const char *fli_name);
    playfli(char *buffer, std::size_t len);
    ~playfli();
    void info(fliinfo *fi = NULL);
    int play(Image_window *win, int first_frame = 0, int last_frame = -1, unsigned long ticks = 0, int brightness = 100);
    void put_buffer(Image_window *win);
    inline Palette *get_palette () { return palette; }
 private:
    void initfli();
    int	nextpal;
    int thispal;
    int changepal;
};


#endif
