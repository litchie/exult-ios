/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Utils.h - Common utility routines.
 **
 **	Written: 10/1/98 - JSF
 **/

/*
Copyright (C) 1998  Jeffrey S. Freedman

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

#ifndef INCL_UTILS
#define INCL_UTILS	1
#ifdef MACOS
  #include <iostream>
  // it is not sufficient to #include <iosfwd> here because Read1() etc.
  // call methods of class istream
  #include <stat.h>
#else
  #include <sys/stat.h>
  #include <iosfwd>
#endif
#include <exception>
#include <hash_map>
#include <string>
#ifdef MACOS
  using Metrowerks::hash_map;
#endif

/*
 *	Hash function for strings:
 */
struct hashstr
{
	long operator() (const char *str) const
	{
		static const unsigned long m = 4294967291u;
		unsigned long result = 0;
		for (; *str != '\0'; ++str)
			result = ((result << 8) + *str) % m;
		return long(result);
	}
};

/*
 *	For testing if two strings match:
 */
struct eqstr
{
	bool operator()(const char* s1, const char* s2) const {
		return std::strcmp(s1, s2) == 0;
	}
};

/*
 * For classes which should not be replicatable
 */


class replication_error : public std::exception
{
	std::string  what_;
public:
	replication_error (const char *what_arg): what_ (what_arg) {  }	
	replication_error (const std::string& what_arg): what_ (what_arg) {  }
	const char *what(void) { return what_.c_str(); }
};

