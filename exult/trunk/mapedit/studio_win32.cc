#include "studio.h"

extern "C" int main_win32(int argc, char **argv)
{
	ExultStudio studio(argc, argv);
	studio.run();
}