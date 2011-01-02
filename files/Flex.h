/*
 *  Copyright (C) 2000-2011  The Exult Team
 *
 *	Original file by Dancer A.L Vesperman
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

#ifndef	__FLEX_H_
#define	__FLEX_H_

#include <vector>
#include <string>
#include <cstring>
#include <iosfwd>
#include "common_types.h"
#include "U7file.h"
#include "exceptions.h"

class DataSource;

/**
 *	The Flex class is an data reader which reads data in the flex
 *	file format. The actual data need not be in a file, however.
 */
class Flex : public U7file
	{
public:
	///	Exult has an extended flex file format to support
	///	extended shape numbers in IFIX objects.
	enum Flex_vers
		{
		orig = 0,		///<	Original file version.
		exult_v2 = 1	///<	Exult extension for IFIX objects.
		};
protected:
	static const unsigned short EXULT_FLEX_MAGIC2 = 0x0000cc00;
	char	title[80];
	uint32	magic1;
	uint32	count;
	uint32	magic2;
	uint32	padding[9];
	struct Reference
		{
		uint32 offset;
		uint32 size;
		Reference() : offset(0),size(0) {};
		};
	std::vector<Reference> object_list;

	virtual void index_file();
public:
	///	Basic constructor.
	///	@param spec	File name and object index pair.
	Flex(const File_spec &spec)
		: U7file(spec)
		{  }

	///	Inspect the version of a flex file.
	///	@return	The flex version.
	Flex_vers get_vers() const
		{
		return (magic2&~0xff) == EXULT_FLEX_MAGIC2 ? 
			static_cast<Flex_vers>(magic2&0xff) : orig;
		}
	///	Gets the number of objects in the flex.
	///	@return	Number of objects.
	virtual size_t	number_of_objects()
		{ return object_list.size(); };
   	virtual	char *	retrieve(uint32 objnum, std::size_t &len); 
	uint32 	get_entry_info(uint32 objnum, size_t &len);
	virtual const char *get_archive_type()
		{ return "FLEX"; };
	static void write_header(DataSource* out, const char *title, size_t count,
						Flex_vers vers = orig);

	static bool is_flex(DataSource *in);
	static bool is_flex(const char *fname);
private:
	/// No default constructor.
	Flex();
	UNREPLICATABLE_CLASS_I(Flex, U7file(""));
	void IndexFlexFile(void);
	};

typedef U7DataFile<Flex> FlexFile;
typedef U7DataBuffer<Flex> FlexBuffer;

/*
 *	This is for writing out a whole Flex file.
 */
class Flex_writer
	{
	std::ofstream *out;		// What we're writing to.
	DataSource *dout;		// Or this, if non-0.
	size_t count;			// # entries.
	long cur_start;			// Start of cur. entry being written.
	uint8 *table;			// Table of offsets & lengths.
	uint8 *tptr;			// ->into table.
	int index;			// Current index.
public:
	Flex_writer(std::ofstream &o, const char *title, size_t cnt,
					Flex::Flex_vers vers = Flex::orig);
	Flex_writer(DataSource* o, const char *title, size_t cnt,
					Flex::Flex_vers vers = Flex::orig);
	~Flex_writer();
	void mark_section_done();	// Finished writing out a section.
	bool close();			// All done.
	};

#endif
