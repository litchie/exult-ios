/*
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

#include <string.h>
#include <stdlib.h>
#include "files/common_types.h"
#include "files/U7file.h"
#include "gamewin.h"
#include "txtscroll.h"
#include "font.h"
#include "game.h"

using std::string;
using std::vector;

TextScroller::TextScroller(const char *archive, int index, Font *fnt, Shape *shp)
{
	font = fnt;
	shapes = shp;
	U7object txtobj(archive, index);
	size_t len;
#ifdef MACOS
	const char	CR = '\n', LF = '\r';
#else
	const char	CR = '\r', LF = '\n';
#endif
		
	char *txt, *ptr, *end;
	txtobj.retrieve(&txt, len);
	ptr = txt;
	end = ptr+len;

	text = new vector<string>();
	while(ptr<end) {
		char *start = ptr;
		ptr = strchr(ptr, LF);
		if(ptr) {
			if(*(ptr-1)==CR) // It's CR/LF
				*(ptr-1) = 0;
			else
				*ptr = 0;
			text->push_back(string(start));
			ptr += 1;
		} else
			break;
	}
	delete [] txt;
}

TextScroller::~TextScroller()
{
	delete text;
}

int TextScroller::show_line(Game_window *gwin, int left, int right, int y, int index)
{
	//The texts used in the main menu contains backslashed sequences that
	//indicates the output format of the lines:
	// \Px   include picture number x (frame nr. of shape passed to constructor)
	// \C    center line
	// \L    left aligned to right center line
	// \R	 right aligned to left center line
	// |	 carriage return (stay on same line)
	// #xxx	 display character with number xxx
	string str = (*text)[index];
	const char * ptr = str.c_str();
	char *txt = new char[strlen(ptr)+1];
	
	char *txtptr = txt;
	int ypos = y;
	int vspace = 2; // 2 extra pixels between lines
	// Align text to the left by default
	int align = -1;
	int xpos = left;
	int center = (right+left)/2;
	bool add_line = true;
	
	while(*ptr) {
		if(!strncmp(ptr,"\\P",2)) {
			int pix = *(ptr+2)-'0';
			ptr +=3;
			Shape_frame *frame = shapes->get_frame(pix);
			if (frame) {
			        gwin->paint_shape(center-frame->get_width()/2,
						  ypos, frame);
				ypos += frame->get_height()+vspace;
			}
		} else if(!strncmp(ptr,"\\C",2)) {
			ptr += 2;
			align = 0;
		} else if(!strncmp(ptr,"\\L",2)) {
			ptr += 2;
			align = 1;
		} else if(!strncmp(ptr,"\\R",2)) {
			ptr += 2;
			align = -1;
		} else if(*ptr=='|' || *(ptr+1)==0) {
			if(*(ptr+1)==0 && *ptr!='|')
			{
				*txtptr++ = *ptr;
				add_line = false;
			}
			*txtptr = 0;
			
			if(align<0)
				xpos = center-font->get_text_width(txt);
			else if(align==0)
				xpos = center-font->get_text_width(txt)/2;
			else
				xpos = center;
			font->draw_text(gwin,xpos,ypos,txt);
			if(*ptr!='|') ypos += font->get_text_height()+vspace;
			txtptr = txt;	// Go to beginning of string
			++ptr;
		} else if(*ptr=='#') {
			ptr++;
			char numerical[4] = {0,0,0,0};
			char *num = numerical;
			while (*ptr >= '0' && *ptr <= '9')
				*num++ = *ptr++;
			*txtptr++ = atoi(numerical);
		} else
			*txtptr++ = *ptr++;
	}
	
	delete [] txt;
	if(add_line)
		ypos += font->get_text_height();
	return ypos;
}


void TextScroller::run(Game_window *gwin, Palette& pal)
{
	int topx = (gwin->get_width()-320)/2;
	int topy = (gwin->get_height()-200)/2;
	int endy = topy+200;
	uint32 starty = endy;
	uint32 startline = 0;
	unsigned int maxlines = text->size();
	bool looping = true;
	uint32 next_time = SDL_GetTicks() + 200;
	while(looping) {
		int ypos = starty;
		uint32 curline = startline;
		gwin->clear_screen();
		do {
			if(curline==maxlines)
				break;
			ypos = show_line(gwin, topx, topx+320, ypos, curline++);
			if(ypos<topy) {		// If this line doesn't appear, don't show it next time
				++startline;
				starty = ypos;
				if(startline>=maxlines) {
					looping = false;
					break;
				}
			}
		} while (ypos<endy);
		pal.apply();
		while (next_time > SDL_GetTicks())
			;
		next_time += 120;
		looping = looping && !wait_delay(0);
		if(!looping)
			pal.fade_out(30);
		starty --;
	}
}

int TextScroller::get_count()
{
	return text->size();
}
