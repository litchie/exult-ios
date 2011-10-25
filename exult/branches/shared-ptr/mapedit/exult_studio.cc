#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef WIN32
#include "utils.h"
#endif

#include "studio.h"

int main(int argc, char **argv)
{
	ExultStudio studio(argc, argv);
	studio.run();

#ifdef WIN32
	cleanup_output("studio_");
#endif

}
