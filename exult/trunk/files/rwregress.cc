
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <iostream>
#include <strstream>
#include <cassert>
#include "utils.h"

using std::cout;
using std::endl;
using std::istrstream;

const char *ss_data = "A BC DE FGHI JKLM NO\0";

#if 0 // just used temporarially for some optimisation tests.
inline uint8 Read1diff
	(
	std::istream& in
	)
	{
	return static_cast<uint8>(in.get());
	}

inline uint32 Read4diff
	(
	std::istream& in
	)
	{
	return static_cast<uint32>(in.get() | (in.get()<<8) | (in.get()<<16) | (in.get()<<24));
	}

#include <sys/time.h>
#include <fstream>

void speedtest()
{
	timeval tvstart;
	timeval tvend;
	timeval tvstart_diff;
	timeval tvend_diff;
	
	std::ifstream file("random");
	assert(!file.fail());
	
	gettimeofday(&tvstart, NULL);
	
	while(!file.eof())
		uint8 ui8 = Read4(file);
	
	gettimeofday(&tvend, NULL);
	
	//-------------------------
	
	std::ifstream file_diff("random");
	assert(!file_diff.fail());
	
	gettimeofday(&tvstart_diff, NULL);
	
	while(!file_diff.eof())
		uint8 ui8 = Read4diff(file_diff);
	
	gettimeofday(&tvend_diff, NULL);
	
	cout << tvstart.tv_sec << '\t' << tvstart.tv_usec << endl;
	cout << tvend.tv_sec << '\t' << tvend.tv_usec << endl;
	cout << "---------------------" << endl;
	cout << tvend.tv_sec - tvstart.tv_sec << '\t' << tvend.tv_usec - tvstart.tv_usec << endl;
	cout << endl;
	cout << tvstart_diff.tv_sec << '\t' << tvstart_diff.tv_usec << endl;
	cout << tvend_diff.tv_sec << '\t' << tvend_diff.tv_usec << endl;
	cout << "---------------------" << endl;
	cout << tvend_diff.tv_sec - tvstart_diff.tv_sec << '\t' << tvend_diff.tv_usec - tvstart_diff.tv_usec << endl;
	cout << endl;
}
#endif

int main(int argc, char *argv[])
{
	istrstream iss(ss_data);
	
	uint8 outread1 = Read1(iss);
	cout << static_cast<char>(outread1) << endl;
	assert(static_cast<char>(outread1)=='A');
	
	assert(static_cast<char>(Read1(iss))==' ');
	
	uint16 outread2 = Read2(iss);
	cout << static_cast<char>(outread2 & 0xff) << static_cast<char>((outread2>>8) & 0xff) << endl;
	assert(static_cast<char>(outread2 & 0xff)=='B');
	assert(static_cast<char>((outread2>>8) & 0xff)=='C');
	
	assert(static_cast<char>(Read1(iss))==' ');
	
	uint16 outread2high = Read2high(iss);
	cout << static_cast<char>((outread2high>>8) & 0xff) << static_cast<char>(outread2high & 0xff) << endl;
	assert(static_cast<char>(outread2high & 0xff)=='E');
	assert(static_cast<char>((outread2high>>8) & 0xff)=='D');
	
	assert(static_cast<char>(Read1(iss))==' ');
	
	uint32 outread4 = Read4(iss);
	cout << static_cast<char>(outread4 & 0xff) << static_cast<char>((outread4>>8) & 0xff)
		 << static_cast<char>((outread4>>16) & 0xff)  << static_cast<char>((outread4>>24) & 0xff) << endl;
	assert(static_cast<char>(outread4 & 0xff)=='F');
	assert(static_cast<char>((outread4>>8) & 0xff)=='G');
	assert(static_cast<char>((outread4>>16) & 0xff)=='H');
	assert(static_cast<char>((outread4>>24) & 0xff)=='I');
	
	assert(static_cast<char>(Read1(iss))==' ');
	
	uint32 outread4high = Read4high(iss);
	cout << static_cast<char>((outread4high>>24) & 0xff) << static_cast<char>((outread4high>>16) & 0xff)
		 << static_cast<char>((outread4high>>8) & 0xff)  << static_cast<char>(outread4high & 0xff) << endl;
	assert(static_cast<char>(outread4high & 0xff)=='M');
	assert(static_cast<char>((outread4high>>8) & 0xff)=='L');
	assert(static_cast<char>((outread4high>>16) & 0xff)=='K');
	assert(static_cast<char>((outread4high>>24) & 0xff)=='J');
	
	//speedtest();
	
	return 0;
}







