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

#include <string.h>
#include <iostream.h>
#include "playfli.h"
#include "utils.h"

playfli::playfli(const char *fli_name)
{
    U7open(fli_stream, fli_name);
    fli_stream.seekg(8);
    fli_size = Read4(fli_stream);
    fli_magic = Read2(fli_stream);
    fli_frames = Read2(fli_stream);
    fli_width = Read2(fli_stream);
    fli_height = Read2(fli_stream);
    fli_depth = Read2(fli_stream);
    fli_flags = Read2(fli_stream);
    fli_speed = Read2(fli_stream);
    fli_stream.seekg(110, ios::cur);
}

void playfli::info()
{
    cout << "Frame count : " << fli_frames << endl;
    cout << "Width :       " << fli_width << endl;
    cout << "Height :      " << fli_height << endl;
    cout << "Depth :       " << fli_depth << endl;
    cout << "Speed :       " << fli_speed << endl;
}

void playfli::play(Image_window *win)
{
    int frame_size;
    int frame_magic;
    int frame_chunks;
    int chunk_size;
    int chunk_type;
    unsigned char *pixbuf;
    int streampos = fli_stream.tellg();
    int xoffset=(win->get_width()-fli_width)/2;
    int yoffset=(win->get_height()-fli_height)/2;
	
    pixbuf = new unsigned char[fli_width];
    // Play frames...
    for (int frame = 0; frame < fli_frames; frame++)
      {
	  fli_stream.seekg(streampos);
	  frame_size = Read4(fli_stream);
	  frame_magic = Read2(fli_stream);
	  frame_chunks = Read2(fli_stream);
	  fli_stream.seekg(8, ios::cur);
	  for (int chunk = 0; chunk < frame_chunks; chunk++)
	    {
		chunk_size = Read4(fli_stream);
		chunk_type = Read2(fli_stream);
		switch (chunk_type)
		  {
		  case 11:
		      {
			  int packets = Read2(fli_stream);
			  unsigned char colors[3 * 256];

			  memset(colors, 0, 3 * 256);
			  int current = 0;

			  for (int p_count = 0; p_count < packets;
			       p_count++)
			    {
				int skip = Read1(fli_stream);

				current += skip;
				int change = Read1(fli_stream);

				if (change == 0)
				    change = 256;
				fli_stream.read((char*)&colors[current*3], change*3);
			    }
			  // Set palette
			  if(win)
				win->set_palette(colors, 63, 100);
		      }
		      break;
		  case 12:
		      {
			  int skip_lines = Read2(fli_stream);
			  int change_lines = Read2(fli_stream);
			  for (int line = 0; line < change_lines; line++)
			    {
				int packets = Read1(fli_stream);
				int pixpos = xoffset;
				for (int p_count = 0; p_count < packets;
				     p_count++)
				  {
				      int skip_count = Read1(fli_stream);
				      pixpos += skip_count;
				      char size_count = Read1(fli_stream);
				      if (size_count < 0)
					{
					    unsigned char data = Read1(fli_stream);
					    memset(pixbuf, data, -size_count);
					    win->copy8(pixbuf,-size_count,1,pixpos,skip_lines+line+yoffset);
					    pixpos -= size_count;
					    
				        } else {
					    fli_stream.read((char*)pixbuf, size_count);
					    win->copy8(pixbuf,size_count,1,pixpos, skip_lines+line+yoffset);
					    pixpos += size_count;
					}
				  }
			    }
		      }
		      break;
		  case 13:
		      break;
		  case 15:
		      for (int line = 0; line < fli_height; line++)
			{
			    int packets = Read1(fli_stream);
			    int pixpos = 0;
			    for (int p_count = 0; p_count < packets;
				 p_count++)
			      {
				  char size_count = Read1(fli_stream);
				  if (size_count > 0)
				    {
					unsigned char data = Read1(fli_stream);
					memset(&pixbuf[pixpos], data, size_count);
					pixpos += size_count;
				    } else {
					fli_stream.read((char*)&pixbuf[pixpos],
							-size_count);
					pixpos -= size_count;
				    }
			      }
				if(win)
                                   win->copy8(pixbuf,fli_width,1,xoffset,line+yoffset);
			}
		      break;
		  case 16:
		      fli_stream.seekg(fli_width * fli_height, ios::cur);
		      break;
		  default:
		      cout << "UNKNOWN FLIC FRAME" << endl;
		      break;
		  }
	    }
	  streampos += frame_size;
	  if(win)
		win->show();
	  SDL_Delay(fli_speed*10);
      }
	delete[] pixbuf;
	
}

playfli::~playfli()
{
    // Clean up...
}
