/**
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

#if !(defined(XWIN) || defined(DOS)) //avoid crash in WIN32
void gettimeofday(timeval* tv, int x); //in objs.cpp
#endif

/*
 *	Create game window.
 */

Game_window::Game_window
	(
	int width, int height		// Window dimensions.
	) : chunkx(0), chunky(0), painted(0), focus(1),
	    brightness(100), 
	    skip_lift(16), debug(0), shapewin(0),
	    tqueue(new Time_queue()), clock(tqueue),
		npc_prox(new Npc_proximity_handler(this)),
	    main_actor(0),
	    conv_choices(0),
	    main_actor_inside(0), mode(intro), showing_item(0), npcs(0),
	    shapes(SHAPES_VGA),
	    faces(FACES_VGA),
	    gumps(GUMPS_VGA)
	{
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
	if (!shapes.read_dims(SHPDIMS_DAT, 150, 1024 - 150))
		abort("Can't open 'shpdims.dat' file.");
					// Create window.
	win = new Image_window(width, height); //<- error in timer

					// Get 12-point font.
	font12 = win->open_font(AVATAR_TTF, 12);
					// Set title.
	win->set_title("Exult Ultima7 Viewer");

	struct timeval timer;
	gettimeofday(&timer, 0);	// Get time of day.
	srand(timer.tv_usec);		// Use it to seed rand. generator.
	timer.tv_sec = 0;		// Force clock to start.
	tqueue->add(timer, &clock, 0);
					// Clear object lists, flags.
	memset((char *) objects, 0, sizeof(objects));
	memset((char *) schunk_read, 0, sizeof(schunk_read));
	get_palette(0);			// Try first palette.
	brighten(20);			// Brighten 20%.
	win->map();			// Now display window.
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
	delete shapewin;
	delete [] conv_choices;
	}

/*
 *	Create a window for showing individual shapes.
 */

