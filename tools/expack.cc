#include "U7file.h"
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "utils.h"
#include "databuf.h"

enum Arch_mode { NONE, LIST, EXTRACT, CREATE, ADD };

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
	const	mac_argc = 25;
	char	*mac_argv[mac_argc] =
		{
			"expack",
			"-c",
			"data/exult.flx",
			"data/exult_quotes.shp",
			"data/exult_credits.shp",
			"data/quotes.txt",
			"data/credits.txt",
			"data/exult_logo.shp",
			"data/exult0.pal",
			"data/black_gate.shp",
			"data/serpent_isle.shp",
			"data/meditown.mid",
			"data/font.shp",
			"data/setup.shp",
			"data/play_intro.shp",
			"data/full_screen.shp",
			"data/cheating.shp",
			"data/ok.shp",
			"data/cancel.shp",
			"data/pointers.shp",
			"data/exit.shp",
			"data/play_1st_scene.shp",
			"data/extras.shp",
			"data/midi_conversion.shp",
			"data/sfx_conversion.shp"
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
	char *ext;
	int index;
	
	ext = strdup("u7o");
  
	if(argc>2) {
		fname = argv[2];
		if((argv[1][0]=='-')&&(strlen(argv[1])==2)) {
			switch(argv[1][1]) {
			case 'l':
				set_mode(mode,LIST);
				break;
			case 'x':
				set_mode(mode,EXTRACT);
				break;
			case 'c':
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
			int count = f->number_of_objects(0);
			cout << "Archive: " << fname << endl;
			cout << "Type: " << f->get_archive_type() << endl;
			cout << "Size: " << count << endl;
			cout << "-------------------------" << endl;
			for(int i=0; i<count; i++) {
				char *buf;
				size_t len;
		
				f->retrieve(i, &buf, &len);
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
				sprintf(outfile,"%d.%s",atoi(argv[3]),ext);
				if(!f.retrieve(outfile))
					cerr << "Failed to get object" << endl;
			} else {
				U7FileManager fm;
				U7file *f = fm.get_file_object(fname);
				int count = f->number_of_objects(0);
				for(index=0; index<count; index++) {
					U7object o(fname,index);
					char outfile[32];
					sprintf(outfile,"%d.%s",index,ext);
					o.retrieve(outfile);
				}
				delete f;
			}
		}
		break;
	case CREATE:
		{
			if(argc<4) {
				cerr << "No files specified" << endl;
				exit(1);
			}
				
			ofstream flex;
			U7open(flex, fname);
			StreamDataSource fs(&flex);
			
			int count = argc-3;
			int *sizes = new int[count];
			for(int i=0; i<count; i++)
				sizes[i] = get_file_size(argv[i+3]);
			
			// The FLEX title
			char title[0x50];
			sprintf(title,"Exult Archive");
			fs.write(title, 0x50);
			// The FLEX magic
			fs.write4(0xFFFF1A00);
			// The archive size
			fs.write4(count);
			// More FLEX magic :-)
			fs.write4(0xCC);
			// Some blank stuff ???
			for(int i=0; i<9; i++)
				fs.write4(0x0);
			// The reference table
			int data_start = 128+8*count;
			for(int i=0; i<count; i++) {
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
			for(int i=0; i<count; i++) {
				if(sizes[i]) {
					ifstream infile;
					U7open(infile, argv[i+3]);
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

