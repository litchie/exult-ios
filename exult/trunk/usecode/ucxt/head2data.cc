
#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include <iostream>
#include <string>
#include <iomanip>
#include <vector>
#include <fstream>
#include "files/utils.h"

using std::cout;
using std::cerr;
using std::endl;
using std::ostream;
using std::string;
using std::setfill;
using std::setbase;
using std::vector;
using std::setw;
using std::ios;
using std::ofstream;

#ifndef __STRING
	#if defined __STDC__ && __STDC__
		#define __STRING(x) #x
	#else
		#define __STRING(x) "x"
	#endif
#endif

void bg_out(const string &fname)
{
	ofstream o;
	U7open(o, fname.c_str());
	
	if(o.fail())
	{
		cerr << "error: could not open `" << fname << "` for writing" << endl;
		exit(1);
	}
	
	o << setfill('0') << setbase(16);
	o.setf(ios::uppercase);
	
	#define	USECODE_INTRINSIC_PTR(NAME)	std::string(__STRING(NAME))
	std::string bgut[] =
	{
	#include "bgintrinsics.h"
	};
	#undef USECODE_INTRINSIC_PTR
	
	o << "<intrinsics>" << endl;
	for(unsigned int i=0; i<0x100; i++)
		o << "\t<0x" << setw(2) << i << "> " << bgut[i] << " </>" << endl;
	o << "</>" << endl;
	
	o.close();
}

void si_out(const string &fname)
{
	ofstream o;
	U7open(o, fname.c_str());
	
	if(o.fail())
	{
		cerr << "error: could not open `" << fname << "` for writing" << endl;
		exit(1);
	}
	
	o << setfill('0') << setbase(16);
	o.setf(ios::uppercase);
	
	#define	USECODE_INTRINSIC_PTR(NAME)	std::string(__STRING(NAME))
	std::string siut[] =
	{
	#include "siintrinsics.h"
	};
	#undef USECODE_INTRINSIC_PTR
	
	o << "<intrinsics>" << endl;
	for(unsigned int i=0; i<0x100; i++)
		o << "\t<0x" << setw(2) << i << "> " << siut[i] << " </>" << endl;
	o << "</>" << endl;
	
	o.close();
}

int main(int argc, char **argv)
{
	if(argc!=3)
	{
		cout << "usage:" << endl
			<< "\thead2data <bg outputfile> <si outputfile>" << endl
			<< endl
			<< "\tWhere the output files are the relative pathnames to the datafiles" << endl
			<< "\tto be output." << endl
			<< "\teg. head2data data/u7bgintrinsics.data data/u7siintrinsics.data" << endl;
		return 1;
	}
	
	bg_out(string(argv[1]));
	si_out(string(argv[2]));
	
	return 0;
}

