#ifndef SDL_COMPAT_H
#define SDL_COMPAT_H

#include "SDL_version.h"

#if SDL_VERSION_ATLEAST(2, 0, 0)
#define SDL_COMPAT_DISPLAY_INDEX 0 // TODO: Add multiple screen support...
#define SDL2_INITIAL_FILL 0, 0, 0, 255 // Make it black
#ifdef MACOSX
#define SDL_FULLSCREEN SDL_WINDOW_FULLSCREEN_DESKTOP // or other monitors go black on fullscreen
#else
#define SDL_FULLSCREEN SDL_WINDOW_FULLSCREEN
#endif
#define SDL_HWPALETTE 0 // Doesn't exist any longer and there's no alternative
#define SDL_VideoModeOK(a,b,c,d) VideoModeOK(a,b,c,d)
#define SDL_UpdateRect(a,b,c,d,e) UpdateRect(a,b,c,d,e) // Should only be called from Image_window
#define SDL_SETCOLORS(a,b,c,d) SDL_SetPaletteColors(a->format->palette,b,c,d)
#define SDL_BYTEORDERH "SDL_endian.h"
#define SDL_EnableUNICODE(a) do {} while(0) // Doesn't need to be ran anymore with SDL 2
#define SDL_SETEVENTFILTER(a) SDL_SetEventFilter(a,0);
#define SDLMod SDL_Keymod
#define SDL_keysym SDL_Keysym
#define SDLKey SDL_Keycode
// Keys that have changed a bit
#define SDLK_KP0 SDLK_KP_0
#define SDLK_KP1 SDLK_KP_1
#define SDLK_KP9 SDLK_KP_9
#define SDLK_NUMLOCK SDLK_NUMLOCKCLEAR
#define SDLK_SCROLLOCK SDLK_SCROLLLOCK
#define KMOD_META KMOD_GUI
// End of keys that have changed a bit
// Events that have changed a bit
#define SDL_ACTIVEEVENT SDL_WINDOWEVENT
//#define SDL_APPMOUSEFOCUS SDL_WINDOWEVENT_ENTER
//#define SDL_APPINPUTFOCUS SDL_WINDOWEVENT_FOCUS_GAINED
// End of events that have changed a bit

#else
#define SDL_SETCOLORS SDL_SetColors
#define SDL_BYTEORDERH "SDL_byteorder.h"
#define SDL_SETEVENTFILTER SDL_SetEventFilter
#endif

#endif
