/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Scale.h - Trying to scale with bilinear interpolation.
 **
 **	Written: 6/14/00 - JSF
 **/

#ifndef INCL_SCALE
#define INCL_SCALE 1

/*
 *	Scale X2 with bilinear interpolation.
 */
template <class Source_pixel, class Dest_pixel, class Manip_pixels>
void Scale2x
	(
	Source_pixel *source,		// ->source pixels.
	int swidth, int sheight,	// Source height.
	Dest_pixel *dest,		// ->dest pixels.
	const Manip_pixels& manip	// Manipulator methods.
	);

/*
 *	Manipulate from 8-bit to 16-bit pixels.
 */
class Manip8to16
	{
	SDL_Color *colors;		// Lookup table (palette).
	SDL_PixelFormat *fmt;		// Format of dest. pixels.
public:
	Manip8to16(SDL_PixelFormat *f)
		: fmt(f), colors(f->palette->colors)
		{  }
	unsigned short rgb(unsigned short r, unsigned short g,
							unsigned short b) const
		{
		return ((r>>fmt->Rloss)<<fmt->Rshift) |
		       ((g>>fmt->Gloss)<<fmt->Gshift) |
		       ((b>>fmt->Bloss)<<fmt->Bshift);
		}
	void copy(unsigned short& dest, unsigned char src) const
		{
		SDL_Color& color = colors[src];
		dest = rgb(color.r, color.g, color.b);
		}
	void split(unsigned char pix, unsigned short& r,
				unsigned short& g, unsigned short& b) const
		{
		SDL_Color& color = colors[pix];
		r = color.r;
		g = color.g;
		b = color.b;
		}
	};

#endif

