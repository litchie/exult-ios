

#include "U7file.h"
#include "Flex.h"
#include <unistd.h>
#include <iostream>




int	main(void)
{
	chdir("/home/dancer2/projects/u7");
	U7object	u("static/endgame.dat",8);

	if(u.retrieve("/tmp/blah"))
		cout << "Got object ok" << endl;
	else
		cout << "Failed to get object" << endl;

	return 0;
}

