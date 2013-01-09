/*
 *	utils.h - Common utility routines.
 *
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
 *  Copyright (C) 2000-2013  The Exult Team
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

#ifndef _UTILS_H_
#define _UTILS_H_

#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <stdio.h>
#include <iosfwd>

#include "common_types.h"
#include "rect.h"

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
	uint32 val = in.get();
	val |= (in.get()<<8);
	return static_cast<uint16>(val);
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
	return static_cast<uint16>(b0 | (b1 << 8));
	}

/*
 *	Read a 2-byte value from a file.
 */

inline uint16 Read2
	(
	std::FILE* in
	)
	{
	uint8 b0, b1;
	if (std::fread(&b0,sizeof(uint8),1,in) != 1) b0 = 0;
	if (std::fread(&b1,sizeof(uint8),1,in) != 1) b1 = 0;
	return static_cast<uint16>(b0 | (b1 << 8));
	}

/*
 *	Read a 2-byte value, hsb first.
 */

inline uint16 Read2high (std::istream &in)
{
	uint32 val = in.get()<<8;
	val |= in.get();
	return static_cast<uint16>(val);
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
	return static_cast<uint16>((b0 << 8) | b1);
	}

/*
 *	Read a 2-byte value from a file.
 */

inline uint16 Read2high
	(
	std::FILE* in
	)
	{
	uint8 b0, b1;
	if (std::fread(&b0,sizeof(uint8),1,in) != 1) b0 = 0;
	if (std::fread(&b1,sizeof(uint8),1,in) != 1) b1 = 0;
	return static_cast<uint16>((b0 << 8) | b1);
	}

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

/*
 *	Read a 4-byte value from a file.
 */

inline uint32 Read4
	(
	std::FILE* in
	)
	{
	uint8 b0, b1, b2, b3;
	if (std::fread(&b0,sizeof(uint8),1,in) != 1) b0 = 0;
	if (std::fread(&b1,sizeof(uint8),1,in) != 1) b1 = 0;
	if (std::fread(&b2,sizeof(uint8),1,in) != 1) b2 = 0;
	if (std::fread(&b3,sizeof(uint8),1,in) != 1) b3 = 0;
	return (b0 | (b1<<8) | (b2<<16) | (b3<<24));
	}

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

/*
 *	Read a 4-byte value from a file.
 */
inline uint32 Read4high
	(
	std::FILE* in
	)
	{
	uint8 b0, b1, b2, b3;
	if (std::fread(&b0,sizeof(uint8),1,in) != 1) b0 = 0;
	if (std::fread(&b1,sizeof(uint8),1,in) != 1) b1 = 0;
	if (std::fread(&b2,sizeof(uint8),1,in) != 1) b2 = 0;
	if (std::fread(&b3,sizeof(uint8),1,in) != 1) b3 = 0;
	return ((b0<<24) | (b1<<16) | (b2<<8) | b3);
	}

/*
 *	Read a 2-byte value, lsb first, unsigned.
 */
inline sint16 Read2s (uint8 *& in)
{	return static_cast<sint16>(Read2(in));	}

inline sint16 Read2s (std::istream &in)
{	return static_cast<sint16>(Read2(in));	}

/*
 *	Read a 4-byte value, lsb first, unsigned.
 */
inline sint32 Read4s (uint8 *& in)
{	return static_cast<sint32>(Read4(in));	}

inline sint32 Read4s (std::istream &in)
{	return static_cast<sint32>(Read4(in));	}

/*
 *	Read a N-byte long value, lsb first.
 */

template<typename T>
inline T ReadN (std::istream &in)
{
	T val = 0;
	for(size_t i = 0; i < sizeof(T); i++)
		val |= static_cast<T>(T(in.get())<<(8*i));
	return val;
}

/*
 *	Read a N-byte value from a buffer.
 */

template<typename T>
inline T ReadN
	(
	uint8 *& in
	)
	{
	T val = 0;
	for(size_t i = 0; i < sizeof(T); i++)
		val |= static_cast<T>(T((*in++))<<(8*i));
	return val;
	}

