/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Gamewin.cc - X-windows Ultima7 map browser.
 **
 **	Written: 7/22/98 - JSF
 **/

/*
Copyright (C) 1998  Jeffrey S. Freedman

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

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "gamewin.h"
#include "items.h"
#include "utils.h"
#include "compile.h"
#include "fnames.h"
#include "usecode.h"
#include "npcnear.h"

					// THE game window:
Game_window *Game_window::game_window = 0;

/*
 *	Create game window.
 */

Game_window::Game_window
	(
	int width, int height		// Window dimensions.
	) : chunkx(0), chunky(0), painted(0), focus(1),
	    brightness(100), 
	    skip_lift(16), debug(0),
	    tqueue(new Time_queue()), clock(tqueue),
		npc_prox(new Npc_proximity_handler(this)),
	    main_actor(0),
	    conv_choices(0), texts(0), num_faces(0), last_face_shown(-1),
	    open_gumps(0),
	    main_actor_inside(0), mode(intro), npcs(0),
	    shapes(), dragging(0),
	    faces(FACES_VGA), gumps(GUMPS_VGA)
	{
	game_window = this;		// Set static ->.
	if (!shapes.is_good())
		abort("Can't open 'shapes.vga' file.");
	if (!faces.is_good())
		abort("Can't open 'faces.vga' file.");
	if (!gumps.is_good())
		abort("Can't open 'gumps.vga' file.");
	u7open(chunks, U7CHUNKS);
	u7open(u7map, U7MAP);
	ifstream ucfile;		// Read in usecode.
	u7open(ucfile, USECODE);
	usecode = new Usecode_machine(ucfile, this);
	ucfile.close();
	ifstream textflx;	
  	u7open(textflx, TEST_FLX);
	Setup_item_names(textflx);	// Set up list of item names.
					// Read in shape dimensions.
	if (!shapes.read_info())
		abort(
		"Can't read shape data (tfa.dat, wgtvol.dat, shpdims.dat).");
					// Create window.
	win = new Image_window(width, height); //<- error in timer

					// Get 12-point font.
	font12 = win->open_font(AVATAR_TTF, 12);
					// Set title.
	win->set_title("Exult Ultima7 Engine");

	unsigned long timer = SDL_GetTicks();
	srand(timer);			// Use time to seed rand. generator.
					// Force clock to start.
	tqueue->add(timer, &clock, (long) this);
					// Clear object lists, flags.
	memset((char *) objects, 0, sizeof(objects));
	memset((char *) schunk_read, 0, sizeof(schunk_read));
	get_palette(0);			// Try first palette.
	brighten(20);			// Brighten 20%.
	}

/*
 *	Deleting game window.
 */

Game_window::~Game_window
	(
	)
	{
	win->close_font(font12);

#if !(defined(XWIN) || defined(DOS)) //avoid crash
  DeleteObject(win->BMHan);
  if (shapewin)
    DeleteObject(shapewin->BMHan);
  return;
#endif
		   //Crashes in Win95 somehow :-(
		   //Only happens when CreateDIBSection() has been called.
		   //crash after last instruction of ~image_window
	delete win;
	delete [] conv_choices;
	}

/*
 *	Abort.
 */

void Game_window::abort
	(
	char *msg,
	...
	)
	{
	va_list ap;
	va_start(ap, msg);
	char buf[512];
	vsprintf(buf, msg, ap);		// Format the message.
	cerr << "Exult (fatal): " << buf << '\n';
	delete this;
	exit(-1);
	}

/*
 *	Resize event occurred.
 */

void Game_window::resized
	(
	unsigned int neww, 
	unsigned int newh
	)
	{			
	win->resized(neww, newh);
	paint();
	}

/*
 *	Open a file, trying the original name (lower case), and the upper
 *	case version of the name.  If it can't be opened either way, we
 *	abort with an error message.
 *
 *	Output: 0 if couldn't open and dont_abort == 1.
 */

int Game_window::u7open
	(
	ifstream& in,			// Input stream to open.
	char *fname,			// May be converted to upper-case.
	int dont_abort			// 1 to just return 0.
	)
	{
	if (!U7open(in, fname))
		if (dont_abort)
			return (0);
		else
			abort("Can't open '%s'.", fname);
	return (1);
	}

/*
 *	Show the absolute game location, where each coordinate is of the
 *	8x8 shape box clicked on.
 */

void Game_window::show_game_location
	(
	int x, int y			// Point on screen.
	)
	{
	x = chunkx*16 + x/8;
	y = chunky*16 + y/8;
	cout << "Game location is (" << x << ", " << y << ")\n";
	}


/*
 *	Get a shape onto the screen.
 */

void Game_window::paint_shape
	(
	Image_window *iwin,		// Window to display it in.
	int xoff, int yoff,		// Where to draw in window.
	Shape_frame *shape		// What to paint.
	)
	{
	if (!shape->rle)		// Not RLE?
		iwin->copy8(shape->data, 8, 8, xoff, yoff);
	else
					// Get compressed data.
		get_rle_shape(iwin, *shape, xoff + 8, yoff + 8);
	}

/*
 *	Show a Run-Length_Encoded shape.
 */

void Game_window::get_rle_shape
	(
	Image_window *iwin,		// Window to draw it in.
	Shape_frame& shape,		// Shape to display.
	int xoff, int yoff		// Where to show in iwin.
	)
	{
	unsigned char *in = shape.data; // Point to data.
	int scanlen;
	while ((scanlen = Read2(in)) != 0)
		{
					// Get length of scan line.
		int encoded = scanlen&1;// Is it encoded?
		scanlen = scanlen>>1;
		short scanx = Read2(in);
		short scany = Read2(in);
		if (!encoded)		// Raw data?
			{
			iwin->copy8(in, scanlen, 1,
					xoff + scanx, yoff + scany);
			in += scanlen;
			continue;
			}
		for (int b = 0; b < scanlen; )
			{
			unsigned char bcnt = *in++;
					// Repeat next char. if odd.
			int repeat = bcnt&1;
			bcnt = bcnt>>1; // Get count.
			if (repeat)
				{
				unsigned char pix = *in++;
				iwin->fill8(pix, bcnt, 1,
					xoff + scanx + b, yoff + scany);
				}
			else		// Get that # of bytes.
				{
				iwin->copy8(in, bcnt, 1,
					xoff + scanx + b, yoff + scany);
				in += bcnt;
				}
			b += bcnt;
			}
		}
	}

