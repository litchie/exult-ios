#include "../alpha_kludges.h"

#ifndef ALPHA_LINUX_CXX
#  include <unistd.h>
#  include <fstream>
#  include <cstdio>
#  include <cstdlib>
#endif
#include "U7file.h"
#include <iostream>
#include "utils.h"
#include "databuf.h"

using std::atoi;
using std::cerr;
using std::cout;
using std::endl;
using std::exit;
using std::FILE;
using std::ifstream;
using std::ofstream;
using std::size_t;
using std::snprintf;
using std::strlen;

enum Arch_mode { NONE, LIST, EXTRACT, CREATE, ADD, RESPONSE };

void set_mode(Arch_mode &mode, Arch_mode new_mode)
{
	if(mode!=NONE) {
		cerr << "Error: cannot specify multiple modes" << endl;
		exit(1);
	} else
		mode = new_mode;
}

long get_file_size(char *fname)
{
	FILE *fp = U7open (fname, "rb");
	if (!fp) {
		cerr << "Could not open file " << fname << endl;
		exit(1);
	}
	fseek(fp, 0, SEEK_END);
	long len = ftell(fp);
	fclose(fp);
	return len;
}

#ifdef MACOS
int mac_main(int argc, char **argv);

int main()
{
	const	int mac_argc = 3;
	char	*mac_argv[mac_argc] =
		{
			"expack",
			"-i",
			"flx.in"
		};
		
	mac_main( mac_argc, mac_argv );
}


int mac_main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
	Arch_mode mode = NONE;
	char *fname = 0;
	char ext[] = "u7o";
	int index;
	char **file_names;
	int  file_count;
	int  file_skip;
  
	if(argc>2) {
		fname = argv[2];
		if((argv[1][0]=='-')&&(strlen(argv[1])==2)) {
			switch(argv[1][1]) {
			case 'i':
				{
				set_mode(mode,RESPONSE);
				ifstream respfile;
				U7open(respfile, fname);
				char *temp = new char[1024];
				file_count = file_skip = 0;
				while(!respfile.eof()) {
					respfile.getline(temp, 1024);
					if(strlen(temp)>0)
						++file_count;
				}
				--file_count; // Skip the first line
				respfile.close();
				U7open(respfile, fname);
				fname = new char[1024];
				// Now read the output file name
				respfile.getline(fname, 1024);
				file_names = new char *[file_count];
				for(int i=0;i<file_count;i++) {
					file_names[i] = new char[1024];
					respfile.getline(file_names[i], 1024);
				}
				respfile.close();
				}
				break;
			case 'l':
				set_mode(mode,LIST);
				break;
			case 'x':
				set_mode(mode,EXTRACT);
				break;
			case 'c':
				file_count = argc-3;
				file_names = argv;
				file_skip = 3;
				set_mode(mode,CREATE);
				break;
			case 'a':
				set_mode(mode,ADD);
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
			int count = f->number_of_objects();
			cout << "Archive: " << fname << endl;
			cout << "Type: " << f->get_archive_type() << endl;
			cout << "Size: " << count << endl;
			cout << "-------------------------" << endl;
			for(int i=0; i<count; i++) {
				char *buf;
				size_t len;
		
				buf = f->retrieve(i, len);
				cout << i << "\t" << len << endl;
				delete [] buf;
			}
		}
    		break;
  	case EXTRACT:
		{
			if(argc==4) {
				U7object f(fname,atoi(argv[3]));
				char outfile[32];
				snprintf(outfile,32,"%d.%s",atoi(argv[3]),ext);
				f.retrieve(outfile);	// may throw!
			} else {
				U7FileManager fm;
				U7file *f = fm.get_file_object(fname);
				int count = f->number_of_objects();
				for(index=0; index<count; index++) {
					U7object o(fname,index);
					char outfile[32];
					snprintf(outfile,32,"%d.%s",index,ext);
					o.retrieve(outfile);
				}
				delete f;
			}
		}
		break;
	case RESPONSE:
	case CREATE:
		{
			if(file_count<1) {
				cerr << "No files specified" << endl;
				exit(1);
			}
				
			ofstream flex;
			U7open(flex, fname);
			StreamDataSource fs(&flex);
			
			int *sizes = new int[file_count];
			for(int i=0; i<file_count; i++)
				sizes[i] = get_file_size(file_names[i+file_skip]);
			
			// The FLEX title
			char title[0x50];
			snprintf(title,0x50,"Exult Archive");
			fs.write(title, 0x50);
			// The FLEX magic
			fs.write4(0xFFFF1A00);
			// The archive size
			fs.write4(file_count);
			// More FLEX magic :-)
			fs.write4(0xCC);
			// Some blank stuff ???
			for(int i=0; i<9; i++)
				fs.write4(0x0);
			// The reference table
			int data_start = 128+8*file_count;
			for(int i=0; i<file_count; i++) {
				if(sizes[i]) {
					fs.write4(data_start);
					fs.write4(sizes[i]);
					data_start += sizes[i];
				} else {
					fs.write4(0x0);
					fs.write4(0x0);
				}
			}
			// The files
			for(int i=0; i<file_count; i++) {
				if(sizes[i]) {
					ifstream infile;
					U7open(infile, file_names[i+file_skip]);
					StreamDataSource ifs(&infile);
					char *buf = new char[sizes[i]];
					ifs.read(buf, sizes[i]);
					fs.write(buf, sizes[i]);
					delete [] buf;
					infile.close();
				}
			}
			flex.close();
			
		}
		break;
	default:
		cout << "Usage:\n " << argv[0] << " -[l|x|c] file [index]" << endl;
		break;
	}
	return 0;
}