/*
 *	Read a N-byte value from a file.
 */

template<typename T>
inline T ReadN
	(
	std::FILE* in
	)
	{
	T val = 0;
	for(size_t i = 0; i < sizeof(T); i++)
		{
		uint8 b0;
		if (std::fread(&b0,sizeof(uint8),1,in) != 1)
			val |= static_cast<T>(T(b0)<<(8*i));
		}
	return val;
	}

/*
 *	Read a N-byte long value, hsb first.
 */

template<typename T>
inline T ReadNhigh (std::istream &in)
{
	T val = 0;
	for(int i = sizeof(T) - 1; i >= 0; i--)
		val |= static_cast<T>(T(in.get())<<(8*i));
	return val;
}

/*
 *	Read a N-byte value from a buffer.
 */

template<typename T>
inline T ReadNhigh
	(
	uint8 *& in
	)
	{
	T val = 0;
	for(int i = sizeof(T) - 1; i >= 0; i--)
		val |= static_cast<T>(T((*in++))<<(8*i));
	return val;
	}

/*
 *	Read a N-byte value from a file.
 */

template<typename T>
inline T ReadNhigh
	(
	std::FILE* in
	)
	{
	T val = 0;
	for(int i = sizeof(T) - 1; i >= 0; i--)
		{
		uint8 b0;
		if (std::fread(&b0,sizeof(uint8),1,in) != 1)
			val |= static_cast<T>(T(b0)<<(8*i));
		}
	return val;
	}

inline int ReadInt(std::istream& in, int def = 0)
	{
	int num;
	if (in.eof())
		return def;
	in >> num;
	if (in.fail())
		return def;
	in.ignore(0xffffff, '/');
	return num;
	}

inline unsigned int ReadUInt(std::istream& in, unsigned int def = 0)
	{
	unsigned int num;
	if (in.eof())
		return def;
	in >> num;
	if (in.fail())
		return def;
	in.ignore(0xffffff, '/');
	return num;
	}

inline void WriteInt
	(
	std::ostream& out,
	int num,
	bool final = false
	)
	{
	out << num;
	if (final)
		out << std::endl;
	else
		out << '/';
	}

inline void WriteInt
	(
	std::ostream& out,
	unsigned int num,
	bool final = false
	)
	{
	out << num;
	if (final)
		out << std::endl;
	else
		out << '/';
	}

inline void ReadRect
	(
	std::istream& in,
	Rectangle& rect
	)
	{
	rect.x = ReadInt(in);
	rect.y = ReadInt(in);
	rect.w = ReadInt(in);
	rect.h = ReadInt(in);
	}

inline void WriteRect
	(
	std::ostream& out,
	const Rectangle& rect,
	bool final = false
	)
	{
	WriteInt(out, rect.x);
	WriteInt(out, rect.y);
	WriteInt(out, rect.w);
	WriteInt(out, rect.h, final);
	}

inline std::string ReadStr(char *&eptr, int off = 1)
	{
	eptr += off;
	char *pos = std::strchr(eptr, '/');
	std::string retval(eptr, pos - eptr);
	eptr = pos;
	return retval;
	}

inline std::string ReadStr(std::istream& in)
	{
	std::string retval;
	std::getline(in, retval, '/');
	return retval;
	}

inline void WriteStr
	(
	std::ostream& out,
	std::string str,
	bool final = false
	)
	{
	out << str;
	if (final)
		out << std::endl;
	else
		out << '/';
	}

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
	*out++ = static_cast<char> (val & 0xff);
	*out++ = static_cast<char> ((val>>8) & 0xff);
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
	*out++ = static_cast<char> (val & 0xff);
	*out++ = static_cast<char> ((val>>8) & 0xff);
	*out++ = static_cast<char> ((val>>16)&0xff);
	*out++ = static_cast<char> ((val>>24)&0xff);
	}

/*
 *	Write a signed 4-byte value to a stream, lsb first.
 */