/*
 *	Get the map objects and scenery for a superchunk.
 */

void Game_window::get_map_objects
	(
	int schunk			// Superchunk # (0-143).
	)
	{
	u7map.seekg(schunk * 16*16*2);	// Get to desired chunk.
	unsigned char buf[16*16*2];
	u7map.read(buf, sizeof(buf));	// Read in the chunk #'s.
	int scy = 16*(schunk/12);	// Get abs. chunk coords.
	int scx = 16*(schunk%12);
	unsigned char *mapdata = buf;
					// Go through chunks.
	for (int cy = 0; cy < 16; cy++)
		for (int cx = 0; cx < 16; cx++)
			{
					// Get chunk #.
			int chunk_num = *mapdata++;
			chunk_num += (*mapdata++) << 8;
			get_chunk_objects(scx + cx, scy + cy, chunk_num);
			}
	}

static int rlecount = 0;	//+++++++++++DEBUGGING.

/*
 *	Read in graphics data into window's image.
 */

void Game_window::get_chunk_objects
	(
	int cx, int cy,			// Chunk index within map.
	int chunk_num			// Desired chunk # within file.
	)
	{
	chunks.seekg(chunk_num * 512);	// Get to desired chunk.
	unsigned char buf[16*16*2];	// Read in 16x16 2-byte shape #'s.
	chunks.read(buf, sizeof(buf));
	unsigned char *data = &buf[0];
	Game_object *rles = 0;		// We'll store RLE shapes list here.
	Game_object *last_rle;		// ->last one we added.
	int rlenum = 0;			// An index into rles.
					// Get list we'll store into.
	Chunk_object_list *olist = get_objects(cx, cy);
					// A chunk is 16x16 shapes.
	for (int shapey = 0; shapey < 16; shapey++)
		for (int shapex = 0; shapex < 16; shapex++)
			{
			ShapeID id(data[0], data[1]);
			Shape_frame *shape = get_shape(id);
			if (shape->rle)
				{
				Game_object *obj = new Game_object(
					data[0], data[1], shapex, shapey);
				if (rles)
					last_rle->set_next(obj);
				else
					rles = obj;
				last_rle = obj;
				rlecount++;
				}
			else		// Flat.
				olist->set_flat(shapex, shapey, id);
			data += 2;
			}
	if (rles)
		{
		last_rle->set_next(0);	// Finish list.
		while (rles)
			{
			Game_object *nxt = rles->get_next();
			olist->add(rles);
			rles = nxt;
			}
		}
	}


/*
 *	Read in the objects for a superchunk from one of the "u7ifix" files.
 */

void Game_window::get_ifix_objects
	(
	int schunk			// Superchunk # (0-143).
	)
	{
	char fname[128];		// Set up name.
	strcpy(fname, U7IFIX);
	int len = strlen(fname);
	fname[len] = '0' + schunk/16;
	int lb = schunk%16;
	fname[len + 1] = lb < 10 ? ('0' + lb) : ('a' + (lb - 10));
	fname[len + 2] = 0;
	ifstream ifix;			// There it is.
	u7open(ifix, fname);
	int scy = 16*(schunk/12);	// Get abs. chunk coords.
	int scx = 16*(schunk%12);
					// Go through chunks.
	for (int cy = 0; cy < 16; cy++)
		for (int cx = 0; cx < 16; cx++)
			{
					// Get to index entry for chunk.
			int chunk_num = cy*16 + cx;
			ifix.seekg(0x80 + chunk_num*8);
					// Get location, length.
			long shapesoff = Read4(ifix);
			if (!shapesoff) // Nothing there?
				continue;
			unsigned long shapeslen = Read4(ifix);
			get_ifix_chunk_objects(ifix, shapesoff, shapeslen/4,
				scx + cx, scy + cy);
			}
	}

/*
 *	Get the objects from one ifix chunk entry onto the screen.
 */

void Game_window::get_ifix_chunk_objects
	(
	ifstream& ifix,
	long filepos,			// Where chunk's data lies.
	int cnt,			// # entries (objects).
	int cx, int cy			// Absolute chunk #'s.
	)
	{
	ifix.seekg(filepos);		// Get to actual shape.
					// Get buffer to hold entries' indices.
	unsigned char *entries = new unsigned char[4*cnt];
	unsigned char *ent = entries;
	ifix.read(entries, 4*cnt);	// Read them in.
					// Get object list for chunk.
	Chunk_object_list *olist = get_objects(cx, cy);
	for (int i = 0; i < cnt; i++, ent += 4)
		{
		Game_object *obj = new Game_object(ent);
		olist->add(obj);
		rlecount++;		//++++++++++++++++++++DEBUGGING
		}
	delete[] entries;		// Done with buffer.
	}

/*
 *	Read in the objects for a superchunk from one of the "u7ireg" files.
 *	(These are the moveable objects.)
 */

void Game_window::get_ireg_objects
	(
	int schunk			// Superchunk # (0-143).
	)
	{
	char fname[128];		// Set up name.
	strcpy(fname, U7IREG);
	int len = strlen(fname);
	fname[len] = '0' + schunk/16;
	int lb = schunk%16;
	fname[len + 1] = lb < 10 ? ('0' + lb) : ('a' + (lb - 10));
	fname[len + 2] = 0;
	ifstream ireg;			// There it is.
	if (!u7open(ireg, fname, 1))
		return;			// Just don't show them.
	int scy = 16*(schunk/12);	// Get abs. chunk coords.
	int scx = 16*(schunk%12);
	read_ireg_objects(ireg, scx, scy);
	}