void Game_window::open_shape_window
	(
	)
	{
#ifndef DOS
	if (shapewin)
		return;			// Already done.
					// Make it 128x128.
	shapewin = new Image_window(128, 128);
					// Indicate events we want.
#ifdef XWIN
	shapewin->select_input(ExposureMask |
		ButtonPressMask | StructureNotifyMask);
#endif
					// Set title.
	shapewin->set_title("Exult Shapes");
	shapewin->map();		// Show it on screen.
	get_palette(0, brightness);	// Re-read palette.
#endif
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
	Window xwin,
	unsigned int neww, 
	unsigned int newh
	)
	{			
					// Pick right window.
	if (shapewin && xwin == shapewin->get_win())
		{
		shapewin->resized(neww, newh);
		shapewin_paint(shapewin_cur);
		}
	else
		{
		win->resized(neww, newh);
		paint();
		}
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

#if 1

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
 *	Show the shape the mouse was clicked on.
 */

void Game_window::debug_shape
	(
	Window xwin,			// Which window.
	int x, int y			// Point on screen.
	)
	{
					// Clicked in shapes window?
	if (shapewin && xwin == shapewin->get_win())
		{			// Show next shape in list.
		shapewin_paint(shapewin_cur + 1);
		shapewin->show();
		return;
		}
	cout << "Mouse is at " << x << ',' << y << '\n';
	shapewin_cur = 0;
	for (int lift = 0; lift < 16; lift ++)
		find_debug_shapes(lift, x, y);
					// Set EOL entry.
	shapewin_objs[shapewin_cur] = Game_object();
	shapewin_cur = 0;
	if (!shapewin)			// Open if first time.
		open_shape_window();
	else
		{
		shapewin_paint(0);	// Already open, so update it.
		shapewin->show();
		}
	}

/*
 *	Find objects at a given position on the screen with a given lift.
 *
 *	Output: Objects are stored in shapewin_objs list.
 */

void Game_window::find_debug_shapes
	(
	int lift,			// Look for objs. with this lift.
	int x, int y			// Pos. on screen.
	)
	{


	x += 4*lift;			// Look for given lift.
	y += 4*lift;
	int cx = x/chunksize;		// Figure chunk #'s within window.
	int cy = y/chunksize;
	int xoff = cx*chunksize;
	int yoff = cy*chunksize;
					// Figure shape within chunk.
	int sx = (x - xoff)/8;
	int sy = (y - yoff)/8;
	cx += chunkx;			// Figure abs. chunk coords.
	cy += chunky;
	Game_object *obj;
	Chunk_object_list *olist = objects[cx][cy];
	if (!olist)
		return;
	for (obj = olist->get_first(); obj; obj = olist->get_next(obj))
		{
		if (obj->get_lift() != lift)
			continue;
		int shapex = obj->get_shape_pos_x();
		int shapey = obj->get_shape_pos_y();
		if (shapex != sx || shapey != sy)
			continue;
		int shapenum = obj->get_shapenum();
		int framenum = obj->get_framenum();
					// Get data from shpdims.dat.
		int dimw = shapes.dims[(shapenum-150)*2];
		int dimh = shapes.dims[(shapenum-150)*2 + 1];
		cout << "\tShape " << shapenum << 
				", frame " << framenum <<
				" at chunk coord (" << cx << ',' <<
				cy <<
				"), shape coord (" << shapex << ',' <<
				shapey << "), lift " << 
				obj->get_lift() << ", shpdims = (" <<
				dimw << ',' << dimh << ')'
				<< '\n';
		Shape_frame *shape = get_shape(shapenum, framenum);
		if (shape)
			{
			Rectangle r = get_shape_rect(obj, cx, cy);
			cout << "\t\t(w0,w1,h0,h1) = (" << shape->xleft << 
				',' <<
				shape->xright  << ',' << shape->yabove << 
				',' <<
				shape->ybelow << ")";
			cout << "  Rect = (" << r.x << ',' << r.y << ',' <<
				r.w << ',' << r.h << ")\n";
			}
					// Store shapes in list.
		shapewin_objs[shapewin_cur++] = *obj;
		}
	}

/*
 *	Set the shape in the shape window.
 */

void Game_window::shapewin_paint
	(
	int num				// Number within list to show.
	)
	{
	shapewin_cur = num;
					// Watch for end of list.
	if (num >= sizeof(shapewin_objs)/sizeof(shapewin_objs[0]) ||
	    shapewin_objs[num].is_eol())
		shapewin_cur = 0;
	Game_object *obj = &shapewin_objs[shapewin_cur];
	if (obj->is_eol())
		return;			// Nothing's valid.
	char title[120];		// Set title.
	sprintf(title, "Shape %d, frame %d", obj->get_shapenum(),
						obj->get_framenum());
	shapewin->set_title(title);
	shapewin->fill8(0);		// Fill (with black).
	paint_shape(shapewin, 64, 64, obj->get_shapenum(), obj->get_framenum());
	}
#endif

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
#if 0
		olist->set(rles);	// Store list (already sorted by
					//   (y, x)).
#else
		while (rles)
			{
			Game_object *nxt = rles->get_next();
			olist->add(rles);
			rles = nxt;
			}
#endif
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
			Container_game_object *cobj = 
				new Container_game_object(
				entry[2], entry[3], shapex, shapey, lift);
					// Read container's objects.
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
					// Want to run proximity usecode on
					//   the visible ones.
	add_nearby_npcs(chunkx, chunky, 
			chunkx + (get_width() + chunksize - 1)/chunksize,
			chunky + (get_height() + chunksize - 1)/chunksize);
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
		win->draw_text_box(font, "Welcome to EXULT V 0.10, a free RPG game engine.\n\nCopyright 2000 J. S. Freedman\nGraphics copyrighted by Origin\nText rendered by FreeType", x, y, 600 < w ? 600 : w, 400 < h ? 400 : h);
		}
	if (showing_item)		// ID'ing an item?
		win->draw_text(font12, showing_item, showing_rect.x,
							showing_rect.y);
	win->clear_clip();
	painted = 1;
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
		if (main_actor_inside && lift >= 5)
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
	if (shapewin)
		shapewin->set_palette(colors, 63, brightness);
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
	timeval& time			// Current time of day.
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
		chunkx*chunksize + winx, chunky*chunksize + winy, 125000);
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
				Rectangle r = get_shape_rect(obj, cx, cy);
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
	Game_object *found[100];
	int cnt = 0;
					// See what was clicked on.
	for (int lift = 0; lift < 3; lift++)
		cnt += find_objects(main_actor->get_lift() + lift, 
							x, y, &found[cnt]);
	for (int i = 0; i < cnt; i++)	// Go through them.
		{
		Game_object *obj = found[i];
					// Show name.
		char *item_name = item_names[obj->get_shapenum()];
		if (item_name)
			{		// Just do the first one found.
			showing_item = item_name;
			showing_rect.x = x;
			showing_rect.y = y;
			showing_rect.w = 8 + win->get_text_width(
							font12, item_name);
			showing_rect.h = 8 + win->get_text_height(font12);
					// Clip to screen.
			showing_rect = clip_to_win(showing_rect);
			paint(showing_rect.x, showing_rect.y,
				showing_rect.w, showing_rect.h);
			return;
			}
		}
	}

