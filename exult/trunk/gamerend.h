/*
 *	gamerend.h - Rendering methods.
 *
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
 *  Copyright (C) 2000-2013  The Exult Team
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

#ifndef GAMEREND_H
#define GAMEREND_H 1

/*
 *	A helper-class for rendering.
 */
class Game_render
	{
	unsigned long render_seq;	// For marking rendered objects.
	int skip;			// Set for each render.  We skip
					//   painting at or above this.
public:
	Game_render() : render_seq(0), skip(31)
		{  }
	void paint_terrain_only(int start_chunkx, int start_chunky,
				int stop_chunkx, int stop_chunky);
					// Render the map & objects.
	int paint_map(int x, int y, int w, int h);
					// Paint "flat" scenery in a chunk.
	void paint_chunk_flats(int cx, int cy, int xoff, int yoff);
	void paint_chunk_flat_rles(int cx, int cy, int xoff, int yoff);
	//				// Paint blackness in a dungeon
	//void paint_dungeon_black(int cx, int cy, int xoff, int yoff, int index=0);
					// Paint objects in given chunk at
					//   given lift.
	int paint_chunk_objects(int cx, int cy);
					// Paint an obj. after dependencies.
	void paint_object(Game_object *obj);
					// Render dungeon blackness
	void paint_blackness(int cx, int cy, int stop_chunkx, int stop_chunky,
							 int index=0);
	};

#endif
