#ifndef SDL_COMPAT_H
#define SDL_COMPAT_H

#include "SDL_version.h"

#if SDL_VERSION_ATLEAST(2, 0, 0)
#define SDL_COMPAT_DISPLAY_INDEX 0 // TODO: Add multiple screen support...
#define SDL_FULLSCREEN SDL_WINDOW_FULLSCREEN
#define SDL_HWPALETTE 0 // Doesn't exist any longer and there's no alternative
// TODO: Make SDL_VideoModeOK do something worthwhile
#define SDL_VideoModeOK(a,b,c,d) 0
#define SDL_SETCOLORS(a,b,c,d) SDL_SetPaletteColors(a->format->palette,b,c,d)
#define SDL_BYTEORDERH "SDL_endian.h"
#define SDL_EnableUNICODE(a) 0 // Doesn't need to be ran anymore with SDL 2

#elif SDL_VERSION_ATLEAST(1, 3, 0)
#include <SDL_Compat.h>
#define SDL_SETCOLORS(a,b,c,d) SDL_SetPaletteColors(a->format->palette,b,c,d)
#define SDL_BYTEORDERH "SDL_endian.h"
#define SDL_SETEVENTFILTER(a) SDL_SetEventFilter(a,0);
#define SDL_GetKeyState(a) SDL_GetKeyboardState(a)

#else
#define SDL_SETCOLORS SDL_SetColors
#define SDL_BYTEORDERH "SDL_byteorder.h"
#define SDL_SETEVENTFILTER SDL_SetEventFilter
#endif

#endif
