/*
 *	utils.h - Common utility routines.
 *
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _UTILS_H_
#define _UTILS_H_

#include <iostream>
#include <string>
#ifndef ALPHA_LINUX_CXX
#  include <iosfwd>
#endif

#include "common_types.h"

#ifndef HAVE_SNPRINTF
extern int snprintf(char *, size_t, const char *, /*args*/ ...);
#endif


/*
 *	Read a 1-byte value.
 */

inline uint8 Read1 (std::istream &in)
{
	return static_cast<uint8>(in.get());
}

/*
 *	Read a 2-byte value, lsb first.
 */

inline uint16 Read2 (std::istream &in)
{
	uint16 val = 0;
	val |= static_cast<uint16>(in.get());
	val |= static_cast<uint16>(in.get()<<8);
	return val;
}

/*
 *	Read a 2-byte value from a buffer.
 */

inline uint16 Read2
	(
	uint8 *& in
	)
	{
	uint8 b0 = *in++;
	uint8 b1 = *in++;
	return (b0 | (b1 << 8));
	}

#ifdef BUFSIZ	/* Kludgy, but I don't want to include stdio.h all the time.*/
/*
 *	Read a 2-byte value from a file.
 */

inline uint16 Read2
	(
	std::FILE* in
	)
	{
	uint8 b0, b1;
	std::fread(&b0,sizeof(uint8),1,in);
	std::fread(&b1,sizeof(uint8),1,in);
	return (b0 | (b1 << 8));
	}
#endif

/*
 *	Read a 2-byte value, hsb first.
 */

inline uint16 Read2high (std::istream &in)
{
	uint16 val = 0;
	val |= static_cast<uint16>(in.get()<<8);
	val |= static_cast<uint16>(in.get());
	return val;
}

/*
 *	Read a 2-byte value from a buffer.
 */

inline uint16 Read2high
	(
	uint8 *& in
	)
	{
	uint8 b0 = *in++;
	uint8 b1 = *in++;
	return ((b0 << 8) | b1);
	}

#ifdef BUFSIZ	/* Kludgy, but I don't want to include stdio.h all the time.*/
/*
 *	Read a 2-byte value from a file.
 */

inline uint16 Read2high
	(
	std::FILE* in
	)
	{
	uint8 b0, b1;
	std::fread(&b0,sizeof(uint8),1,in);
	std::fread(&b1,sizeof(uint8),1,in);
	return ((b0 << 8) | b1);
	}
#endif

/*
 *	Read a 4-byte long value, lsb first.
 */

inline uint32 Read4 (std::istream &in)
{
	uint32 val = 0;
	val |= static_cast<uint32>(in.get());
	val |= static_cast<uint32>(in.get()<<8);
	val |= static_cast<uint32>(in.get()<<16);
	val |= static_cast<uint32>(in.get()<<24);
	return val;
}

/*
 *	Read a 4-byte value from a buffer.
 */

inline uint32 Read4
	(
	uint8 *& in
	)
	{
	uint8 b0 = *in++;
	uint8 b1 = *in++;
	uint8 b2 = *in++;
	uint8 b3 = *in++;
	return (b0 | (b1<<8) | (b2<<16) | (b3<<24));
	}

#ifdef BUFSIZ	/* Kludgy, but I don't want to include stdio.h all the time.*/
/*
 *	Read a 4-byte value from a file.
 */

inline uint32 Read4
	(
	std::FILE* in
	)
	{
	uint8 b0, b1, b2, b3;
	std::fread(&b0,sizeof(uint8),1,in);
	std::fread(&b1,sizeof(uint8),1,in);
	std::fread(&b2,sizeof(uint8),1,in);
	std::fread(&b3,sizeof(uint8),1,in);
	return (b0 | (b1<<8) | (b2<<16) | (b3<<24));
	}
#endif
/*
 *	Read a 4-byte long value, hsb first.
 */

inline uint32 Read4high (std::istream &in)
{
	uint32 val = 0;
	val |= static_cast<uint32>(in.get()<<24);
	val |= static_cast<uint32>(in.get()<<16);
	val |= static_cast<uint32>(in.get()<<8);
	val |= static_cast<uint32>(in.get());
	return val;
}

