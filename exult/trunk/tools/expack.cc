#include "U7file.h"
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include "utils.h"

int main(int argc, char **argv)
{
  enum Arch_mode { NONE, LIST, EXTRACT, CAT, ADD } mode;
  char *fname;
  int index;
  
  mode = NONE;
  fname = 0;
  
  if(argc>2) {
    fname = argv[2];
    if((argv[1][0]=='-')&&(strlen(argv[1])==2)) {
      switch(argv[1][1]) {
      case 'l':
	mode = LIST;
	break;
      case 'x':
	mode = EXTRACT;
	break;
      case 'c':
	mode = CAT;
	break;
      case 'a':
	mode = ADD;
	break;
      default:
	mode = NONE;
	break;
      }
    }
  }
  
  switch(mode) {
  case LIST:
    {
      if(argc!=3)
	break;
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
    }
    break;
  case EXTRACT:
    {
      if(argc==4) {
	U7object f(fname,atoi(argv[3]));
	char outfile[32];
	sprintf(outfile,"%d.u7o",atoi(argv[3]));
	if(!f.retrieve(outfile))
	  cout << "Failed to get object" << endl;
      } else {
	U7FileManager fm;
	U7file *f = fm.get_file_object(fname);
	int count = f->number_of_objects(0);
	for(index=0; index<count; index++) {
	  U7object o(fname,index);
	  char outfile[32];
	  sprintf(outfile,"%d.u7o",index);
	  o.retrieve(outfile);
	}
	delete f;
      }
    }
    break;
  default:
    cout << "Usage:\n " << argv[0] << " -[l|x|c] file [index]" << endl;
    break;
  }
  return 0;
}

