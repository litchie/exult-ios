

#include "U7file.h"
#include "Flex.h"
#include <unistd.h>
#include <iostream>




int	main(int argc, char **argv)
{
	if(argc==3) {
		U7object f(argv[1],atoi(argv[2]));

		if(f.retrieve("/tmp/blah"))
			cout << "Got object ok" << endl;
		else
			cout << "Failed to get object" << endl;
	} else {
		cout << "Usage:\n " << argv[0] << " file index" << endl;
	}
	return 0;
}

