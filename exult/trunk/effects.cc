/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Effects.cc - Special effects.
 **
 **	Written: 5/25/2000 - JSF
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

#include "gamewin.h"
#include "actors.h"
#include "effects.h"
#include "Zombie.h"
#include "dir.h"

int Cloud::randcnt = 0;

/*
 *	Some special effects may not need painting.
 */

void Special_effect::paint
	(
	Game_window *
	)
	{
	}

/*
 *	Create an animation from the 'sprites.vga' file.
 */

Sprites_effect::Sprites_effect
	(
	int num,			// Index.
	int px, int py			// Screen location.
	) : sprite_num(num), frame_num(0), tx(px), ty(py)
	{
	Game_window *gwin = Game_window::get_game_window();
	frames = gwin->get_sprite_num_frames(num);
					// Start immediately.
	gwin->get_tqueue()->add(SDL_GetTicks(), this, 0L);
	}

/*
 *	Animation.
 */

void Sprites_effect::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata
	)
	{
	const int delay = 50;		// Delay between frames.
	Game_window *gwin = Game_window::get_game_window();
	if (frame_num == frames)	// At end?
		{			// Remove & delete this.
		gwin->remove_effect(this);
		gwin->paint();
		return;
		}
	Sprites_effect::paint(gwin);	// Render.
	gwin->set_painted();
	frame_num++;			// Next frame.
					// Add back to queue for next time.
	gwin->get_tqueue()->add(curtime + delay, this, udata);
	}

/*
 *	Render.
 */

void Sprites_effect::paint
	(
	Game_window *gwin
	)
	{
	gwin->paint_sprite((tx - gwin->get_scrolltx())*tilesize,
		(ty - gwin->get_scrollty())*tilesize, sprite_num, frame_num);
	}

/*
 *	Get direction in 1/16's starting from North.
 */

inline int Get_dir16
	(
	Tile_coord& t1,
	Tile_coord& t2
	)
	{
					// Treat as cartesian coords.
	return Get_direction16(t1.ty - t2.ty, t2.tx - t1.tx);
	}

/*
 *	Initialize path & add to time queue.
 */

void Projectile_effect::init
	(
	Tile_coord s,			// Source,
	Tile_coord d			// Destination.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	frames = gwin->get_shape_num_frames(shape_num);
	pos = s;			// Get starting position.
	path = new Zombie();		// Create simple pathfinder.
					// Find path.  Should never fail.
	path->NewPath(pos, d, 0);
	if (frames >= 24)		// Use frames 8-23, for direction
		{			//   going clockwise from North.
		int dir = Get_dir16(s, d);
		frame_num = 8 + dir;
		}
	else if (frames == 1)
		frame_num == 0;
	else
		frame_num = -1;		// We just won't show it.
					// Start immediately.
	gwin->get_tqueue()->add(SDL_GetTicks(), this, 0L);
	}


/*
 *	Create a projectile animation.
 */

Projectile_effect::Projectile_effect
	(
	Actor *att,			// Source of spell/attack.
	Game_object *to,		// End here, 'attack' it with shape.
	int shnum,			// Projectile shape # in 'shapes.vga'.
	int weap			// Weapon (bow, gun, etc.) shape, or 0.
	) : attacker(att), target(to), shape_num(shnum), weapon(weap),
		frame_num(0)
	{
	init(attacker->get_abs_tile_coord(), to->get_abs_tile_coord());
	}

/*
 *	Constructor used by missile eggs.
 */

Projectile_effect::Projectile_effect
	(
	Tile_coord s,			// Start here.
	Tile_coord d,			// End here.
	int shnum,			// Projectile shape
	int weap			// Weapon (bow, gun, etc.) shape, or 0.
	) : attacker(0), target(0), shape_num(shnum), weapon(weap), 
		frame_num(0)
	{
	init(s, d);
	}

/* 
 *	Another used by missile eggs.
 */

Projectile_effect::Projectile_effect
	(
	Tile_coord s,			// Start here.
	Game_object *to,		// End here, 'attack' it with shape.
	int shnum,			// Projectile shape
	int weap			// Weapon (bow, gun, etc.) shape, or 0.
	) : attacker(0), target(to), shape_num(shnum), weapon(weap), 
		frame_num(0)
	{
	init(s, to->get_abs_tile_coord());
	}

