#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "studio.h"

int main(int argc, char **argv)
{
	ExultStudio studio(argc, argv);
	studio.run();
}