inline void Write4s(std::ostream& out, sint32 val)
{
	Write4(out, static_cast<uint32>(val));
}

/*
 *	Write a N-byte value, lsb first.
 */

template<typename T>
inline void WriteN
	(
	std::ostream& out,
	T val
	)
	{
	for(size_t i = 0; i < sizeof(T); i++)
		out.put(static_cast<char>((val>>(8*i))&0xff));
	}

/*
 *	Write a N-byte value, msb first.
 */

template<typename T>
inline void WriteNhigh
	(
	std::ostream& out,
	T val
	)
	{
	for(int i = sizeof(T) - 1; i >= 0 ; i--)
		out.put(static_cast<char>((val>>(8*i))&0xff));
	}

/*
 *	Write a N-byte value to a buffer, lsb first.
 */

template<typename T>
inline void WriteN
	(
	uint8 *& out,		// Write here and update.
	T val
	)
	{
	for(size_t i = 0; i < sizeof(T); i++)
		*out++ = static_cast<char>((val>>(8*i))&0xff);
	}

bool U7open
	(
	std::ifstream& in,			// Input stream to open.
	const char *fname,			// May be converted to upper-case.
	bool is_text = false			// Should the file be opened in text mode
	);
bool U7open
	(
	std::ofstream& out,			// Output stream to open.
	const char *fname,			// May be converted to upper-case.
	bool is_text = false			// Should the file be opened in text mode
	);

std::FILE* U7open
	(
	const char *fname,			// May be converted to upper-case.
	const char *mode			// File access mode.
	);

bool U7open_static
	(
	std::ifstream& in,		// Input stream to open.
	const char *fname,		// May be converted to upper-case.
	bool is_text			// Should file be opened in text mode
	);
void U7remove(
	const char *fname
	);

bool U7exists(
	const char *fname
	);

inline bool U7exists(std::string fname) { return U7exists(fname.c_str()); }

#ifdef UNDER_CE
inline void perror(const char *errmsg) { return; }
#define strdup myce_strdup
char *myce_strdup(const char *s);

#endif

int U7mkdir(
	const char *dirname,
	int mode
	);

#ifdef WIN32
void redirect_output(const char *prefix = "std");
void cleanup_output(const char *prefix = "std");
#endif
void setup_data_dir(const std::string& data_path, const char *runpath);
void setup_program_paths();

// These are not supported in WinCE (PocketPC) for now
#ifndef UNDER_CE

int U7chdir(
	const char *dirname
	);

void U7copy
	(
	const char *src,
	const char *dest
	);

#endif //UNDER_CE

bool is_system_path_defined(const std::string& key);
void store_system_paths();
void reset_system_paths();
void clear_system_path(const std::string& key);
void add_system_path(const std::string& key, const std::string& value);
void clone_system_path(const std::string& new_key, const std::string& old_key);
std::string get_system_path(const std::string &path);

#ifdef MACOSX
#define BUNDLE_CHECK(x,y) ((is_system_path_defined("<BUNDLE>") && U7exists((x)))  ? (x) : (y))
#else
#define BUNDLE_CHECK(x,y) (y)
#endif

void to_uppercase(std::string &str);
std::string to_uppercase(const std::string &str);

int Log2(uint32 n);
uint32 msb32(uint32 n);
int fgepow2(uint32 n);

char *newstrdup(const char *s);
char *Get_mapped_name(const char *from, int num, char *to);
int Find_next_map(int start, int maxtry);


inline int bitcount (unsigned char n)
	{
#define TWO(c)     (0x1u << (c))
#define MASK(c)    ((static_cast<uint32>(-1)) / (TWO(TWO(c)) + 1u))
#define COUNT(x,c) ((x) & MASK(c)) + (((x) >> (TWO(c))) & MASK(c))
	// Only works for 8-bit numbers.
	n = static_cast<unsigned char> (COUNT(n, 0));
	n = static_cast<unsigned char> (COUNT(n, 1));
	n = static_cast<unsigned char> (COUNT(n, 2));
	return n;
	}

#endif	/* _UTILS_H_ */
