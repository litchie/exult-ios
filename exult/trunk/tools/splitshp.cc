/*
Copyright (C) 2001  The Exult Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <iostream>

#ifndef ALPHA_LINUX_CXX
#  include <cstdio>
#  include <cstring>
#  include <string>
#endif

#include "exult_types.h"


using std::cout;
using std::endl;
using std::cerr;
using std::string;
using std::strlen;
using std::strrchr;
using std::strcpy;
using std::memcpy;

struct u7frame {
	uint32 datalen;
	uint8 *data;
};

struct u7shape {
	int num_frames;
	struct u7frame *frames;
};

unsigned int read1(FILE *f)
{
	unsigned char b0;
	b0 = fgetc(f);
	return b0;
}

unsigned int read2(FILE *f)
{
	unsigned char b0, b1;
	b0 = fgetc(f);
	b1 = fgetc(f);
	return (b0 + (b1<<8));
}

signed int read2signed(FILE *f)
{
	unsigned char b0, b1;
	signed int i0;
	b0 = fgetc(f);
	b1 = fgetc(f);
	i0 = b0 + (b1<<8);
	if (i0 >= 32768) { i0 -= 65536; }
	return (i0);
}

unsigned int read4(FILE *f)
{
	unsigned char b0, b1, b2, b3;
	b0 = fgetc(f);
	b1 = fgetc(f);
	b2 = fgetc(f);
	b3 = fgetc(f);
	return (b0 + (b1<<8) + (b2<<16) + (b3<<24));
}

void write4(FILE *f, unsigned int b)
{
	fputc(b & 0xFF, f);
	fputc((b >> 8) & 0xFF, f);
	fputc((b >> 16) & 0xFF, f);
	fputc((b >> 24) & 0xFF, f);
}

char* framefilename(char * shapefilename, int frame)
{
	char *fn = new char[strlen(shapefilename)+4];  //_xx\0
	char *dot = strrchr(shapefilename, '.');
#ifdef WIN32
	char *slash = strrchr(shapefilename, '\\');
#else
	char *slash = strrchr(shapefilename, '/');
#endif
	int dotpos;

	if (dot == 0 || slash > dot)
		dotpos = strlen(shapefilename);
	else
		dotpos = dot - shapefilename;

	memcpy(fn, shapefilename, dotpos);
	sprintf(fn + dotpos, "_%02i", frame);
	strcpy(fn + dotpos + 3, shapefilename + dotpos);

	return fn;
}

void split_shape(char* filename)
{
	FILE *shpfile, *framefile;
	int file_size, shape_size, hdr_size;
	int frame_offset, next_frame_offset;
	int num_frames;
	int datalen;
	uint8 *data;
	char *framename;
	int i;
	
	shpfile = fopen (filename, "rb");
	if (!shpfile) {
		cerr << "Can't open " << filename << endl;
		return;
	}
	fseek(shpfile, 0, SEEK_END);
	file_size = ftell(shpfile);
	fseek(shpfile, 0, SEEK_SET);
	
	shape_size = read4(shpfile);
	
	if(file_size!=shape_size) {	/* 8x8 tile */
		num_frames = file_size/64;
		fseek(shpfile, 0, SEEK_SET);		/* Return to start of file */
		cout << "num_frames = " << num_frames << endl;
		data = new uint8[64];
		for(i=0; i<num_frames; i++) {
			framename = framefilename(filename, i);
			cout << "writing " << framename << "..." << endl;
			framefile = fopen(framename, "wb");
			fread(data, 1, 64, shpfile);
			fwrite(data, 1, 64, framefile);
			fclose(framefile);
			delete[] framename;
		}
		delete[] data;
	} else {
		hdr_size = read4(shpfile);
		num_frames = (hdr_size-4)/4;
		
		cout << "num_frames = "<< num_frames << endl;
		
		for(i=0; i<num_frames; i++) {
			framename = framefilename(filename, i);
			cout << "writing " << framename << "..." << endl;
			framefile = fopen(framename, "wb");

			// Go to where frame offset is stored
			fseek(shpfile, (i+1)*4, SEEK_SET);
			frame_offset = read4(shpfile);

			if (i+1 < num_frames)
				next_frame_offset = read4(shpfile);
			else
				next_frame_offset = shape_size;

			fseek(shpfile, frame_offset, SEEK_SET);
			datalen = next_frame_offset - frame_offset;

			write4(framefile, datalen+8);
			write4(framefile, 8);

			data = new uint8[datalen];
			fread(data, 1, datalen, shpfile);
			fwrite(data, 1, datalen, framefile);
			fclose(framefile);
			delete[] framename;
			delete[] data;
		}
	}
	cout << "done" << endl;

	fclose(shpfile);
}

void merge_frames(char *shapefile, char** framefiles, int numframefiles)
{
#if 0
	u7shape shape;

	shape->numframes = numframefiles;
	shape->frames = new u7frame[numframefiles];
#endif

	// finish this later...

}

int main(int argc, char *argv[])
{
	char* shapefile;
	int numframefiles;
	char* framefiles[255];
	u7shape* sh;
	
	if (argc < 2) {
	  cout << "Usage: To split: splitshp [shape file]" << endl
           << "     or to pack: splitshp [shape file] [frame files]" << endl;
	  return 0;
	}
	
	shapefile = argv[1];

	if (argc > 2) {
		numframefiles = argc - 2;
		for (int i=0; i<numframefiles; i++)
			framefiles[i] = argv[i+2];
		merge_frames(shapefile, framefiles, numframefiles);
	} else {
		split_shape(shapefile);
	}

	return 0;
}
