#include "U7file.h"
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "utils.h"
#include <vector>

int main(int argc, char **argv)
{
	if(argc!=4) {
		cerr << "txt2cc [input.txt] [output.cc] [function]" << endl;
		exit(1);
	}
	
	U7object txtobj(argv[1], 0);
	size_t len;
		
	char *txt, *ptr, *end;
	txtobj.retrieve(&txt, len);
	ptr = txt;
	end = ptr+len;
	ofstream output;
	U7open(output, argv[2]);
	output << "#include <vector>" << endl << endl;
	output << "vector<char *> *" << argv[3] << "()" << endl;
	output << "{" << endl;
	output << "  vector<char *> *text = new vector<char *>();" << endl;
	while(ptr<end) {
		char *start = ptr;
		ptr = strchr(ptr, '\n');
		if(ptr) {
			if(*(ptr-1)=='\r') // It's CR/LF
				*(ptr-1) = 0;
			else
				*ptr = 0;
			output << "  text->push_back(\"";
			for(char *c=start; *c!=0; c++) {
				if(*c=='\\')
					output << "\\\\";
				else if(*c=='\"')
					output << "\\\"";
				else
					output << *c;
			}
			output << "\");" << endl;
			ptr += 1;
		} else
			break;
	}
	delete [] txt;
	output << "  return text;" << endl;
	output << "}" << endl;
	output.close();
}
