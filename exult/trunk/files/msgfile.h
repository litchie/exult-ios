/**
 **	Msgfile.h - Read in text message file.
 **
 **	Written: 6/25/03
 **/

#ifndef INCL_MSGFILE_H
#define INCL_MSGFILE_H 1

using std::istream;
using std::ostream;
using std::vector;

int Read_text_msg_file
	(
	istream& in,
	vector<char *>& strings,	// Strings returned here, each
					//   allocated on heap.
	char *section = 0
	);
int Read_text_msg_file
	(
	istream& in,
	char **& strings,		// Strings returned here, each
					//   allocated on heap.
	int& count,
	char *section = 0
	);
void Write_msg_file_section
	(
	ostream& out, 
	char *section, 
	char **items, 
	int num_items
	);

#endif