/*
 *	Read a list of ireg objects.  They are either placed in the desired
 *	game chunk, or added to their container.
 */

void Game_window::read_ireg_objects
	(
	ifstream& ireg,			// File to read from.
	int scx, int scy,		// Abs. chunk coords. of superchunk.
	Container_game_object *container// Container, or null.
	)
	{
	int entlen;			// Gets entry length.
					// Go through entries.
	while (((entlen = Read1(ireg), ireg.good())))
		{
		if (!entlen || entlen == 1)
			if (container)
				return;	// Skip 0's & ends of containers.
			else
				continue;
		if (entlen != 6 && entlen != 12 && entlen != 18)
			{		// 
			long pos = ireg.tellg();
			ireg.seekg(pos + entlen);
			continue;	// Only know these two types.
			}
		unsigned char entry[18];// Get entry.
		ireg.read(entry, entlen);
		int cx = entry[0] >> 4; // Get chunk indices within schunk.
		int cy = entry[1] >> 4;
					// Get coord. #'s where shape goes.
		int shapex = entry[0] & 0xf;
		int shapey = entry[1] & 0xf;
					// Get shape #.
		int shapeid = entry[2]+256*(entry[3]&3);
		unsigned int lift, quality, type;
		Game_object *obj;
		if (shapeid == 275)	// An "egg"?
			{
			Egg_object *egg = create_egg(entry);
			get_objects(scx + cx, scy + cy)->add_egg(egg);
			continue;
			}
		else if (entlen == 6)	// Simple entry?
			{
			type = 0;
			lift = entry[4] >> 4;
			quality = entry[5];
			obj = new Game_object(
				entry[2], entry[3], shapex, shapey, lift);
			}
		else if (entlen == 12)	// Container?
			{
			type = entry[4] + 256*entry[5];
			lift = entry[9] >> 4;
			quality = 0;
#if 0
cout << item_names[entry[2]+256*(entry[3]&3)] << ": ";
for (int e = 0; e < 12; e++)
	cout << (void *) entry[e] << ' ';
cout << '\n';
#endif
			Container_game_object *cobj = 
				new Container_game_object(
				entry[2], entry[3], shapex, shapey, lift);
					// Read container's objects.
			if (type)	// ???Don't understand this yet.
				read_ireg_objects(ireg, scx, scy, cobj);
			obj = cobj;
			}
		else
			continue;	// FOR NOW.
		obj->set_quality(quality);
		if (!container)
			get_objects(scx + cx, scy + cy)->add(obj);
		else
			container->add(obj);
#if 0	/* Not sure about this yet. */
		if (entlen == 12 &&	// Container?
					// Not an egg or ??.
		    !(entry[2]==0x13 && ((entry[3]&3)==1)) &&
		    !(entry[2]==0xc1 && entry[3]==3))
					// Skip contents
			while ((entlen = Read1(ireg)) > 1 && ireg.good())
				ireg.read(entry, entlen);
#endif
		rlecount++;		//++++++++++++++++++++DEBUGGING
		}
	}

/*
 *	Create an "egg".
 */

Egg_object *Game_window::create_egg
	(
	unsigned char *entry		// 1-byte ireg entry.
	)
	{
	unsigned short type = entry[4] + 256*entry[5];
	int prob = entry[6];		// Probability (1-100).
	int data1 = entry[7] + 256*entry[8];
	int lift = entry[9] >> 4;
	int data2 = entry[10] + 256*entry[11];
					// Get egg info. (debugging).
	unsigned char egg_type = type&0xf;
	unsigned char criteria = (type & (7<<4)) >> 4;
	unsigned char nocturnal = (type >> 7) & 1;
	unsigned char once = (type >> 8) & 1;
	unsigned char hatched = (type >> 9) & 1;
	unsigned short distance = (type & (0x1f << 10)) >> 10;
	unsigned char auto_reset = (type >> 15) & 1;

	Egg_object *obj = new Egg_object(entry[2], entry[3], 
		entry[0]&0xf, entry[1]&0xf, lift, type, prob,
		data1, data2);
	return (obj);
	}

/*
 *	Read in the objects in a superchunk.
 */

void Game_window::get_superchunk_objects
	(
	int schunk			// Superchunk #.
	)
	{
	get_map_objects(schunk);	// Get map objects/scenery.
	get_ifix_objects(schunk);	// Get objects from ifix.
	get_ireg_objects(schunk);	// Get moveable objects.
	schunk_read[schunk] = 1;	// Done this one now.
// cout << "Rlecount = " << rlecount << '\n';
	}

/*
 *	Put the actor(s) in the world.
 */

void Game_window::init_actors
	(
	)
	{
	if (main_actor)			// Already done?
		return;
	read_npcs();			// Read in all U7 NPC's.
	}


/*
 *	Paint a rectangle in the window by pulling in vga chunks.
 */

