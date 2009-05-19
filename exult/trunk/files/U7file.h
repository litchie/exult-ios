/*
 *  Copyright (C) 2000-2008  The Exult Team
 *
 *	Based on code by Dancer A.L Vesperman
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

#ifndef	_U7FILE_H_
#define	_U7FILE_H_

#include <fstream>
#include <string>
#include "databuf.h"
#include "U7obj.h"
#include "exceptions.h"
#include "common_types.h"

/**
 *	The U7file class is an abstract data object which is
 *	the basis of "file" reading classes for flex, table, iff
 *	and "flat" (single object) file formats.
 */
class U7file
	{
protected:
	///	This is used to uniquely identify the data object.
	///	For files, this is {"path/filename", -1}.
	/// For objects in multi-object files, this is
	/// {"path/filename", index}.
	File_spec identifier;
	///	Pointer to the DataSource which will be used by
	///	derived classes.
	DataSource *data;
	/// Causes file/buffer information to be read. Or will do,
	/// when it is implemented for derived classes.
	virtual void index_file()
		{  }
private:
	///	No default constructor.
	U7file();
	UNREPLICATABLE_CLASS(U7file);
public:
	///	Initializes from a file spec. Needed by some constructors.
	///	@param spec	A file specification.
	U7file(const File_spec &spec)
		: identifier(spec)
		{  }
	///	Deletes the data source. The destructors of derived classes
	///	should delete anything else that is needed by the datasource
	///	(e.g. buffers) but NOT the datadource.
	virtual	~U7file()
		{ delete data; }

	virtual	size_t number_of_objects(void)=0;
	virtual	char *retrieve(uint32 objnum, std::size_t &len)=0;
	virtual const char *get_archive_type()=0;
	};

/**
 *	This template class implements a basic framework for loading
 *	data from a file through a stream.
 *
 *	The template parameter class T should be derived from U7file.
 */
template <class T>
class U7DataFile : public T
	{
protected:
	///	The stream from which data will come.
	std::ifstream _file;
private:
	///	No default constructor.
	U7DataFile();
	UNREPLICATABLE_CLASS(U7DataFile);
public:
	///	This constructor treats the identifier as a file name and
	///	opens the file if it exists. It also creates and initializes
	///	the data source, or sets it to null if the file is not there.
	///	@param spec	Name of file to open. Ignores the index portion.
	U7DataFile(const char *name)
		: T(name)
		{
		if (U7exists(this->identifier.name))
			{
			U7open(this->_file, this->identifier.name);
			this->data = new StreamDataSource(&(this->_file));
			this->index_file();
			}
		else
			this->data = 0;
		}
	///	This destructor simply closes the file.
	virtual	~U7DataFile()
		{ this->_file.close(); }
	};

/**
 *	This template class implements a basic framework for loading
 *	data from a memory buffer. This class TAKES OWNERSHIP of the
 *	buffer and deletes it at the end.
 *
 *	The template parameter class T should be derived from U7file.
 */
template <class T>
class U7DataBuffer : public T
	{
protected:
	///	The buffer to read from.
	const char *_buffer;
	///	Buffer length.
	size_t _len;
private:
	///	No default constructor.
	U7DataBuffer();
	UNREPLICATABLE_CLASS(U7DataBuffer);
public:
	///	Creates and initializes the data source from the data source.
	///	Takes ownership of the parameters and deletes them when done,
	///	so callers should NOT delete them.
	///	@param spec	Unique identifier for this data object.
	///	@param dt	BufferDataSource that we shoud use.
	U7DataBuffer(const File_spec &spec, BufferDataSource *dt)
		: T(spec), _buffer((const char *)dt->getPtr()), _len(dt->getSize())
		{
		this->data = dt;
		this->index_file();
		}
	///	Creates and initializes the data source from the buffer.
	///	Takes ownership of the parameters and deletes them when done,
	///	so callers should NOT delete them.
	///	@param spec	Unique identifier for this data object.
	///	@param buf	Buffer to read from. The class deletes the buffer
	///	at the end. Can be null if l also is.
	///	@param l	Length of data in the buffer.
	U7DataBuffer(const File_spec &spec, const char *buf, unsigned int l)
		: T(spec), _buffer(buf), _len(l)
		{
		this->identifier = spec;
		this->data = new BufferDataSource(this->_buffer, this->_len);
		this->index_file();
		}
	///	Creates and initializes the data source from the specified
	///	file/number pair.
	///	@param spec	Unique identifier for this data object. The 'name'
	///	member **MUST** be a valid file name.
	U7DataBuffer(const File_spec &spec)
		: T(spec)
		{
		this->identifier = spec;
		std::size_t size;
		U7object from(spec.name, spec.index);
		this->_buffer = from.retrieve(size);
		this->data = new BufferDataSource(_buffer, this->_len = size);
		this->index_file();
		}
	///	This destructor simply deletes the buffer if non-null.
	virtual	~U7DataBuffer()
		{ if (this->_buffer) delete this->_buffer; }
	};

// Not yet.
#if 0
/**
 *	This class stores data from each file.
 */
class File_data
	{
protected:
	///	The file this refers to. Should NOT be deleted!
	U7file *file;
	///	Whether the file comes from patch dir.
	bool patch;
	///	Number of objects in the file.
	int cnt;
public:
	File_data(const File_spec& spec);
	File_data(const File_data& other)
		: file(other.file), patch(other.patch), cnt(other.cnt)
		{  }
	bool from_patch() const
		{ return patch; }
	size_t number_of_objects() const
		{ return cnt; }
	char *retrieve(uint32 objnum, std::size_t &len, bool& pt)
		{
		pt = patch;
		len = 0; return file ? file->retrieve(objnum, len) : 0;
		}
	const char *get_archive_type()
		{ return file ? file->get_archive_type() : "NONE"; }
	};

/**
 *	Abstract file loader, adapted for loading different resources
 *	from different "files".
 */
class U7multifile
	{
protected:
	///	This is the list of contained files.
	std::vector<File_data> files;
private:
	///	No default constructor.
	U7multifile();
	UNREPLICATABLE_CLASS(U7multifile);
public:
	U7multifile(const File_spec& spec);
	U7multifile(const File_spec& spec0, const File_spec& spec1);
	U7multifile(const File_spec& spec0, const File_spec& spec1,
			const File_spec& spec2);
	U7multifile(const std::vector<File_spec>& specs);
	~U7multifile();

	size_t number_of_objects(void) const;
	char *retrieve(uint32 objnum, std::size_t &len, bool& patch) const;
	};
#endif

#endif
