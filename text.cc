/**
 **	Text.cc - Text handling (using FreeType).
 **
 **	Written: 11/20/98 - JSF
 **/

#include <string.h>
#include "text.h"

/*
 *	Statics:
 */
TT_Engine Font_face::engine;
unsigned char Font_face::initialized = 0;


/*
 *	Read in a glyph.
 */

Glyph::Glyph
	(
	Font_face *font,
	int index
	)
	{
					// Load the glyph.
	TT_Load_Glyph(font->instance, font->glyph, index, TTLOAD_DEFAULT);
	TT_Glyph_Metrics metrics;
	TT_Get_Glyph_Metrics(font->glyph, &metrics);
					// Get dimensions in pixels.
	int width = (metrics.bbox.xMax - metrics.bbox.xMin)/64;
	int height = (metrics.bbox.yMax - metrics.bbox.yMin)/64;
	bitmap.rows = height;		// Set up bitmap structure.
	bitmap.cols = (width + 7)/8;
	bitmap.width = width;
	bitmap.flow = TT_Flow_Down;
	bitmap.size = bitmap.cols * bitmap.rows;
	bitmap.bitmap = new char[bitmap.size];
	memset(bitmap.bitmap, 0, bitmap.size);
					// Render glyph.
	TT_Get_Glyph_Bitmap(font->glyph, &bitmap, -metrics.bbox.xMin, 
							-metrics.bbox.yMin);
					// Store a couple more things.
	bearingX = metrics.bearingX/64;
	bearingY = metrics.bearingY/64;
	advance = metrics.advance/64;
	}

/*
 *	Delete a glyph.
 */

Glyph::~Glyph
	(
	)
	{
	delete bitmap.bitmap;
	}

/*
 *	Create a font engine.
 */
Font_face::Font_face
	(
	char *fname,			// .ttf filename.
	int pts				// Points desired.
	)
	{
	if (!initialized)		// Init. engine if first time.
		{
		error = TT_Init_FreeType(&engine);
		if (error)
			return;
		initialized = 1;
		}
					// Open font file.
	if ((error = TT_Open_Face(engine, fname, &face)) != 0)
		return;
					// Get font properties.
	TT_Get_Face_Properties(face, &props);
	if ((error = TT_New_Instance(face, &instance)) != 0)
		return;
					// Create cache.
	glyphs = new Glyph *[props.num_Glyphs];
	memset((char *) glyphs, 0, props.num_Glyphs * sizeof(Glyph *));
					// For now, set res. to 96.
	if ((error = TT_Set_Instance_Resolutions(instance, 96, 96)) != 0)
		return;
					// Set to desired pts.
	if ((error = TT_Set_Instance_CharSize(instance, pts*64)) != 0)
		return;
	TT_Get_Instance_Metrics(instance, &imetrics);
	if ((error = TT_New_Glyph(face, &glyph)) != 0)
		return;
	int i;				// Go through char. mappings.
	for (i = 0; i < props.num_CharMaps; i++)
		{
		TT_UShort platform, encoding;
		TT_Get_CharMap_ID(face, i, &platform, &encoding);
#if 0
					// Let's use Windows encoding for now.
		if (platform == 3 && encoding == 1)
#endif	/* ++++++Just use the first one for now.	*/
			break;
		}
					// Error-out if not found.
	if ((error = TT_Get_CharMap(face, i, &charmap)) != 0)
		return;
	}

/*
 *	Done with a font.
 */

Font_face::~Font_face
	(
	)
	{
	TT_Close_Face(face);
					// Free all the glyph bitmaps.
	for (int i = 0; i < props.num_Glyphs; i++)
		delete glyphs[i];
	}