void Game_window::paint
	(
	int x, int y, int w, int h	// Rectangle to cover.
	)
	{
	if (!win->ready())
		return;
	win->set_clip(x, y, w, h);	// Clip to this area.
					// Which chunks to start with:
					// Watch for shapes 1 chunk to left.
	int start_chunkx = chunkx + x/chunksize - 1;
	if (start_chunkx < 0)
		start_chunkx = 0;
	int start_chunky = chunky + y/chunksize - 1;
	if (start_chunky < 0)
		start_chunky = 0;
	int startx = (start_chunkx - chunkx) * chunksize;
	int starty = (start_chunky - chunky) * chunksize;
	int stopx = x + w;
	int stopy = y + h;
					// Go 1 chunk past end.
	int stop_chunkx = chunkx + stopx/chunksize + 
						(stopx%chunksize != 0) + 1;
	if (stop_chunkx > num_chunks)
		stop_chunkx = num_chunks;
	int stop_chunky = chunky + stopy/chunksize + 
						(stopy%chunksize != 0) + 1;
	if (stop_chunky > num_chunks)
		stop_chunky = num_chunks;
	int xoff, yoff;			// Offsets on screen.
	int cx, cy;			// Chunk #'s.
					// Read in "map", "ifix" objects for
					//  all visible superchunks.
	for (cy = start_chunky; cy < stop_chunky; )
		{
		int schunky = cy/16;	// 16 chunks/superchunk.
					// How far to bottom of superchunk?
		int num_chunks_y = 16 - (cy%16);
		if (cy + num_chunks_y > stop_chunky)
			{
			num_chunks_y = stop_chunky - cy;
			if (num_chunks_y <= 0)
				break;	// Past end of world.
			}
		for (cx = start_chunkx; cx < stop_chunkx; )
			{
			int schunkx = cx/16;
			int num_chunks_x = 16 - (cx%16);
			if (cx + num_chunks_x > stop_chunkx)
				{
				num_chunks_x = stop_chunkx - cx;
				if (num_chunks_x <= 0)
					break;
				}
					// Figure superchunk #.
			int schunk = 12*schunky + schunkx;
					// Read it if necessary.
			if (!schunk_read[schunk])
				get_superchunk_objects(schunk);
					// Increment x coords.
			cx += num_chunks_x;
			}
					// Increment y coords.
		cy += num_chunks_y;
		}
					// Paint all the flat scenery.
	for (yoff = starty, cy = start_chunky; cy < stop_chunky; 
						yoff += chunksize, cy++)
		for (xoff = startx, cx = start_chunkx; 
				cx < stop_chunkx; xoff += chunksize, cx++)
			paint_chunk_flats(cx, cy, xoff, yoff);

					// Do each level going upwards.
	for (int lift = 0; lift < 16; lift++)
					// Draw the chunks' objects.
		for (yoff = starty, cy = start_chunky; cy < stop_chunky; 
						yoff += chunksize, cy++)
			for (xoff = startx, cx = start_chunkx; 
				cx < stop_chunkx; xoff += chunksize, cx++)
				paint_chunk_objects(lift, cx, cy, xoff, yoff);
	static Font_face *font = 0;
	if (!font)
#ifdef XWIN
		font = win->open_font(AVATAR_TTF, 24);
#else
		font = win->open_font(AVATAR_TTF, 14);
#endif
	if (mode == intro && font != 0)
		{
		int x = 15, y = 15;
		int w = get_width() - x, h = get_height() - y;
		char buf[512];
		sprintf(buf, "Welcome to EXULT V 0.%02d, a free RPG game engine.\n\nCopyright 2000 J. S. Freedman\nGraphics copyrighted by Origin\nText rendered by FreeType", RELNUM);
		win->draw_text_box(font, buf, 
				x, y, 600 < w ? 600 : w, 400 < h ? 400 : h);
		}
					// Draw text.
	for (Text_object *txt = texts; txt; txt = txt->next)
		paint_text(txt);
					// Draw gumps.
	for (Gump_object *gmp = open_gumps; gmp; gmp = gmp->get_next())
		paint_gump(gmp);
	win->clear_clip();
	painted = 1;
	}

/*
 *	Paint a text object.
 */

void Game_window::paint_text
	(
	Text_object *txt
	)
	{
	char *msg = txt->msg;
	if (*msg == '@')
		msg++;
	int len = strlen(msg);
	if (msg[len - 1] == '@')
		len--;
	win->draw_text(font12, msg, len,
			(txt->cx - chunkx)*chunksize + txt->sx*tilesize,
		        (txt->cy - chunky)*chunksize + txt->sy*tilesize);
	painted = 1;
	}

/*
 *	Paint a gump and its contents.
 */

void Game_window::paint_gump
	(
	Gump_object *gmp
	)
	{
	paint_gump(win, gmp->get_x(), gmp->get_y(),
				gmp->get_shapenum(), gmp->get_framenum());
	//+++++++++++++++elements
	}

/*
 *	Paint the flat (non-rle) shapes in a chunk.
 */

void Game_window::paint_chunk_flats
	(
	int cx, int cy,			// Chunk coords (0 - 12*16).
	int xoff, int yoff		// Where to place chunk.
	)
	{
	Chunk_object_list *olist = get_objects(cx, cy);
					// Go through array of shapes.
	for (int shapey = 0; shapey < 16; shapey++)
		for (int shapex = 0; shapex < 16; shapex++)
			{
			ShapeID id = olist->get_flat(shapex, shapey);
			if (!id.is_invalid())
					// Draw shape.
				paint_shape(win, xoff + shapex*8,
					yoff + shapey*8,
					id.get_shapenum(), id.get_framenum());
			}
	}

/*
 *	Paint a chunk's objects, left-to-right, top-to-bottom.
 */

void Game_window::paint_chunk_objects
	(
	int at_lift,			// Only paint this lift.
	int cx, int cy,			// Chunk coords (0 - 12*16).
	int xoff, int yoff		// Where to place chunk.
	)
	{
	Game_object *obj;
	Chunk_object_list *olist = get_objects(cx, cy);
	int above_actor;		// If inside, figure height above
	if (main_actor_inside)		//   actor's head.
		{
		above_actor = main_actor->get_lift() + 
		  shapes.get_info(main_actor->get_shapenum()).get_3d_height();
		}
	else
		above_actor = 32;
	for (obj = olist->get_first(); obj; obj = olist->get_next(obj))
		{
		int shapex = obj->get_shape_pos_x();
		int shapey = obj->get_shape_pos_y();
		int lift = obj->get_lift();
		if (lift > at_lift)	// They're sorted by lift first.
			break;
		if (lift != at_lift || lift >= skip_lift)
			continue;	// Don't show things off ground.
					// Is actor inside?  Skip roofs if so.
		if (lift >= above_actor)
			continue;
					// Get shape & frame.
		int shapenum = obj->get_shapenum();
		int framenum = obj->get_framenum();
					// Draw shape.
		paint_shape(win, xoff + shapex*8 - 4*lift, 
				yoff + shapey*8 - 4*lift,
					shapenum, framenum);
		}
	}