/*
 *	Delete.
 */

Projectile_effect::~Projectile_effect
	(
	)
	{
	delete path;
	}

/*
 *	Add a dirty rectangle for the current position and frame.
 */

inline void Projectile_effect::add_dirty
	(
	Game_window *gwin
	)
	{
	if (pos.tx == -1 || frame_num == -1)
		return;			// Already at destination.
	Shape_frame *shape = gwin->get_shape(shape_num, frame_num);
					// Force repaint of prev. position.
	int liftpix = pos.tz*tilesize/2;
	gwin->add_dirty(gwin->clip_to_win(gwin->get_shape_rect(shape,
			(pos.tx - gwin->get_scrolltx())*tilesize - liftpix,
	    (pos.ty - gwin->get_scrollty())*tilesize - liftpix).enlarge(4)));
	}

/*
 *	See if something was hit.
 *
 *	Output:	->target hit, or 0.
 */

inline Game_object *Find_target
	(
	Game_window *gwin,
	Tile_coord pos
	)
	{
	if (pos.tz%5 == 0)		// On floor?
		pos.tz++;		// Look up 1 tile.
	Tile_coord dest = pos;		// This gets modified.
	if (!Chunk_object_list::is_blocked(pos, 1, MOVE_FLY, 0) &&
	    dest == pos)
		return (0);
	return Game_object::find_blocking(pos);
	}

/*
 *	Animation.
 */

void Projectile_effect::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata
	)
	{
	const int delay = 100;		// Delay between frames.
	Game_window *gwin = Game_window::get_game_window();
	add_dirty(gwin);		// Force repaint of old pos.
	if (!path->GetNextStep(pos) ||	// Get next spot.
					// If missile egg, detect target.
			(!target && (target = Find_target(gwin, pos)) != 0))
		{			// Done? 
		if (target)
			target->attacked(attacker, weapon, shape_num);
		pos.tx = -1;		// Signal we're done.
		gwin->remove_effect(this);
		return;
		}
	add_dirty(gwin);		// Paint new spot/frame.
					// Add back to queue for next time.
	gwin->get_tqueue()->add(curtime + delay, this, udata);
	}

/*
 *	Render.
 */

void Projectile_effect::paint
	(
	Game_window *gwin
	)
	{
	if (pos.tx == -1 || frame_num == -1)
		return;			// Already at destination.
	int liftpix = pos.tz*tilesize/2;
	gwin->paint_shape((pos.tx - gwin->get_scrolltx())*tilesize - liftpix,
		(pos.ty - gwin->get_scrollty())*tilesize - liftpix, 
		shape_num, frame_num);
	}

/*
 *	Create a text object.
 */

Text_effect::Text_effect
	(
	const char *m, 			// A copy is made.
	int t_x, int t_y, 		// Abs. tile coords.
	int w, int h
	) : msg(strdup(m)), tx(t_x), ty(t_y), width(w), height(h)
	{
	}

/*
 *	Remove from screen.
 */

void Text_effect::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// Ignored.
	)
	{
	Game_window *gwin = (Game_window *) udata;
					// Repaint slightly bigger rectangle.
	Rectangle rect((tx - gwin->get_scrolltx() - 1)*tilesize,
		       (ty - gwin->get_scrollty() - 1)*tilesize,
			width + 2*tilesize, height + 2*tilesize);
					// Intersect with screen.
	rect = gwin->clip_to_win(rect);
	gwin->remove_effect(this);	// Remove & delete this.
	if (rect.w > 0 && rect.h > 0)	// Watch for negatives.
		gwin->paint(rect.x, rect.y, rect.w, rect.h);

	}

/*
 *	Render.
 */

void Text_effect::paint
	(
	Game_window *gwin
	)
	{
	const char *ptr = msg;
	if (*ptr == '@')
		ptr++;
	int len = strlen(ptr);
	if (ptr[len - 1] == '@')
		len--;
	gwin->paint_text(0, ptr, len, (tx - gwin->get_scrolltx())*tilesize,
				(ty - gwin->get_scrollty())*tilesize);
	}

