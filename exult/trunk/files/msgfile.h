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
class DataSource;

int Read_text_msg_file
	(
	DataSource* in,
	vector<char *>& strings,	// Strings returned here, each
					//   allocated on heap.
	const char *section = 0
	);
int Read_text_msg_file
	(
	istream& in,
	vector<char *>& strings,	// Strings returned here, each
					//   allocated on heap.
	const char *section = 0
	);
int Read_text_msg_file
	(
	istream& in,
	char **& strings,		// Strings returned here, each
					//   allocated on heap.
	int& count,
	const char *section = 0
	);
bool Search_text_msg_section
	(
	DataSource* in,
	const char *section = 0
	);
int Read_text_msg_file_sections
	(
	DataSource* in,
	vector<vector<char *> >& strings,	// Strings returned here
	const char *sections[],			// Section names
	int numsections
	);
int Read_text_msg_file_sections
	(
	istream& in,
	vector<vector<char *> >& strings,	// Strings returned here
	const char *sections[],			// Section names
	int numsections
	);
void Write_msg_file_section
	(
	ostream& out, 
	const char *section, 
	char **items, 
	int num_items
	);

#endif
