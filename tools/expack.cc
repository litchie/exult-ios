#include "U7file.h"
#include "utils.h"
#include <unistd.h>
#include <iostream>

int main(int argc, char **argv)
{
  enum Arch_mode { LIST, EXTRACT } mode;
  char *fname;
  int index;
  
  mode = LIST;

  if(argc==2) {
    fname = argv[1];

    U7FileManager fm;
    U7file *f = fm.get_file_object(fname);
    int count = f->number_of_objects(0);
    cout << "Archive: " << fname << endl;
    cout << "Type: " << f->get_archive_type() << endl;
    cout << "Size: " << count << endl;
    cout << "-------------------------" << endl;
    for(int i=0; i<count; i++) {
	char *buf;
	size_t len;
	
	f->retrieve(i, &buf, &len);
	cout << (i+1) << "\t" << len << endl;
	delete [] buf;
    }

  } else if(argc==3) {
    fname = argv[1];
    index = atoi(argv[2]);
    U7object f(fname,atoi(argv[2]));
    if(f.retrieve("object.dat"))
      cout << "Got object ok" << endl;
    else
      cout << "Failed to get object" << endl;
  } else {
    cout << "Usage:\n " << argv[0] << " -[l|x] file [index]" << endl;
  }
  return 0;
}

