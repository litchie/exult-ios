#ifdef MSVC_FIND_NEARBY_KLUDGE

// This is so the Compiler wont crash when attempting to compile Objs.cc
//
// Yes, you read right... CRASH!
//
// The damn thing will crash because it doesn't like <T*>

#ifndef FN_VECTOR
#error(Can't Include find_nearby.h if FN_VECTOR is not #define'd)
#endif

#ifndef FN_OBJECT
#error(Can't Include find_nearby.h if FN_OBJECT is not #define'd)
#endif

#ifndef FN_CAST
#error(Can't Include find_nearby.h if FN_CAST is not #define'd)
#endif

int Game_object::find_nearby
	(
	FN_VECTOR& vec,			// Objects appended to this.
	Tile_coord pos,			// Look near this point.
	int shapenum,			// Shape to look for.  
					//   -1=any (but always use mask?),
					//   c_any_shapenum=any.
	int delta,			// # tiles to look in each direction.
	int mask,			// See Check_mask() above.
	int qual,			// Quality, or c_any_qual for any.
	int framenum			// Frame #, or c_any_framenum for any.
	)
	{
	if (delta < 0)			// +++++Until we check all old callers.
		delta = 24;
	if (shapenum > 0 && mask == 4)	// Ignore mask=4 if shape given!
		mask = 0;
	int vecsize = vec.size();
	Game_window *gwin = Game_window::get_instance();
	Game_map *gmap = gwin->get_map();
	Rectangle tiles(pos.tx - delta, pos.ty - delta, 1 + 2*delta, 1 + 
								2*delta);
					// Stay within world.
	Rectangle world(0, 0, c_num_chunks*c_tiles_per_chunk, 
					c_num_chunks*c_tiles_per_chunk);
	tiles = tiles.intersect(world);
					// Figure range of chunks.
	int start_cx = tiles.x/c_tiles_per_chunk,
	    end_cx = (tiles.x + tiles.w - 1)/c_tiles_per_chunk;
	int start_cy = tiles.y/c_tiles_per_chunk,
	    end_cy = (tiles.y + tiles.h - 1)/c_tiles_per_chunk;
					// Go through all covered chunks.
	for (int cy = start_cy; cy <= end_cy; cy++)
		for (int cx = start_cx; cx <= end_cx; cx++)
			{		// Go through objects.
			Map_chunk *chunk = gmap->get_chunk(cx, cy);
			Object_iterator next(chunk->get_objects());
			Game_object *obj;
			while ((obj = next.get_next()) != 0)
				{	// Check shape.
				if (shapenum >= 0)
					{
					if (obj->get_shapenum() != shapenum)
						continue;
					}
				if (qual != c_any_qual && obj->get_quality() 
								!= qual)
					continue;
				if (framenum !=  c_any_framenum &&
					obj->get_framenum() != framenum)
					continue;
				if (!Check_mask(gwin, obj, mask))
					continue;
				Tile_coord t = obj->get_tile();
				if (tiles.has_point(t.tx, t.ty)) {
					FN_OBJECT* castobj = obj FN_CAST;
					if (castobj) vec.push_back(castobj);
				}
				}
			}
					// Return # added.
	return (vec.size() - vecsize);
	}

#undef FN_VECTOR
#undef FN_OBJECT
#undef FN_CAST

#endif
