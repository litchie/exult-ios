/**
 **	Msgfile.h - Read in text message file.
 **
 **	Written: 6/25/03
 **/

#ifndef INCL_MSGFILE_H
#define INCL_MSGFILE_H 1

using std::istream;
using std::vector;

int Read_text_msg_file
	(
	istream& in,
	vector<char *>& strings		// Strings returned here, each
					//   allocated on heap.
	);
#endif