/*
 *	Init. a weather effect.
 */

Weather_effect::Weather_effect
	(
	int duration,			// Length in game minutes.
	int delay			// Delay before starting.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	stop_time = SDL_GetTicks() + delay + 1000*((duration*60)/time_factor);
					// Start immediately.
	gwin->get_tqueue()->add(SDL_GetTicks() + delay, this, 0L);
	}

/*
 *	Paint raindrop.
 */

inline void Raindrop::paint
	(
	Image_window8 *iwin,		// Where to draw.
	int scrolltx, int scrollty,	// Tile at top-left corner.
	Xform_palette xform		// Transform array.
	)
	{
	unsigned long ascrollx = scrolltx*(unsigned long)tilesize,
		      ascrolly = scrollty*(unsigned long)tilesize;
	int x = ax - ascrollx, y = ay - ascrolly;
	if (x < 0 || y < 0 || 
			x >= iwin->get_width() || y >= iwin->get_height())
		return;
	if (oldpix == 255)
		oldpix = iwin->get_pixel8(x, y);	// Get pixel.
	iwin->put_pixel8(xform[oldpix], x, y);
	}

/*
 *	Move raindrop.
 */

inline void Raindrop::next
	(
	Image_window8 *iwin,		// Where to draw.	
	int scrolltx, int scrollty,	// Tile at top-left corner.
	Xform_palette xform,		// Transform array.
	int w, int h			// Dims. of window.
	)
	{
	unsigned long ascrollx = scrolltx*(unsigned long)tilesize,
		      ascrolly = scrollty*(unsigned long)tilesize;
	int x = ax - ascrollx, y = ay - ascrolly;
					// Still on screen?  Restore pix.
	if (x >= 0 && y >= 0 && x < w && y < h && oldpix != 255)
		iwin->put_pixel8(oldpix, x, y);
	oldpix = 255;
					// Time to restart?
	if (x < 0 || x >= w || y < 0 || y >= h)
		{			
		int r = rand();
					// Have a few fall faster.
		yperx = (r%4) ? 1 : 2;
		ax = ascrollx + r%(w - w/8);
		ay = ascrolly + r%(h - h/4);
		}
	else				// Next spot.
		{
		int delta = 1 + rand()%4;
		ax += delta;
		ay += delta + yperx;
		}
					// Save old pixel & paint new.
	paint(iwin, scrolltx, scrollty, xform);
	}

/*
 *	Rain.
 */

void Rain_effect::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	if (!gwin->is_main_actor_inside())
		{			// Don't show rain inside buildings!
		Image_window8 *win = gwin->get_win();
		int w = win->get_width(), h = win->get_height();
					// Get transform table.
		Xform_palette xform = gwin->get_xform(8);//++++Experiment.
		const int num_drops = sizeof(drops)/sizeof(drops[0]);
		int scrolltx = gwin->get_scrolltx(),
		    scrollty = gwin->get_scrollty();
					// Move drops.
		for (int i = 0; i < num_drops; i++)
			drops[i].next(win, scrolltx, scrollty, xform, w, h);
		gwin->set_painted();
		}
	if (curtime < stop_time)	// Keep going?
		gwin->get_tqueue()->add(curtime + 100, this, udata);
	else
		{
		gwin->set_all_dirty();
		gwin->remove_effect(this);
		}
	}

/*
 *	Paint rain.
 */

void Rain_effect::paint
	(
	Game_window *gwin
	)
	{
	if (gwin->is_main_actor_inside() || gwin->get_mode() !=
						Game_window::normal)
		return;			// Inside.
					// Get transform table.
	Xform_palette xform = gwin->get_xform(8);//++++Experiment.
	int scrolltx = gwin->get_scrolltx(),
	    scrollty = gwin->get_scrollty();
	Image_window8 *win = gwin->get_win();
	const int num_drops = sizeof(drops)/sizeof(drops[0]);
	for (int i = 0; i < num_drops; i++)
		drops[i].paint(win, scrolltx, scrollty, xform);
	gwin->set_painted();
	}

/*
 *	Lightning.
 */

