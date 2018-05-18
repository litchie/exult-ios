/*
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

#ifndef DATA_H
#define DATA_H

#include <cstdio>
#include <cstddef>
#include <cstring>
#include <cassert>
#include <fstream>
#include <string>
#include <iomanip>
#include <vector>
#include "U7obj.h"
#include "utils.h"

typedef char *charptr;

class IDataSource {
public:
	IDataSource() {}
	virtual ~IDataSource() {}

	virtual uint32 peek() = 0;

	virtual uint32 read1() = 0;
	virtual uint16 read2() = 0;
	virtual uint16 read2high() = 0;
	virtual uint32 read4() = 0;
	virtual uint32 read4high() = 0;
	virtual void read(void *, size_t) = 0;
	virtual void read(std::string&, size_t) = 0;

	virtual void seek(size_t) = 0;
	virtual void skip(std::streamoff) = 0;
	virtual size_t getSize() = 0;
	virtual size_t getPos() = 0;
	virtual bool eof() = 0;
	virtual bool good() {
		return true;
	}
	virtual void clear_error() { }

	void readline(std::string &str) {
		str.erase();
		while (!eof()) {
			char character =  static_cast<char>(read1());

			if (character == '\r') continue;        // Skip cr
			else if (character == '\n') break;      // break on line feed

			str += character;
		}
	}
};

class IStreamDataSource: public IDataSource {
protected:
	std::istream *in;
public:
	IStreamDataSource(std::istream *data_stream) : in(data_stream) {
	}

	virtual ~IStreamDataSource() {}

	virtual uint32 peek() {
		return in->peek();
	}

	virtual uint32 read1() {
		return Read1(in);
	}

	virtual uint16 read2() {
		return Read2(in);
	}

	virtual uint16 read2high() {
		return Read2high(in);
	}

	virtual uint32 read4() {
		return Read4(in);
	}

	virtual uint32 read4high() {
		return Read4high(in);
	}

	virtual void read(void *b, size_t len) {
		in->read(static_cast<char *>(b), len);
	}

	virtual void read(std::string& s, size_t len) {
		s.resize(len);
		in->read(&s[0], len);
	}

	virtual void seek(size_t pos) {
		in->seekg(pos);
	}

	virtual void skip(std::streamoff pos) {
		in->seekg(pos, std::ios::cur);
	}

	virtual size_t getSize() {
		size_t pos = in->tellg();
		in->seekg(0, std::ios::end);
		size_t len = in->tellg();
		in->seekg(pos);
		return len;
	}

	virtual size_t getPos() {
		return in->tellg();
	}

	virtual bool eof() {
		return in->eof();
	}
	virtual bool good() {
		return in->good();
	}
	virtual void clear_error() {
		in->clear();
	}
};

class IBufferDataSource: public IDataSource {
protected:
	const unsigned char *buf;
	const unsigned char *buf_ptr;
	std::size_t size;
public:
	IBufferDataSource(const void *data, size_t len) {
		// data can be NULL if len is also 0
		assert(data != 0 || len == 0);
		buf_ptr = buf = static_cast<const unsigned char *>(data);
		size = len;
	}

	virtual ~IBufferDataSource() {}

	virtual uint32 peek() {
		return *buf_ptr;
	}

	virtual uint32 read1() {
		return Read1(buf_ptr);
	}

	virtual uint16 read2() {
		return Read2(buf_ptr);
	}

	virtual uint16 read2high() {
		return Read2high(buf_ptr);
	}

	virtual uint32 read4() {
		return Read4(buf_ptr);
	}

	virtual uint32 read4high() {
		return Read4high(buf_ptr);
	}

	virtual void read(void *b, size_t len) {
		std::memcpy(b, buf_ptr, len);
		buf_ptr += len;
	}

	virtual void read(std::string& s, size_t len) {
		s = std::string(reinterpret_cast<const char *>(buf_ptr), len);
		buf_ptr += len;
	}

	virtual void seek(size_t pos) {
		buf_ptr = buf + pos;
	}

	virtual void skip(std::streamoff pos) {
		buf_ptr += pos;
	}

	virtual size_t getSize() {
		return size;
	}

	virtual size_t getPos() {
		return buf_ptr - buf;
	}

	const unsigned char *getPtr() {
		return buf_ptr;
	}

	virtual bool eof() {
		return buf_ptr >= buf + size;
	}
};

class IExultDataSource: public IBufferDataSource {
	unsigned char *data;
public:
	IExultDataSource(const File_spec &fname, int index)
		: IBufferDataSource(0, 0), data(0) {
		U7object obj(fname, index);
		buf = buf_ptr = data = reinterpret_cast<unsigned char *>(obj.retrieve(size));
	}

	IExultDataSource(const File_spec &fname0, const File_spec &fname1, int index)
		: IBufferDataSource(0, 0), data(0) {
		U7multiobject obj(fname0, fname1, index);
		buf = buf_ptr = data = reinterpret_cast<unsigned char *>(obj.retrieve(size));
	}

	IExultDataSource(const File_spec &fname0, const File_spec &fname1,
	                 const File_spec &fname2, int index)
		: IBufferDataSource(0, 0), data(0) {
		U7multiobject obj(fname0, fname1, fname2, index);
		buf = buf_ptr = data = reinterpret_cast<unsigned char *>(obj.retrieve(size));
	}

	~IExultDataSource() {
		delete [] data;
	}
};

class ODataSource {
public:
	ODataSource() {}
	virtual ~ODataSource() {}

	virtual void write1(uint32) = 0;
	virtual void write2(uint16) = 0;
	virtual void write2high(uint16) = 0;
	virtual void write4(uint32) = 0;
	virtual void write4high(uint32) = 0;
	virtual void write(const void *, size_t) = 0;
	virtual void write(const std::string &) = 0;

	virtual void seek(size_t) = 0;
	virtual void skip(std::streamoff) = 0;
	virtual size_t getSize() = 0;
	virtual size_t getPos() = 0;
	virtual void flush() { }
	virtual bool good() {
		return true;
	}
	virtual void clear_error() { }
};

class OStreamDataSource: public ODataSource {
protected:
	std::ostream *out;
public:
	OStreamDataSource(std::ostream *data_stream) : out(data_stream) {
	}

	virtual ~OStreamDataSource() {}

	virtual void write1(uint32 val) {
		Write1(out, static_cast<uint16>(val));
	}

	virtual void write2(uint16 val) {
		Write2(out, val);
	}

	virtual void write2high(uint16 val) {
		Write2high(out, val);
	}

	virtual void write4(uint32 val) {
		Write4(out, val);
	}

	virtual void write4high(uint32 val) {
		Write4high(out, val);
	}

	virtual void write(const void *b, size_t len) {
		out->write(static_cast<const char *>(b), len);
	}

	virtual void write(const std::string &s) {
		out->write(&s[0], s.size());
	}

	virtual void seek(size_t pos) {
		out->seekp(pos);
	}

	virtual void skip(std::streamoff pos) {
		out->seekp(pos, std::ios::cur);
	}

	virtual size_t getSize() {
		size_t pos = out->tellp();
		out->seekp(0, std::ios::end);
		size_t len = out->tellp();
		out->seekp(pos);
		return len;
	}

	virtual size_t getPos() {
		return out->tellp();
	}

	virtual void flush() {
		out->flush();
	}
	virtual bool good() {
		return out->good();
	}
	virtual void clear_error() {
		out->clear();
	}
};

class OBufferDataSource: public ODataSource {
protected:
	unsigned char *buf;
	unsigned char *buf_ptr;
	std::size_t size;
public:
	OBufferDataSource(void *data, size_t len) {
		// data can be NULL if len is also 0
		assert(data != 0 || len == 0);
		buf_ptr = buf = static_cast<unsigned char *>(data);
		size = len;
	}

	virtual ~OBufferDataSource() {}

	virtual void write1(uint32 val) {
		Write1(buf_ptr, val);
	}

	virtual void write2(uint16 val) {
		Write2(buf_ptr, val);
	}

	virtual void write2high(uint16 val) {
		Write2high(buf_ptr, val);
	}


	virtual void write4(uint32 val) {
		Write4(buf_ptr, val);
	}

	virtual void write4high(uint32 val) {
		Write4high(buf_ptr, val);
	}

	virtual void write(const void *b, size_t len) {
		std::memcpy(buf_ptr, b, len);
		buf_ptr += len;
	}

	virtual void write(const std::string &s) {
		write(&s[0], s.size());
	}

	virtual void seek(size_t pos) {
		buf_ptr = buf + pos;
	}

	virtual void skip(std::streamoff pos) {
		buf_ptr += pos;
	}

	virtual size_t getSize() {
		return size;
	}

	virtual size_t getPos() {
		return buf_ptr - buf;
	}

	unsigned char *getPtr() {
		return buf_ptr;
	}
};

class OExultDataSource: public OBufferDataSource {
public:
	OExultDataSource(const File_spec &fname, int index)
		: OBufferDataSource(0, 0) {
		U7object obj(fname, index);
		buf = buf_ptr = reinterpret_cast<unsigned char *>(obj.retrieve(size));
	}

	OExultDataSource(const File_spec &fname0, const File_spec &fname1, int index)
		: OBufferDataSource(0, 0) {
		U7multiobject obj(fname0, fname1, index);
		buf = buf_ptr = reinterpret_cast<unsigned char *>(obj.retrieve(size));
	}

	OExultDataSource(const File_spec &fname0, const File_spec &fname1,
	                 const File_spec &fname2, int index)
		: OBufferDataSource(0, 0) {
		U7multiobject obj(fname0, fname1, fname2, index);
		buf = buf_ptr = reinterpret_cast<unsigned char *>(obj.retrieve(size));
	}

	~OExultDataSource() {
		delete [] buf;
	}
};

#endif