#define	UNREPLICATABLE_CLASS(NAME)	NAME(const NAME &) { throw replication_error( #NAME " cannot be replicated"); }; \
					NAME &operator=(const NAME &) { throw replication_error( #NAME " cannot be replicated"); return *this; }

#define	UNREPLICATABLE_CLASS_I(NAME,INIT)	NAME(const NAME &) : INIT { throw replication_error( #NAME " cannot be replicated"); }; \
					NAME &operator=(const NAME &) { throw replication_error( #NAME " cannot be replicated"); return *this; }


/*
 *	Read a 1-byte value.
 */

inline unsigned char Read1
	(
	std::istream& in
	)
	{
	unsigned char c;
	in.get((char&) c);
	return (c);
	}

/*
 *	Read a 2-byte value, lsb first.
 */

inline unsigned int Read2
	(
	std::istream& in
	)
	{
	unsigned char b0, b1;
	in.get((char&) b0);
	in.get((char&) b1);
	return (b0 + (b1 << 8));
	}

/*
 *	Read a 2-byte value from a buffer.
 */

inline unsigned int Read2
	(
	unsigned char *& in
	)
	{
	unsigned char b0 = *in++;
	unsigned char b1 = *in++;
	return (b0 + (b1 << 8));
	}

#ifdef BUFSIZ	/* Kludgy, but I don't want to include stdio.h all the time.*/
/*
 *	Read a 2-byte value from a file.
 */

inline unsigned int Read2
	(
	std::FILE* in
	)
	{
	unsigned char b0, b1;
	fread(&b0,sizeof(unsigned char),1,in);
	fread(&b1,sizeof(unsigned char),1,in);
	return (b0 + (b1 << 8));
	}
#endif

/*
 *	Read a 2-byte value, hsb first.
 */

inline unsigned int Read2high
	(
	std::istream& in
	)
	{
	unsigned char b0, b1;
	in.get((char&) b0);
	in.get((char&) b1);
	return ((b0 << 8) + b1);
	}

/*
 *	Read a 2-byte value from a buffer.
 */

inline unsigned int Read2high
	(
	unsigned char *& in
	)
	{
	unsigned char b0 = *in++;
	unsigned char b1 = *in++;
	return ((b0 << 8) + b1);
	}

#ifdef BUFSIZ	/* Kludgy, but I don't want to include stdio.h all the time.*/
/*
 *	Read a 2-byte value from a file.
 */

inline unsigned int Read2high
	(
	std::FILE* in
	)
	{
	unsigned char b0, b1;
	fread(&b0,sizeof(unsigned char),1,in);
	fread(&b1,sizeof(unsigned char),1,in);
	return ((b0 << 8) + b1);
	}
#endif

/*
 *	Read a 4-byte long value, lsb first.
 */

inline unsigned long Read4
	(
	std::istream& in
	)
	{
	unsigned char b0, b1, b2, b3;
	in.get((char&) b0);
	in.get((char&) b1);
	in.get((char&) b2);
	in.get((char&) b3);
	return (b0 + (b1<<8) + (b2<<16) + (b3<<24));
	}

/*
 *	Read a 4-byte value from a buffer.
 */

inline unsigned long Read4
	(
	unsigned char *& in
	)
	{
	unsigned char b0 = *in++;
	unsigned char b1 = *in++;
	unsigned char b2 = *in++;
	unsigned char b3 = *in++;
	return (b0 + (b1<<8) + (b2<<16) + (b3<<24));
	}

#ifdef BUFSIZ	/* Kludgy, but I don't want to include stdio.h all the time.*/
/*
 *	Read a 4-byte value from a file.
 */

inline unsigned long Read4
	(
	std::FILE* in
	)
	{
	unsigned char b0, b1, b2, b3;
	fread(&b0,sizeof(unsigned char),1,in);
	fread(&b1,sizeof(unsigned char),1,in);
	fread(&b2,sizeof(unsigned char),1,in);
	fread(&b3,sizeof(unsigned char),1,in);
	return (b0 + (b1<<8) + (b2<<16) + (b3<<24));
	}
#endif
/*
 *	Read a 4-byte long value, hsb first.
 */

inline unsigned long Read4high
	(
	std::istream& in
	)
	{
	unsigned char b0, b1, b2, b3;
	in.get((char&) b0);
	in.get((char&) b1);
	in.get((char&) b2);
	in.get((char&) b3);
	return ((b0<<24) + (b1<<16) + (b2<<8) + b3);
	}

/*
 *	Read a 4-byte value from a buffer.
 */

inline unsigned long Read4high
	(
	unsigned char *& in
	)
	{
	unsigned char b0 = *in++;
	unsigned char b1 = *in++;
	unsigned char b2 = *in++;
	unsigned char b3 = *in++;
	return ((b0<<24) + (b1<<16) + (b2<<8) + b3);
	}

#ifdef BUFSIZ	/* Kludgy, but I don't want to include stdio.h all the time.*/
/*
 *	Read a 4-byte value from a file.
 */

inline unsigned long Read4high
	(
	std::FILE* in
	)
	{
	unsigned char b0, b1, b2, b3;
	fread(&b0,sizeof(unsigned char),1,in);
	fread(&b1,sizeof(unsigned char),1,in);
	fread(&b2,sizeof(unsigned char),1,in);
	fread(&b3,sizeof(unsigned char),1,in);
	return ((b0<<24) + (b1<<16) + (b2<<8) + b3);
	}
#endif

/*
 *	Write a 2-byte value, lsb first.
 */

inline void Write2
	(
	std::ostream& out,
	int val
	)
	{
	out.put((char) (val&0xff));
	out.put((char) ((val>>8)&0xff));
	}

/*
 *	Write a 2-byte value to a buffer, lsb first.
 */

inline void Write2
	(
	unsigned char *& out,		// Write here and update.
	int val
	)
	{
	*out++ = val & 0xff;
	*out++ = (val>>8) & 0xff;
	}

/*
 *	Write a 4-byte value, lsb first.
 */

inline void Write4
	(
	std::ostream& out,
	int val
	)
	{
	out.put((char) (val&0xff));
	out.put((char) ((val>>8)&0xff));
	out.put((char) ((val>>16)&0xff));
	out.put((char) ((val>>24)&0xff));
	}

/*
 *	Write a 4-byte value to a buffer, lsb first.
 */

inline void Write4
	(
	unsigned char *& out,		// Write here and update.
	int val
	)
	{
	*out++ = val & 0xff;
	*out++ = (val>>8) & 0xff;
	*out++ = (val>>16)&0xff;
	*out++ = (val>>24)&0xff;
	}

int U7open
	(
	std::ifstream& in,			// Input stream to open.
	const char *fname			// May be converted to upper-case.
	);
int U7open
	(
	std::ofstream& out,			// Output stream to open.
	const char *fname			// May be converted to upper-case.
	);

#ifdef BUFSIZ	/* Kludgy, but I don't want to include stdio.h all the time.*/
std::FILE* U7open
	(
	const char *fname,			// May be converted to upper-case.
	const char *mode			// File access mode.
	);
#endif
void U7remove(
	const char *fname
	);

int U7exists(
	const char *fname
	);

int Log2
	(
	unsigned int n
	);

void add_system_path(const char *key, const char *value);

#endif
