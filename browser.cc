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

#include "files/U7file.h" 
#include "gamewin.h"
#include "game.h"
#include "browser.h"
	
ShapeBrowser::ShapeBrowser()
	{
		num_shapes = 0;
		current_shape = 0;
		num_frames = 0;
		current_frame = 0;
		num_files = game->get_resource("files/shapes/count").num;
		current_file = 0;
		shapes = 0;
		num_palettes = game->get_resource("palettes/count").num;
		current_palette = 0;
		num_xforms = game->get_resource("xforms/count").num;
		current_xform = -1;
	}

ShapeBrowser::~ShapeBrowser()
	{
		if(shapes)
			delete shapes;
	}

static void handle_key(int shift, int& value, int max, int amt = 1)
{
	if(shift)
		value -= amt;
	else
		value += amt;
	if(value<0)
		value = max-1;
	else if(value>=max)
		value = 0;
}
	
void ShapeBrowser::browse_shapes()
	{
		
		Game_window *gwin = Game_window::get_game_window();
		//Image_window8 *win = gwin->get_win();
		//int topx = (gwin->get_width()-320)/2;
		//int topy = (gwin->get_height()-200)/2;
		int centerx = gwin->get_width()/2;
		int centery = gwin->get_height()/2;
		Palette pal;
		char buf[255];
		str_int_pair pal_tuple, xform_tuple;
		char *fname;
		
		sprintf(buf,"files/shapes/%d",current_file);
		fname = game->get_resource(buf).str;
		if(!shapes)
			shapes = new Vga_file(fname);
		bool looping = true;
		bool redraw = true;
		SDL_Event event;
		//int active;
		
		do {
 		        if (redraw) {
			        gwin->clear_screen();
			        sprintf(buf,"palettes/%d",current_palette);
				pal_tuple = game->get_resource(buf);
				char xfrsc[256];
				if (current_xform > 0)
					{
					sprintf(xfrsc, "xforms/%d", 
							current_xform);
					xform_tuple = game->
						get_resource(xfrsc);
					pal.load(pal_tuple.str, pal_tuple.num,
						xform_tuple.str, 
						xform_tuple.num);
					}
				else
					pal.load(pal_tuple.str,pal_tuple.num);
				sprintf(buf,"VGA File: '%s'", fname);
				gwin->paint_text(MAINSHP_FONT1, buf, 0, 170);
				num_shapes = shapes->get_num_shapes();
				sprintf(buf,"Shape: %3d/%3d", current_shape, num_shapes-1);
				gwin->paint_text(MAINSHP_FONT1, buf, 0, 180);
			
			        num_frames = shapes->get_num_frames(current_shape);
				sprintf(buf,"Frame: %3d/%3d", current_frame, num_frames-1);
				gwin->paint_text(MAINSHP_FONT1, buf, 160, 180);
				sprintf(buf,"Palette: %s, %d", pal_tuple.str, pal_tuple.num);
				gwin->paint_text(MAINSHP_FONT1, buf, 0, 190);

			        if (num_frames) {
				        Shape_frame *frame = shapes->get_shape(
					        current_shape, current_frame);
 				        if (frame) {
					        sprintf(buf,"%d x %d", frame->get_width(), frame->get_height());
					        gwin->paint_text(MAINSHP_FONT1, buf, 32, 32);
  					        
						//draw outline
						gwin->get_win()->fill8(255, 
						    frame->get_width()+4, frame->get_height()+4, 
						    gwin->get_width()/2 - frame->get_xleft() - 2, 
						    gwin->get_height()/2 - frame->get_yabove() - 2);
					        gwin->get_win()->fill8(0,
						    frame->get_width()+2, frame->get_height()+2,
						    gwin->get_width()/2 - frame->get_xleft()-1, 
						    gwin->get_height()/2 - frame->get_yabove()-1);

						//draw shape
					        gwin->paint_shape(gwin->get_width()/2, gwin->get_height()/2, frame, 1);

				        } else
					        gwin->paint_text(MAINSHP_FONT1, "No Shape", centerx-20, centery-5);
 			        } else
				        gwin->paint_text(MAINSHP_FONT1, "No Shape", centerx-20, centery-5);
 	
			        pal.apply();
				redraw = false;
			}
  			SDL_WaitEvent(&event);
			if(event.type==SDL_KEYDOWN) {
               			redraw = true;
				int shift = event.key.keysym.mod & KMOD_SHIFT;
				//int ctrl = event.key.keysym.mod & KMOD_CTRL;
				switch(event.key.keysym.sym) {
				case SDLK_ESCAPE:
					looping = false;
					break;
				case SDLK_v:
					handle_key(shift, current_file, num_files);
					current_shape = 0;
					current_frame = 0;
					delete shapes;
					sprintf(buf,"files/shapes/%d",current_file);
					fname = game->get_resource(buf).str;
					shapes = new Vga_file(fname);
					break;
				case SDLK_p:
					handle_key(shift, current_palette, num_palettes);
					current_xform = -1;
					break;
				case SDLK_x:
					handle_key(shift, current_xform,
							num_xforms);
					break;
				case SDLK_s:
					handle_key(shift, current_shape, num_shapes);
					current_frame = 0;
					break;
				case SDLK_j:	// Jump by 20.
					handle_key(shift, current_shape, 
							num_shapes, 20);
					current_frame = 0;
					break;
				case SDLK_f:
					handle_key(shift, current_frame, num_frames);
					break;
				default:
					break;
				}
			}
		} while(looping);
	}
	
bool ShapeBrowser::get_shape(int& shape, int& frame)
	{
		if(!shapes || current_file!=0)
			return false;
		else {
			shape = current_shape;
			frame = current_frame;
			return true;
		}
	}
