#include "playfli.h"

int main(int argc, char **argv) 
{
	if(argc!=2) {
		cout << "Usage: " << argv[0] << " filename" << endl;
		exit(1);
	}
	playfli fli(argv[1]);
	fli.info();
	fli.play(0);
}
