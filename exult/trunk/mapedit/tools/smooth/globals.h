#ifdef _MSC_VER
#define strcasecmp _stricmp
#define strncasecmp  _strnicmp
#endif

#ifdef WIN32
#include <windows.h>
typedef HMODULE libhandle_t;
#define PLUGIN_EXPORT __declspec(dllexport)
#else
typedef void *libhandle_t;
#define PLUGIN_EXPORT
#endif

#ifdef MAIN
#define EXTERN
#else
#define EXTERN extern
#endif

// this is totally arbitrary but seems reasonable. It is the max length of a line in the config file
#define MAX_LINE_LENGTH 1024

// since we use indexed images, we have a limitation of 256 colours
#define MAX_COLOURS 256

#include "SDL.h"

// note: there are some almost static stuff and some very variable stuff
// global's variables
typedef struct  g_var_struct {
	SDL_Surface *image_out; //var
	int global_x; //var
	int global_y; //var
} glob_variables;

EXTERN glob_variables g_variables;

// global's almost statics
typedef struct g_stat_struct {
	int debug; //stat
	SDL_Surface *image_in; //stat
	char *filein; //stat
	char *fileout; //stat
	char *config_file; //stat
} glob_statics;

EXTERN glob_statics g_statics;

typedef char colour_hex[8];

typedef void (*pfnPluginApply)(colour_hex ret_col, glob_variables *g_variables);

typedef struct pacman {
	struct pacman *next;
	pfnPluginApply plugin_apply; // for storing plugins' apply
	libhandle_t handle; // for storing dlopens' handle
} node;

// this is what holds the address of the plugin_apply functions to apply to colours.
// probably the most critical variable of the program
EXTERN node *action_table[MAX_COLOURS];


// this is to keep track of all loaded plugins. We need this to cleanly unload them later
EXTERN node *hdl_list;