/*
 *	Read a 4-byte value from a buffer.
 */

inline uint32 Read4high
	(
	uint8 *& in
	)
	{
	uint8 b0 = *in++;
	uint8 b1 = *in++;
	uint8 b2 = *in++;
	uint8 b3 = *in++;
	return ((b0<<24) | (b1<<16) | (b2<<8) | b3);
	}

#ifdef BUFSIZ	/* Kludgy, but I don't want to include stdio.h all the time.*/
/*
 *	Read a 4-byte value from a file.
 */

inline uint32 Read4high
	(
	std::FILE* in
	)
	{
	uint8 b0, b1, b2, b3;
	std::fread(&b0,sizeof(uint8),1,in);
	std::fread(&b1,sizeof(uint8),1,in);
	std::fread(&b2,sizeof(uint8),1,in);
	std::fread(&b3,sizeof(uint8),1,in);
	return ((b0<<24) | (b1<<16) | (b2<<8) | b3);
	}
#endif

/*
 *	Write a 1-byte value.
 */

inline void Write1(std::ostream& out, uint16 val)
{
	out.put(static_cast<char> (val&0xff));
}

/*
 *	Write a 2-byte value, lsb first.
 */

inline void Write2
	(
	std::ostream& out,
	uint16 val
	)
	{
	out.put(static_cast<char> (val&0xff));
	out.put(static_cast<char> ((val>>8)&0xff));
	}

/*
 *	Write a 2-byte value, msb first.
 */

inline void Write2high
	(
	std::ostream& out,
	uint16 val
	)
	{
	out.put(static_cast<char> ((val>>8)&0xff));
	out.put(static_cast<char> (val&0xff));
	}

/*
 *	Write a 2-byte value to a buffer, lsb first.
 */

inline void Write2
	(
	uint8 *& out,		// Write here and update.
	uint16 val
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
	uint32 val
	)
	{
	out.put(static_cast<char> (val&0xff));
	out.put(static_cast<char> ((val>>8)&0xff));
	out.put(static_cast<char> ((val>>16)&0xff));
	out.put(static_cast<char> ((val>>24)&0xff));
	}

/*
 *	Write a 4-byte value, msb first.
 */

inline void Write4high
	(
	std::ostream& out,
	uint32 val
	)
	{
	out.put(static_cast<char> ((val>>24)&0xff));
	out.put(static_cast<char> ((val>>16)&0xff));
	out.put(static_cast<char> ((val>>8)&0xff));
	out.put(static_cast<char> (val&0xff));
	}

/*
 *	Write a 4-byte value to a buffer, lsb first.
 */

inline void Write4
	(
	uint8 *& out,		// Write here and update.
	uint32 val
	)
	{
	*out++ = val & 0xff;
	*out++ = (val>>8) & 0xff;
	*out++ = (val>>16)&0xff;
	*out++ = (val>>24)&0xff;
	}

inline void Write4s(std::ostream& out, sint32 val)
{
	Write4(out, static_cast<uint32>(val));
}

void U7open
	(
	std::ifstream& in,			// Input stream to open.
	const char *fname,			// May be converted to upper-case.
	bool is_text = false			// Should the file be opened in text mode
	);
void U7open
	(
	std::ofstream& out,			// Output stream to open.
	const char *fname,			// May be converted to upper-case.
	bool is_text = false			// Should the file be opened in text mode
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

inline int U7exists(std::string fname) { return U7exists(fname.c_str()); }


int U7mkdir(
	const char *dirname,
	int mode
	);

int U7chdir(
	const char *dirname
	);

void U7copy
	(
	const char *src,
	const char *dest
	);

bool is_system_path_defined(const std::string& key);
void store_system_paths();
void reset_system_paths();
void clear_system_path(const std::string& key);
void add_system_path(const std::string& key, const std::string& value);
std::string get_system_path(const std::string &path);

void to_uppercase(std::string &str);
std::string to_uppercase(const std::string &str);

int Log2(uint32 n);

char *newstrdup(const char *s);

#endif	/* _UTILS_H_ */
