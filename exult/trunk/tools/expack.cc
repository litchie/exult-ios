#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef ALPHA_LINUX_CXX
#  include <unistd.h>
#  include <fstream>
#  include <cstdio>
#  include <cstdlib>
#endif
#include <iostream>
#include <vector>
#include <string>
#include "U7file.h"
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
using std::strcpy;
using std::strlen;
using std::vector;
using std::string;

enum Arch_mode { NONE, LIST, EXTRACT, CREATE, ADD, RESPONSE };

void set_mode(Arch_mode &mode, Arch_mode new_mode)
{
	if(mode!=NONE) {
		cerr << "Error: cannot specify multiple modes" << endl;
		exit(1);
	} else
		mode = new_mode;
}

long get_file_size(const char *fname)
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
	vector<string>	file_names;
  
	if(argc>2) {
		fname = argv[2];
		if((argv[1][0]=='-')&&(strlen(argv[1])==2)) {
			switch(argv[1][1]) {
			case 'i':
				{
					char temp[1024];
					ifstream respfile;

					set_mode(mode,RESPONSE);
					U7open(respfile, fname);
					// Read the output file name
					fname = new char[1024];
					respfile.getline(fname, 1024);
					while(!respfile.eof()) {
						respfile.getline(temp, 1024);
						if(strlen(temp)>0)
							file_names.push_back(temp);
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
				for(int i=0;i<argc-3;i++) {
					file_names.push_back(argv[i+3]);
				}
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
			if(file_names.size()<1) {
				cerr << "No files specified" << endl;
				exit(1);
			}
				
			ofstream flex;
			U7open(flex, fname);
			StreamDataSource fs(&flex);
			
			vector<int>	file_sizes;
			for(vector<string>::const_iterator X = file_names.begin(); X != file_names.end(); ++X)
				file_sizes.push_back(get_file_size(X->c_str()));
			
			// The FLEX title
			char title[0x50];
			snprintf(title,0x50,"Exult Archive");
			fs.write(title, 0x50);
			// The FLEX magic
			fs.write4(0xFFFF1A00);
			// The archive size
			fs.write4(file_names.size());
			// More FLEX magic :-)
			fs.write4(0xCC);
			// Some blank stuff ???
			for(int i=0; i<9; i++)
				fs.write4(0x0);
			// The reference table
			int data_start = 128+8*file_names.size();
			for(vector<int>::const_iterator X = file_sizes.begin(); X != file_sizes.end(); ++X) {
				if(*X) {
					fs.write4(data_start);
					fs.write4(*X);
					data_start += *X;
				} else {
					fs.write4(0x0);
					fs.write4(0x0);
				}
			}
			// The files
			for(int i=0; i<file_names.size(); i++) {
				if(file_sizes[i]) {
					ifstream infile;
					U7open(infile, file_names[i].c_str());
					StreamDataSource ifs(&infile);
					char *buf = new char[file_sizes[i]];
					ifs.read(buf, file_sizes[i]);
					fs.write(buf, file_sizes[i]);
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

