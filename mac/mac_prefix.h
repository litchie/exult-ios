// this tells the code we compile on the macintosh.
#define		MACOS	1


// no autoconf on macintosh, thus "emulate" it
#include	"mac_autoconfig.h"


// missing functions:
#define		isascii(x)	(!((x) & 0x80))