void Lightning_effect::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int r = rand();			// Get a random #.
	int delay = 100;		// Delay for next time.
	if (save_brightness > 0)	// Just turned white?  Restore.
		{
		gwin->set_palette(-1, save_brightness);
		save_brightness = -1;
		gwin->show(1);
		if (curtime >= stop_time)
			{		// Time to stop.
			gwin->remove_effect(this);
			return;
			}
		if (r%5 < 2)		// Occassionally flash again.
			delay = (1 + r%7)*40;
		else			// Otherwise, wait several secs.
			delay = (4000 + r%4);
		}
	else				// Time to flash.
		{
		save_brightness = gwin->get_brightness();
		gwin->set_palette(-1, 400);
		gwin->show(1);
		delay = (1 + r%3)*50;
		}
	gwin->get_tqueue()->add(curtime + delay, this, udata);
	}

/*
 *	Start a storm.
 */

Storm_effect::Storm_effect
	(
	int duration,			// In game minutes.
	int delay			// In msecs.
	) : Weather_effect(duration, delay), start(1)
	{
	Game_window *gwin = Game_window::get_game_window();
					// Start raining soon.
	int rain_delay = 20 + rand()%1000;
	gwin->add_effect(new Rain_effect(duration - 1, rain_delay));
					// Then lightning.
	int lightning_delay = rain_delay + rand()%500;
	gwin->add_effect(new Lightning_effect(
			duration - 1, lightning_delay));
	}

/*
 *	Start/end of storm.
 */
void Storm_effect::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	if (start)
		{
		start = 0;
					// Darken sky.
		int brightness = gwin->get_brightness();
		brightness -= 20 + rand()%30;
		if (brightness < 20)
			brightness = 20;
		gwin->set_palette(-1, brightness);
					// Nothing more to do but end.
		gwin->get_tqueue()->add(stop_time, this, udata);
		}
	else				// Must be time to stop.
		gwin->remove_effect(this);
	}

/*
 *	Storm ended.
 */

Storm_effect::~Storm_effect
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	gwin->restore_users_brightness();
	}

/*
 *	Create a cloud.
 */

Cloud::Cloud
	(
	short dx, short dy		// Deltas for movement.
	) : frame(0), wx(0), wy(0), deltax(dx), deltay(dy), count(-1)
	{
	Game_window *gwin = Game_window::get_game_window();
					// Get abs. values.
	int adx = deltax > 0 ? deltax : -deltax;
	int ady = deltay > 0 ? deltay : -deltay;
	if (adx < ady)
		max_count = 2*gwin->get_height()/ady;
	else
		max_count = 2*gwin->get_width()/adx;
	start_time = 0;
	}

const int CLOUD = 2;		// Shape #.

/*
 *	Set starting screen position according to direction.
 */

void Cloud::set_start_pos
	(
	Shape_frame *shape,
	int w, int h,			// Dims. of window.
	int& x, int& y			// Screen pos. returned.
	)
	{
	if (!deltax)			// Special cases first.
		{
		x = rand()%w;
		y = deltay > 0 ? -shape->get_ybelow() : 
						h + shape->get_yabove();
		return;
		}
	if (!deltay)
		{
		y = rand()%h;
		x = deltax > 0 ? -shape->get_xright() : w + shape->get_xleft();
		return;
		}
	int halfp = w + h;		// 1/2 perimeter.
	int r = rand()%halfp;		// Start on one of two sides.
	if (r > h)			// Start on top/bottom.
		{
		x = r - h;
		y = deltay > 0 ? -shape->get_ybelow() : 
						h + shape->get_yabove();
		return;
		}
	y = r;				// On left or right side.
	if (deltax > 0)			// Going right?
		x = -shape->get_xright();
	else				// Going left?
		x = w + shape->get_xleft();
	}

/*
 *	Move cloud
 */

