/*
 *  Copyright (C) 2000-2001  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef ALPHA_LINUX_CXX
#  include <unistd.h>
#  include <fstream>
#  include <cstdio>
#  include <cstdlib>
#  include <cstring>
#endif
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include "U7file.h"
#include "Flex.h"
#include "utils.h"
#include "databuf.h"
#include "crc.h"
#include "exceptions.h"

using std::atoi;
using std::cerr;
using std::cout;
using std::endl;
using std::exit;
using std::FILE;
using std::ifstream;
using std::ofstream;
using std::size_t;
using std::strrchr;
using std::strncpy;
using std::strncat;
using std::strlen;
using std::vector;
using std::string;

enum Arch_mode { NONE, LIST, EXTRACT, CREATE, ADD, RESPONSE };

bool is_text_file(const char *fname)
{
	int len = strlen(fname);

	// only if the filename is greater than 4 chars
	if (len > 4 && fname[len-4] == '.' &&
		(fname[len-3] == 't' || fname[len-3] == 'T') &&
		(fname[len-2] == 'x' || fname[len-2] == 'X') &&
		(fname[len-1] == 't' || fname[len-1] == 'T'))
	{
		return true;
	}

	return false;
}

bool is_null_entry(const char *fname)
{
	int len = strlen(fname);

	if (len >= 4 && fname[len-4] == 'N' && fname[len-3] == 'U' &&
					fname[len-2] == 'L' && fname[len-1] == 'L')
		return true;

	return false;

}

void set_mode(Arch_mode &mode, Arch_mode new_mode)
{
	if(mode!=NONE) {
		cerr << "Error: cannot specify multiple modes" << endl;
		exit(1);
	} else
		mode = new_mode;
}

// Converts all .'s to _'s
void make_header_name(char *filename)
{
	int i = strlen (filename);

	while (i--) if (filename[i] == '.') filename[i] = '_';
		else if (filename[i] == '/' || filename[i] == '\\' || filename[i] == ':') break;
}

// Makes a name uppercase
void make_uppercase (char *name)
{
	int i = strlen (name);

	while (i--) if (name[i] >= 'a' && name[i] <= 'z') name[i] -= 'a' - 'A';
}

// strips a path from a filename
void strip_path (char *filename)
{
	int i = strlen (filename);

	while (i--)
	{
		if (filename[i] == '\\' || filename[i] == '/' || filename[i] == ':')
			break;
	}

	// Has a path
	if (i >= 0)
	{
		int j = 0;

		for (i++, j = 0; filename[j+i]; j++)
			filename[j] = filename[j+i];

		filename[j] = 0;
	}
}

long get_file_size(const char *fname)
{
	if (is_null_entry(fname)) 
		return 0; // an empty entry

	const char *mode = "rb";
	bool text = is_text_file(fname);
	if (text)
		mode = "r";

	FILE *fp;
	try {
		fp = U7open (fname, mode);
	} catch (const file_open_exception& e) {
		cerr << e.what() << endl;
		exit(1);
	}

	long len = 0;
	if (!text)
	{
		fseek(fp, 0, SEEK_END);
		len = ftell(fp);
	}
	else while (fgetc(fp) != EOF)
		len++;

	fclose(fp);
	return len;
}


// This getline() accepts any kind of endline on any platform
// (So it will accept windows linefeeds in linux, and vice versa)
void getline(ifstream& file, char* buf, int size)
{
	int i = 0;
	char c;
	file.get(c);

	while (i < size-1 && (c>=' ' || c=='\t') && file.good()) {
		buf[i++] = c;
		file.get(c);
	}
	buf[i] = '\0'; // terminator

	while (!(file.peek()>=' ' || file.peek()=='\t') && file.good())
		file.get(c);
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

	char	*mac_argv2[mac_argc] =
		{
			"expack",
			"-i",
			"bg/flx.in"
		};
		
	mac_main( mac_argc, mac_argv2 );

	char	*mac_argv3[mac_argc] =
		{
			"expack",
			"-i",
			"si/flx.in"
		};
		
	mac_main( mac_argc, mac_argv3 );
}


int mac_main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
	Arch_mode mode = NONE;
	char fname[1024];
	char hname[1024];
	char hprefix[1024];
	char ext[] = "u7o";
	int index;
	vector<string>	file_names;
  	hname[0] = 0;

	if(argc>2) {
		strncpy(fname, argv[2], 1024);
		if((argv[1][0]=='-')&&(strlen(argv[1])==2)) {
			switch(argv[1][1]) {
			case 'i':
				{
					char temp[1024];
					char path_prefix[1024];
					
					ifstream respfile;
					char *slash = strrchr(fname, '/');
					if(slash) {
						int len = slash-fname+1;
						strncpy(path_prefix, fname, len);
						path_prefix[len] = 0;
					} else
						path_prefix[0] = 0;

					set_mode(mode,RESPONSE);
					try {
						U7open(respfile, fname, true);
					} catch (const file_open_exception& e) {
						cerr << e.what() << endl;
						exit(1);
					}
					
					// Read the output file name
					getline(respfile, temp, 1024);
					strncpy(fname, path_prefix, 1024);
					strncat(fname, temp, 1024);

					// Header file name
					strncpy (hprefix, temp, 1024);
					make_header_name(hprefix);
					strncpy (hname, path_prefix, 1024);
					strncat (hname, hprefix, 1024);
					strncat (hname, ".h", 1024);
					strip_path (hprefix);
					make_uppercase (hprefix);

					while(respfile.good()) {
						getline(respfile, temp, 1024);
						if(strlen(temp)>0) {
							char temp2[1024];
							strncpy(temp2, path_prefix,1024);
							strncat(temp2, temp, 1024);
							file_names.push_back(temp2);
						}
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
				{
				for(int i=0;i<argc-3;i++) {
					file_names.push_back(argv[i+3]);
				}
				set_mode(mode,CREATE);
				break;
				}
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
				int nobjs = f.number_of_objects();
				int n = atoi(argv[3]);
				if (n >= nobjs) {
					cerr << "Obj. #(" << n <<
						") is too large.  ";
					cerr << "Flex size is " << 
						nobjs << '.' << endl;
					exit(1);
				}
				char outfile[32];
				snprintf(outfile,32,"%d.%s", n, ext);
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
#if 0	/* Why not allow an empty flex? */
			if(file_names.size()<1) {
				cerr << "No files specified" << endl;
				exit(1);
			}
