#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#if defined(WIN32) && !defined(USE_CONSOLE)
#include <cstdio>
#define STDOUT_FILE	TEXT("studio_out.txt")
#define STDERR_FILE	TEXT("studio_err.txt")
#endif

#include "studio.h"

int main(int argc, char **argv)
{
// Code copied from SDL_main
#if defined(WIN32) && !defined(USE_CONSOLE)

	/* Flush the output in case anything is queued */
	fclose(stdout);
	fclose(stderr);

	/* Redirect standard input and standard output */
	FILE *newfp = freopen(STDOUT_FILE, "w", stdout);
	if ( newfp == NULL ) {	/* This happens on NT */
#if !defined(stdout)
		stdout = fopen(STDOUT_FILE, "w");
#else
		newfp = fopen(STDOUT_FILE, "w");
		if ( newfp ) {
			*stdout = *newfp;
		}
#endif
	}
	newfp = freopen(STDERR_FILE, "w", stderr);
	if ( newfp == NULL ) {	/* This happens on NT */
#if !defined(stderr)
		stderr = fopen(STDERR_FILE, "w");
#else
		newfp = fopen(STDERR_FILE, "w");
		if ( newfp ) {
			*stderr = *newfp;
		}
#endif
	}
	setvbuf(stdout, NULL, _IOLBF, BUFSIZ);	/* Line buffered */
	setbuf(stderr, NULL);			/* No buffering */

#endif //defined(WIN32) && !defined(USE_CONSOLE)

	ExultStudio studio(argc, argv);
	studio.run();

}