inline void Cloud::next
	(
	Game_window *gwin,		// Game window.
	unsigned long curtime,		// Current time of day.
	int w, int h			// Dims. of window.
	)
	{
	if (curtime < start_time)
		return;			// Not yet.
					// Get top-left world pos.
	long scrollx = gwin->get_scrolltx()*tilesize;
	long scrolly = gwin->get_scrollty()*tilesize;
	Shape_frame *shape = gwin->get_sprite_shape(CLOUD, frame);
	gwin->add_dirty(gwin->clip_to_win(gwin->get_shape_rect(
			shape, wx - scrollx, wy - scrolly).enlarge(4)));
	if (count <= 0)			// Time to restart?
		{
					// Set start time randomly.
		start_time = curtime + 2000*randcnt + rand()%2000;
		randcnt = (randcnt + 1)%4;
		start_time = SDL_GetTicks() + 2000*randcnt + rand()%500;
cout << "Cloud: start_time = " << start_time << endl;
		count = max_count;
		frame = rand()%gwin->get_sprite_num_frames(CLOUD);
		int x, y;		// Get screen pos.
		set_start_pos(shape, w, h, x, y);
		wx = x + scrollx;
		wy = y + scrolly;
		}
	else
		{
		wx += deltax;
		wy += deltay;
		count--;
		}
	gwin->add_dirty(gwin->clip_to_win(gwin->get_shape_rect(
			shape, wx - scrollx, wy - scrolly).enlarge(4)));
	}

/*
 *	Paint cloud.
 */

void Cloud::paint
	(
	Game_window *gwin
	)
	{
	if (count > 0)			// Might not have been started.
		gwin->paint_sprite(wx - gwin->get_scrolltx()*tilesize, 
			wy - gwin->get_scrollty()*tilesize, CLOUD, frame);
	}

/*
 *	Create a few clouds to float across the screen.
 */

Clouds_effect::Clouds_effect
	(
	int duration,			// In game minutes.
	int delay			// In msecs.
	) : Weather_effect(duration, delay)
	{
	num_clouds = 2 + rand()%5;	// Pick #.
	clouds = new Cloud *[num_clouds];
					// Figure wind direction.
	int dx = rand()%5 - 2;
	int dy = rand()%5 - 2;
	if (!dx && !dy)
		{
		dx = 1 + rand()%2;
		dy = 1 - rand()%3;
		}
#if 0
	if (dx != 0)			// Move a bit faster.
		dx += dx > 0 ? 1 : -1;
	if (dy != 0)
		dy += dy > 0 ? 1 : -1;
#endif
	for (int i = 0; i < num_clouds; i++)
		{			// Modify speed of some.
		int deltax = dx, deltay = dy;
		if (rand()%2 == 0)
			{
			deltax += deltax/2;
			deltay += deltay/2;
			}
		clouds[i] = new Cloud(deltax, deltay);
		}
	}

/*
 *	Cloud drift.
 */

void Clouds_effect::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata
	)
	{
	const int delay = 100;
	Game_window *gwin = Game_window::get_game_window();
	if (curtime >= stop_time)
		{			// Time to stop.
		gwin->remove_effect(this);
		gwin->set_all_dirty();
		return;
		}
	int w = gwin->get_width(), h = gwin->get_height();
	for (int i = 0; i < num_clouds; i++)
		clouds[i]->next(gwin, curtime, w, h);
	gwin->get_tqueue()->add(curtime + delay, this, udata);
	}

/*
 *	Render.
 */

void Clouds_effect::paint
	(
	Game_window *gwin
	)
	{
	if (!gwin->is_main_actor_inside() && gwin->get_mode() == 
						Game_window::normal)
		for (int i = 0; i < num_clouds; i++)
			clouds[i]->paint(gwin);
	}

/*
 *	Shake the screen.
 */

void Earthquake::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Image_window *win = gwin->get_win();
	int w = win->get_width(), h = win->get_height();
	int sx = 0, sy = 0;
	int dx = rand()%9 - 4;
	int dy = rand()%9 - 4;
	if (dx > 0)
		w -= dx;
	else
		{
		w += dx;
		sx -= dx;
		dx = 0;
		}
	if (dy > 0)
		h -= dy;
	else
		{
		h += dy;
		sy -= dy;
		dy = 0;
		}
	win->copy(sx, sy, w, h, dx, dy);
	gwin->set_painted();
	gwin->show();
					// Shake back.
	win->copy(dx, dy, w, h, sx, sy);
	if (++i < len)			// More to do?  Put back in queue.
		gwin->get_tqueue()->add(curtime + 100, this, udata);
	else
		delete this;
	}