/*
 *	Read in a palette.
 */

void Game_window::get_palette
	(
	int pal_num,			// 0-11.
	int brightness			// Brightness % (100 = normal).
	)
	{
	ifstream pal;
	u7open(pal, PALETTES_FLX);
	pal.seekg(256 + 3*256*pal_num); // Get to desired palette.
	unsigned char colors[3*256];	// Read it in.	
	pal.read(colors, sizeof(colors));
					// They use 6 bits.
	win->set_palette(colors, 63, brightness);
	}

/*
 *	Brighten/darken palette.
 */

void Game_window::brighten
	(
	int per				// +- percentage to change.
	)
	{
	brightness += per;
	if (brightness < 20)		// Have a min.
		brightness = 20;
	get_palette(0, brightness);
	paint();
	}

/*
 *	Shift view by one chunk.
 */

void Game_window::view_right
	(
	)
	{
	if (chunkx + get_width()/chunksize >= num_chunks - 1)
		return;
	int w = get_width(), h = get_height();
					// Shift image to left.
	win->copy(chunksize, 0, w - chunksize, h, 0, 0);
	chunkx++;			// Increment offset.
					// Paint 1 column to right.
	paint(w - chunksize, 0, chunksize, h);
					// Find newly visible NPC's.
	int from_cx = chunkx + (w + chunksize - 1)/chunksize - 1;
	add_nearby_npcs(from_cx, chunky, from_cx + 1, 
			chunky + (h + chunksize - 1)/chunksize);
	}
void Game_window::view_left
	(
	)
	{
	if (chunkx <= 0)
		return;
	win->copy(0, 0, get_width() - chunksize, get_height(), chunksize, 0);
	chunkx--;
	int h = get_height();
	paint(0, 0, chunksize, h);
					// Find newly visible NPC's.
	add_nearby_npcs(chunkx, chunky, chunkx + 1, 
			chunky + (h + chunksize - 1)/chunksize);
	}
void Game_window::view_down
	(
	)
	{
	if (chunky + get_height()/chunksize >= num_chunks - 1)
		return;
	int w = get_width(), h = get_height();
	win->copy(0, chunksize, w, h - chunksize, 0, 0);
	chunky++;
	paint(0, h - chunksize, w, chunksize);
					// Find newly visible NPC's.
	int from_cy = chunky + (h + chunksize - 1)/chunksize - 1;
	add_nearby_npcs(chunkx, from_cy, 
			chunkx + (w + chunksize - 1)/chunksize,
			from_cy + 1);
	}
void Game_window::view_up
	(
	)
	{
	if (chunky < 0)
		return;
	int w = get_width();
	win->copy(0, 0, w, get_height() - chunksize, 0, chunksize);
	chunky--;
	paint(0, 0, w, chunksize);
					// Find newly visible NPC's.
	add_nearby_npcs(chunkx, chunky, 
			chunkx + (w + chunksize - 1)/chunksize, chunky + 1);
	}

/*
 *	Repaint a sprite after it has been moved.
 */

void Game_window::repaint_sprite
	(
	Sprite *sprite,
	Rectangle& oldrect		// Where it used to be.
	)
	{
					// Get new rectangle.
	Rectangle newrect = get_shape_rect(sprite);
					// Merge them.
	Rectangle sum = oldrect.add(newrect);
	sum.x -= 4;			// Make a little bigger.
	sum.y -= 4;
	sum.w += 8;
	sum.h += 8;
					// Intersect with screen.
	sum = clip_to_win(sum);
	if (sum.w > 0 && sum.h > 0)	// Watch for negatives.
		paint(sum.x, sum.y, sum.w, sum.h);
	}
#if 0	/* ++++++++Going away. */
/*
 *	Animation.
 */

void Game_window::animate
	(
	unsigned long time		// Current time of day.
	)
	{
	if (!focus || mode == conversation)
		return;			// We're dormant.
	int cx, cy, sx, sy;		// Get chunk, shape within chunk.
	int frame;
	int repaint_all = 0;		// Flag to repaint window.
	int blocked = 0;		// Flag for obstacle.
	if (main_actor->next_frame(time, cx, cy, sx, sy, frame) &&
					// Watch for occupied square.
	    !(blocked = objects[cx][cy]->is_occupied(shapes, main_actor,
					main_actor->get_lift(), sx, sy)))
		{
					// At left?
		if (cx - chunkx <= 0 && sx < 6)
			view_left();
					// At right?
		else if ((cx - chunkx)*16 + sx >= get_width()/8 - 4)
			view_right();
					// At top?
		if (cy - chunky <= 0 && sy < 6)
			view_up();
					// At bottom?
		else if ((cy - chunky)*16 + sy >= get_height()/8 - 4)
			view_down();
					// Get old chunk it's in.
		int old_cx = main_actor->get_cx(),
					old_cy = main_actor->get_cy();
					// Get old rectangle.
		Rectangle oldrect = get_shape_rect(main_actor);
		Chunk_object_list *olist = get_objects(cx, cy);
					// Move it.
		main_actor->move(cx, cy, olist, sx, sy, frame);
					// Near an egg?
		Egg_object *egg = olist->find_egg(sx, sy);
		if (egg)
			egg->activate(usecode);
		int inside;		// See if moved inside/outside.
					// In a new chunk?
		if ((main_actor->get_cx() != old_cx ||
		    main_actor->get_cy() != old_cy) &&
			main_actor_inside != find_roof(main_actor->get_cx(),
						main_actor->get_cy()))
			{
			main_actor_inside = !main_actor_inside;
			repaint_all = 1;// Changed, so paint everything.
			}
		else
			repaint_sprite(main_actor, oldrect);
		}
	else if (blocked)
		main_actor->stop();	// Not sure about this.
	Actor *actor = main_actor;	// Go through rest of actors.
	while ((actor = actor->get_next()) != main_actor)
		{
		blocked = 0;
		if (actor->next_frame(time, cx, cy, sx, sy, frame) &&
					// Watch for occupied square.
		    !(blocked = objects[cx][cy]->is_occupied(shapes, actor,
						actor->get_lift(), sx, sy)))
			{
					// Get old rectangle.
			Rectangle oldrect = get_shape_rect(actor);
					// Move it.
			actor->move(cx, cy, get_objects(cx, cy), 
							sx, sy, frame);
					// Skip if repainting whole screen.
			if (!repaint_all)
				repaint_sprite(actor, oldrect);
			}
		else if (blocked)
			actor->stop();	// Is this okay?
		}
	if (repaint_all)
		paint();		// Paint whole window now.
	}