#endif
			ofstream flex;
			try {
				U7open(flex, fname);
			} catch (const file_open_exception& e) {
				cerr << e.what() << endl;
				exit(1);
			}

			char hline[1024];
			ofstream header;
			if (!hname[0]) {	// Need header name.
				strncpy (hprefix, fname, 1024);
				make_header_name(hprefix);
				strncat (hname, hprefix, 1024);
				strncat (hname, ".h", 1024);
			}		
			try {
				U7open(header, hname, true);
			} catch (const file_open_exception& e) {
				cerr << e.what() << endl;
				exit(1);
			}

			// The FLEX title
			Flex_writer writer(flex, "Exult Archive", file_names.size());

			// The beginning of the header
			header << "// Header for \"" << fname << "\" Created by expack" << std::endl << std::endl;
			header << "// DO NOT MODIFY" << std::endl << std::endl;
			header << "#ifndef " << hprefix << "_INCLUDED" << std::endl;
			header << "#define " << hprefix << "_INCLUDED" << std::endl << std::endl;

			// The files
				{
				for(int i=0; i<file_names.size(); i++) {
					int fsize = get_file_size(file_names[i].c_str());
					if(fsize) {
						ifstream infile;
						try {
							U7open(infile, file_names[i].c_str(), is_text_file(file_names[i].c_str()));
						} catch (const file_open_exception& e) {
							cerr << e.what() << endl;
							exit(1);
						}
						StreamDataSource ifs(&infile);
						char *buf = new char[fsize];
						ifs.read(buf, fsize);
						flex.write(buf, fsize);
						delete [] buf;
						infile.close();

						strncpy (hline, file_names[i].c_str(), 1024);
						strip_path(hline);
						make_header_name(hline);
						make_uppercase(hline);
						header << "#define\t" << hprefix << "_" << hline << "\t\t" << i << std::endl;
					}
				writer.mark_section_done();
				}
			}
			if (!writer.close())
				cerr << "Error writing " << fname << endl;

			uint32 crc32val = crc32_syspath(fname);
			header << std::endl << "#define\t" << hprefix << "_CRC32\t0x";
			header << std::hex << crc32val << std::dec << "U" << std::endl;

			header << std::endl << "#endif" << std::endl << std::endl;
			header.close();
			
		}
		break;
	default:
		cout << "Usage:" << endl
			 << argv[0] << " -[l|x|c] file [index]" << endl
			 << argv[0] << " -i indexfile" << endl;
		break;
	}
	return 0;
}