/*
 *	Handle a double-click.
 */

void Game_window::double_clicked
	(
	Window xwin,			// Window it occurred in.
	int x, int y			// Coords in window.
	)
	{
	if (xwin != win->get_win())
		return;			// Not the main window.
	Game_object *found[100];
	int cnt = 0;
					// See what was clicked on.
	for (int lift = 0; lift < 3; lift++)
		cnt += find_objects(main_actor->get_lift() + lift, 
							x, y, &found[cnt]);
//	cout << cnt << " objects found.\n";
	for (int i = 0; i < cnt; i++)	// Go through them.
		{
		Game_object *obj = found[i];
cout << "Object name is " << obj->get_name() << '\n';
		enum Game_mode save_mode = mode;
		obj->activate(usecode);
		mode = save_mode;
		paint();//????Not sure+++++++++
		}
	}

/*
 *	Show a "face" on the screen.  Npc_text_rect is also set.
 *	+++++++Got to handle more than one.
 */

void Game_window::show_face
	(
	int shape,			// Shape (NPC #).
	int frame
	)
	{
					// Get screen rectangle.
	Rectangle sbox = get_win_rect();
					// Get character's portrait.
	Shape_frame *face = faces.get_shape(shape, frame);
					// Draw at top of screen.
	Rectangle actbox(16, 16,
			face->get_width() + 4, face->get_height() + 4);
					// Draw whom we're talking to.
					// Put a black box w/ white bdr.
	win->fill8(1, actbox.w + 4, actbox.h + 4, actbox.x - 2, actbox.y - 2);
	win->fill8(0, actbox.w, actbox.h, actbox.x, actbox.y);
	paint_shape(win, actbox.x + actbox.w - 2 - 8, 
			actbox.y + actbox.h - 2 - 8, face);
					// This is where NPC text will go.
	npc_text_rect = Rectangle(actbox.x + actbox.w + 16,
				actbox.y + 8,
				sbox.w - actbox.x - actbox.w - 32,
				6*win->get_text_height(font12));
	}

/*
 *	Show what the NPC had to say.
 */

void Game_window::show_npc_message
	(
	char *msg
	)
	{
	paint(npc_text_rect);		// Clear what was there before.
	win->draw_text_box(font12, msg, npc_text_rect.x, npc_text_rect.y,
					npc_text_rect.w, npc_text_rect.h);
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
					// Get screen rectangle.
	Rectangle sbox = get_win_rect();
	int x = 0, y = 0;		// Keep track of coords. in box.
	int height = win->get_text_height(font12);
	int space_width = win->get_text_width(font12, "   ");
					// Get main actor's portrait.
	Shape_frame *face = faces.get_shape(main_actor->get_face_shapenum());

	Rectangle mbox(16, npc_text_rect.y + npc_text_rect.h + 6*height,
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
	mode = conversation;
	}

/*
 *	User clicked during a conversation.
 */

void Game_window::conversation_choice
	(
	int x, int y			// Where mouse was clicked.
	)
	{
	int i;
	for (i = 0; conv_choices[i].w != 0 &&
			!conv_choices[i].has_point(x, y); i++)
		;
	if (conv_choices[i].w != 0)	// Found one?
		usecode->chose_response(i);
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
	timeval curtime;
	gettimeofday(&curtime, 0);
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
 *	End intro.
 */

void Game_window::end_intro
	(
	)
	{
	if (mode == intro)
		{
		mode = normal;
		paint();
					// Start with Iolo.
#if 0	/*+++++++Doesn't work yet. */
		usecode->call_usecode(0x401, npcs[1], 
					Usecode_machine::game_start);
#endif
		}
	}