#endif

/*
 *	Start moving the actor.
 */

void Game_window::start_actor
	(
	int winx, int winy		// Move towards this win. location.
	)
	{
	if (mode != normal)
		return;
					// Move every 1/8 sec.
	main_actor->start(this,
		chunkx*chunksize + winx, chunky*chunksize + winy, 125);
	}

/*
 *	Stop the actor.
 */

void Game_window::stop_actor
	(
	)
	{
	main_actor->stop();		// Stop and set resting state.
	paint();
	}

/*
 *	Find a "roof" in the given chunk.
 *
 *	Output: 1 if found, else 0.
 */

int Game_window::find_roof
	(
	int cx, int cy			// Absolute chunk coords.
	)
	{
	Chunk_object_list *olist = objects[cx][cy];
	if (!olist)
		return (0);
	return (olist->is_roof());
#if 0
	Game_object *obj;
	for (obj = olist->get_first(); obj; obj = olist->get_next(obj))
		if (obj->get_lift() >= 5)
			return (1);	// Found one.
	return (0);
#endif
	}

/*
 *	Find objects that can be selected, dragged, or activated.
 *	The last one in the list is the 'highest' in terms of lift, with
 *	objects visible in a 'gump' the highest.
 *
 *	Output:	# of objects stored.
 */

int Game_window::find_objects
	(
	int x, int y,			// Pos. on screen.
	Game_object **list		// Objects found are stored here.
	)
	{
	int cnt = 0;
	int actor_lift = main_actor->get_lift();
	int start = actor_lift > 0 ? -1 : 0;
					// See what was clicked on.
	for (int lift = start; lift < 3; lift++)
		cnt += find_objects(actor_lift + lift, x, y, &list[cnt]);
//+++++++++Look in gumps.
	return (cnt);
	}

/*
 *	Find objects at a given position on the screen with a given lift.
 *
 *	Output: # of objects, stored in list.
 */

int Game_window::find_objects
	(
	int lift,			// Look for objs. with this lift.
	int x, int y,			// Pos. on screen.
	Game_object **list		// Objects found are stored here.
	)
	{
//	cout << "Find_objects:	mouse is at (" << x << ',' << y << ")\n";
					// Figure chunk #'s.
	int start_cx = chunkx + (x + 4*lift)/chunksize;
	int start_cy = chunky + (y + 4*lift)/chunksize;
	Game_object *obj;
	int cnt = 0;			// Count # found.
					// Check 1 chunk down & right too.
	for (int ycnt = 0; ycnt < 2; ycnt++)
		{
		int cy = start_cy + ycnt;
		for (int xcnt = 0; xcnt < 2; xcnt++)
			{
			int cx = start_cx + xcnt;
			Chunk_object_list *olist = objects[cx][cy];
			if (!olist)
				continue;
			for (obj = olist->get_first(); obj; 
						obj = olist->get_next(obj))
				{
				if (obj->get_lift() != lift)
					continue;
				Rectangle r = get_shape_rect(obj);
				if (r.has_point(x, y))
					list[cnt++] = obj;
				}
			}
		}
	return (cnt);
	}

/*
 *	Show the names of the items the mouse is clicked on.
 */

void Game_window::show_items
	(
	int x, int y			// Coords. in window.
	)
	{
	Game_object *found[100];	// See what was clicked on.
	int cnt = find_objects(x, y, found);
#if 0
	for (int i = 0; i < cnt; i++)	// Go through them.
		{
		Game_object *obj = found[i];
					// Show name.
		char *item_name = item_names[obj->get_shapenum()];
		if (item_name)
			{
					// Stagger if more than 1.
			int tx = x + i*win->get_text_width(font12, " ");
			int ty = y + i*win->get_text_height(font12);
			add_text(item_name, tx, ty);
			}
		}
#else
					// Just do top item.
	if (cnt)
		{
		Game_object *obj = found[cnt - 1];
					// Show name.
		char *item_name = obj->get_name();
		if (item_name)
			add_text(item_name, x, y);
//++++++++Testing
#if 1
		int shnum = obj->get_shapenum();
		Shape_info& info = shapes.get_info(shnum);
		cout << "Object " << shnum << " has 3d tiles (x, y, z): " <<
			info.get_3d_xtiles() << ", " <<
			info.get_3d_ytiles() << ", " <<
			info.get_3d_height() << '\n';
#endif
		}
#endif
	}

/*
 *	Add a text object at a given spot.
 */

