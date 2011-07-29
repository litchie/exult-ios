#ifndef SDL_COMPAT_H
#define SDL_COMPAT_H

#ifdef SDL_VER_1_3
#include <SDL_Compat.h>
#define SDL_SETCOLORS(a,b,c,d) SDL_SetPaletteColors(a->format->palette,b,c,d)
#define SDL_BYTEORDERH "SDL_endian.h"
#define SDL_SETEVENTFILTER(a) SDL_SetEventFilter(a,0);
#else
#define SDL_SETCOLORS SDL_SetColors
#define SDL_BYTEORDERH "SDL_byteorder.h"
#define SDL_SETEVENTFILTER SDL_SetEventFilter
#endif

#endif
