/*   SDLMain.m - main entry point for our Cocoa-ized SDL app
       Darrell Walisser - dwaliss1@purdue.edu

    Feel free to customize this file to suit your needs
*/

#import "SDL.h"
#import "SDLMain.h"
#import <sys/param.h> /* for MAXPATHLEN */
#import <unistd.h>

static int    gArgc;
static char  **gArgv;
static NSString *gAppName = 0;

@interface NSString (ReplaceSubString)
- (NSString *)stringByReplacingRange:(NSRange)aRange with:(NSString *)aString;
@end


/* The main class of the application, the application's delegate */
@implementation SDLMain

/* Invoked from the Quit menu item */
- (void) quit:(id)sender
{
    SDL_Event event;
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
}

/* Set the working directory to the .app's parent directory */
- (void) setupWorkingDirectory
{
    char parentdir[MAXPATHLEN];
    char *c;
    
    strncpy ( parentdir, gArgv[0], sizeof(parentdir) );
    c = (char*) parentdir;
    
    while (*c != '\0')     /* go to end */
        c++;
    
    while (*c != '/')      /* back up to parent */
        c--;
    
    *c++ = '\0';             /* cut off last part (binary name) */
    
    assert ( chdir (parentdir) == 0 );   /* chdir to the binary app's parent */
    assert ( chdir ("../../../") == 0 ); /* chdir to the .app's parent */
    
    gAppName = [ NSString stringWithCString: c ];
}

/* Fix menu to contain the real app name instead of "SDL App" */
- (void) fixMenu:(NSMenu *)aMenu
{
    NSRange aRange;
    NSEnumerator *enumerator;
    NSMenuItem *menuItem;

    aRange = [[aMenu title] rangeOfString:@"SDL App"];
    if (aRange.length != 0)
        [aMenu setTitle: [[aMenu title] stringByReplacingRange:aRange with:gAppName]];

    enumerator = [[aMenu itemArray] objectEnumerator];
    while ((menuItem = [enumerator nextObject]))
    {
        aRange = [[menuItem title] rangeOfString:@"SDL App"];
        if (aRange.length != 0)
            [menuItem setTitle: [[menuItem title] stringByReplacingRange:aRange with:gAppName]];
        if ([menuItem hasSubmenu])
            [self fixMenu: [menuItem submenu]];
    }
    [ aMenu sizeToFit ];
}

/* Called when the internal event loop has just started running */
- (void) applicationDidFinishLaunching: (NSNotification *) note
{
    int status;

    /* Set the working directory to the .app's parent directory */
    [ self setupWorkingDirectory ];

    /* Set the main menu to contain the real app name instead of "SDL App" */
    [ self fixMenu: [ NSApp mainMenu ] ];

    /* Hand off to main application code */
    status = SDL_main (gArgc, gArgv);

    /* We're done, thank you for playing */
    exit(status);
}
@end


@implementation NSString (ReplaceSubString)

- (NSString *)stringByReplacingRange:(NSRange)aRange with:(NSString *)aString
{
    unsigned int bufferSize;
    unsigned int selfLen = [self length];
    unsigned int aStringLen = [aString length];
    unichar *buffer;
    NSRange localRange;
    NSString *result;

    bufferSize = selfLen + aStringLen - aRange.length;
    buffer = NSAllocateMemoryPages(bufferSize*sizeof(unichar));
    
    // Get first part into buffer
    localRange.location = 0;
    localRange.length = aRange.location;
    [self getCharacters:buffer range:localRange];
    
    // Get middle part into buffer
    localRange.location = 0;
    localRange.length = aStringLen;
    [aString getCharacters:(buffer+aRange.location) range:localRange];
     
    // Get last part into buffer
    localRange.location = aRange.location + aRange.length;
    localRange.length = selfLen - localRange.location;
    [self getCharacters:(buffer+aRange.location+aStringLen) range:localRange];
    
    // Build output string
    result = [NSString stringWithCharacters:buffer length:bufferSize];
    
    NSDeallocateMemoryPages(buffer, bufferSize);
    
    return result;
}

@end


#ifdef main
#  undef main
#endif

/* Main entry point to executible - should *not* be SDL_main! */
int main (int argc, char **argv) {

    /* Copy the arguments into a global variable */
    int i;
    
    /* This is passed if we are launched by double-clicking */
    if ( argc >= 2 && strncmp (argv[1], "-psn", 4) == 0 ) {
        gArgc = 1;
    } else {
        gArgc = argc;
    }
    gArgv = (char**) malloc (sizeof(*gArgv) * (gArgc+1));
    assert (gArgv != NULL);
    for (i = 0; i < gArgc; i++) {
        gArgv[i] = argv[i];
    }
    gArgv[i] = NULL;

    NSApplicationMain (argc, argv);
    return 0;
}