void Game_window::add_text
	(
	char *msg,
	int x, int y			// Pixel coord. on screen.
	)
	{
	Text_object *txt = new Text_object(msg,
		chunkx + x/chunksize, chunky + y/chunksize,
		(x%chunksize)/tilesize, (y%chunksize)/tilesize,
				8 + win->get_text_width(font12, msg),
				8 + win->get_text_height(font12));
	paint_text(txt);		// Draw it.
	txt->next = texts;		// Insert into chain.
	txt->prev = 0;
	if (txt->next)
		txt->next->prev = txt;
	texts = txt;
					// Show for a couple seconds.
	unsigned long curval = SDL_GetTicks();
	tqueue->add(curval + 2000, txt, (long) this);
	}

/*
 *	Remove a text item from the chain and delete it.
 *	Note:  It better not still be in the time queue.
 */

void Game_window::remove_text
	(
	Text_object *txt
	)
	{
	if (txt->next)
		txt->next->prev = txt->prev;
	if (txt->prev)
		txt->prev->next = txt->next;
	else				// Head of chain.
		texts = txt->next;
	delete txt;
	}

/*
 *	Remove all text items.
 */

void Game_window::remove_all_text
	(
	)
	{
	if (!texts)
		return;
	while (texts)
		{
		tqueue->remove(texts);	// Remove from time queue if there.
		remove_text(texts);
		}
	paint();			// Just paint whole screen.
	}

/*
 *	Handle a double-click.
 */

void Game_window::double_clicked
	(
	int x, int y			// Coords in window.
	)
	{
	Game_object *found[100];	// See what was clicked on.
	int cnt = find_objects(x, y, found);
//	cout << cnt << " objects found.\n";
	remove_all_text();		// Remove text msgs. from screen.
	for (int i = 0; i < cnt; i++)	// Go through them.
		{
		Game_object *obj = found[i];
cout << "Object name is " << obj->get_name() << '\n';
		enum Game_mode save_mode = mode;
		init_faces();		// Be sure face list is empty.
		obj->activate(usecode);
		npc_prox->wait(4);	// Delay "barking" for 4 secs.
		mode = save_mode;
		paint();//????Not sure+++++++++
		}
	}

/*
 *	Store information about an NPC's face and text on the screen during
 *	a conversation:
 */
class Npc_face_info
	{
	int shape;			// NPC's shape #.
	unsigned char text_pending;	// Text has been written, but user
					//   has not yet been prompted.
	Rectangle face_rect;		// Rectangle where face is shown.
	Rectangle text_rect;		// Rectangle NPC statement is shown in.
	friend class Game_window;
	Npc_face_info(int sh) : shape(sh), text_pending(0)
		{  }
	};

/*
 *	Initialize face list.
 */

void Game_window::init_faces
	(
	)
	{
	for (int i = 0; i < num_faces; i++)
		{
		delete face_info[i];
		face_info[i] = 0;
		}
	num_faces = 0;
	last_face_shown = -1;
	}

/*
 *	Show a "face" on the screen.  Npc_text_rect is also set.
 */

void Game_window::show_face
	(
	int shape,			// Shape (NPC #).
	int frame
	)
	{
	const int max_faces = sizeof(face_info)/sizeof(face_info[0]);
	if (num_faces == max_faces)
		{
		cout << "Can't show more than " << max_faces << " faces\n";
		return;
		}
					// Get character's portrait.
	Shape_frame *face = faces.get_shape(shape, frame);
	Npc_face_info *info = 0;
	Rectangle actbox;		// Gets box where face goes.
					// See if already on screen.
	for (int i = 0; i < num_faces; i++)
		if (face_info[i]->shape == shape)
			{
			info = face_info[i];
			last_face_shown = i;
			break;
			}
//++++++Really should look for empty slot & use it.
	if (!info)			// New one?
		{
					// Get last one shown.
		Npc_face_info *prev = num_faces ? face_info[num_faces - 1] : 0;
		info = new Npc_face_info(shape);
		last_face_shown = num_faces;
		face_info[num_faces++] = info;
					// Get screen rectangle.
		Rectangle sbox = get_win_rect();
					// Get text height.
		int text_height = win->get_text_height(font12);
					// Figure starting y-coord.
		int starty = prev ? prev->text_rect.y + prev->text_rect.h +
					2*text_height : 16;
		actbox = Rectangle(16, starty,
			face->get_width() + 4, face->get_height() + 4);
					// Clip to screen.
		actbox = sbox.intersect(actbox);
		info->face_rect = actbox;
					// This is where NPC text will go.
		info->text_rect = Rectangle(actbox.x + actbox.w + 16,
				actbox.y,
				sbox.w - actbox.x - actbox.w - 32,
				8*text_height);
		info->text_rect = sbox.intersect(info->text_rect);
		}
	else
		actbox = info->face_rect;
					// Draw whom we're talking to.
					// Put a black box w/ white bdr.
	win->fill8(1, actbox.w + 4, actbox.h + 4, actbox.x - 2, actbox.y - 2);
	win->fill8(0, actbox.w, actbox.h, actbox.x, actbox.y);
	paint_shape(win, actbox.x + actbox.w - 2 - 8, 
			actbox.y + actbox.h - 2 - 8, face);
	}

/*
 *	Remove face from screen.
 */

void Game_window::remove_face
	(
	int shape
	)
	{
	int i;				// See if already on screen.
	for (i = 0; i < num_faces; i++)
		if (face_info[i]->shape == shape)
			break;
	if (i == num_faces)
		return;			// Not found.
	Npc_face_info *info = face_info[i];
	paint(info->face_rect.x - 8, info->face_rect.y - 8,
		info->face_rect.w + 16, info->face_rect.h + 16);
	paint(info->text_rect);
	delete face_info[i];
	face_info[i] = 0;
	num_faces--;
	if (last_face_shown == i)	// Just in case.
		last_face_shown == num_faces - 1;
	}

/*
 *	Show what the NPC had to say.
 */

void Game_window::show_npc_message
	(
	char *msg
	)
	{
	if (last_face_shown == -1)
		return;
	Npc_face_info *info = face_info[last_face_shown];
	Rectangle& box = info->text_rect;
	paint(box);			// Clear what was there before.
	win->draw_text_box(font12, msg, box.x, box.y,
					box.w, box.h);
	info->text_pending = 1;
	painted = 1;
	show();
	}

/*
 *	Is there NPC text that the user hasn't had a chance to read?
 */

int Game_window::is_npc_text_pending
	(
	)
	{
	for (int i = 0; i < num_faces; i++)
		if (face_info[i]->text_pending)
			return (1);
	return (0);
	}

/*
 *	Show the Avatar's conversation choices (and face).
 */

void Game_window::show_avatar_choices
	(
	int num_choices,
	char **choices
	)
	{
	mode = conversation;
					// Get screen rectangle.
	Rectangle sbox = get_win_rect();
	int x = 0, y = 0;		// Keep track of coords. in box.
	int height = win->get_text_height(font12);
	int space_width = win->get_text_width(font12, "   ");
					// Get main actor's portrait.
	Shape_frame *face = faces.get_shape(main_actor->get_face_shapenum());

	Rectangle mbox(16, sbox.h - face->get_height() - 3*height,
//npc_text_rect.y + npc_text_rect.h + 6*height,
			face->get_width() + 4, face->get_height() + 4);
	win->fill8(1, mbox.w + 4, mbox.h + 4, mbox.x - 2, mbox.y - 2);
	win->fill8(0, mbox.w, mbox.h, mbox.x, mbox.y);
					// Draw portrait.
	paint_shape(win, mbox.x + mbox.w - 2 - 8, 
				mbox.y + mbox.h - face->ybelow - 2 - 8, 
				face);
					// Set to where to draw sentences.
	Rectangle tbox(mbox.x + mbox.w + 16, mbox.y + 8,
				sbox.w - mbox.x - mbox.w - 32,
				sbox.h - mbox.y - 16);
	paint(tbox);			// Paint background.
	delete [] conv_choices;		// Set up new list of choices.
	conv_choices = new Rectangle[num_choices + 1];
	for (int i = 0; i < num_choices; i++)
		{
		char *text = choices[i];
		int width = win->get_text_width(font12, text);
		if (x > 0 && x + width > tbox.w)
			{		// Start a new line.
			x = 0;
			y += height;
			}
					// Store info.
		conv_choices[i] = Rectangle(tbox.x + x, tbox.y + y,
					width, height);
		win->draw_text(font12, text, tbox.x + x, tbox.y + y);
		x += width + space_width;
		}
					// Terminate the list.
	conv_choices[num_choices] = Rectangle(0, 0, 0, 0);
	for (int i = 0; i < num_faces; i++)	// Clear 'pending' flags.
		face_info[i]->text_pending = 0;
	painted = 1;
	}

/*
 *	User clicked during a conversation.
 *
 *	Output:	Index (0-n) of choice, or -1 if not on a choice.
 */

int Game_window::conversation_choice
	(
	int x, int y			// Where mouse was clicked.
	)
	{
	int i;
	for (i = 0; conv_choices[i].w != 0 &&
			!conv_choices[i].has_point(x, y); i++)
		;
	if (conv_choices[i].w != 0)	// Found one?
		return (i);
	else
		return (-1);
	}

/*
 *	Show a gump.
 */

void Game_window::show_gump
	(
	Game_object *obj,		// Container gump represents.
	int shapenum			// Shape # in 'gumps.vga'.
	)
	{
	extern void Gump_events();
	int x = get_width()/2, y = get_height()/2;
	Gump_object *new_gump = new Gump_object(obj, x, y, shapenum);
					// Paint new one last.
	new_gump->append_to_chain(open_gumps);
	paint();			// Show everything.
	if (open_gumps == new_gump)	// First one?
		{
		Game_mode savemode = mode;
		mode = gump;
		Gump_events();		// Go into gump mode.
					// For now, this seems right...
		new_gump->remove_from_chain(open_gumps);
		paint();
		mode = savemode;
		}
					// Else we're already in it.
	}

/*
 *	Add NPC's in a given range of chunk to the queue for nearby NPC's.
 */

void Game_window::add_nearby_npcs
	(
	int from_cx, int from_cy,	// Starting chunk coord.
	int stop_cx, int stop_cy	// Go up to, but not including, these.
	)
	{
	unsigned long curtime = SDL_GetTicks();
	for (int cy = from_cy; cy != stop_cy; cy++)
		for (int cx = from_cx; cx != stop_cx; cx++)
			for (Npc_actor *npc = get_objects(cx, cy)->get_npcs();
						npc; npc = npc->get_next())
				if (!npc->is_nearby())
					{
					npc->set_nearby();
					npc_prox->add(curtime, npc);
					}
	}

/*
 *	Tell all npc's to update their schedules at a new 3-hour period.
 */

void Game_window::schedule_npcs
	(
	int hour3			// 0=midnight, 1=3am, 2=6am, etc.
	)
	{
					// Go through npc's.
	for (int i = 1; i < num_npcs; i++)
		((Npc_actor *) npcs[i])->update_schedule(this, hour3);
	paint();			// Repaint all.
	}

/*
 *	End intro.
 */

void Game_window::end_intro
	(
	)
	{
	if (mode == intro)
		{
		mode = normal;
		init_actors();		// Set up actors if not already done.
		paint();
					// Start with Iolo.
		mode = conversation;
		usecode->call_usecode(0x401, npcs[1], 
					Usecode_machine::game_start);
		paint();
		mode = normal;
					// Want to run proximity usecode on
					//   the visible ones.
		add_nearby_npcs(chunkx, chunky, 
			chunkx + (get_width() + chunksize - 1)/chunksize,
			chunky + (get_height() + chunksize - 1)/chunksize);
		}
	}
