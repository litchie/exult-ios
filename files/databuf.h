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

/**
 * Abstract input base class.
 */
class IDataSource {
public:
	IDataSource() = default;
	IDataSource(const IDataSource&) = delete;
	IDataSource& operator=(const IDataSource&) = delete;
	IDataSource(IDataSource&&) noexcept = default;
	IDataSource& operator=(IDataSource&&) noexcept = default;
	virtual ~IDataSource() noexcept = default;

	virtual uint32 peek() = 0;

	virtual uint32 read1() = 0;
	virtual uint16 read2() = 0;
	virtual uint16 read2high() = 0;
	virtual uint32 read4() = 0;
	virtual uint32 read4high() = 0;
	virtual void read(void *, size_t) = 0;
	virtual void read(std::string&, size_t) = 0;
	virtual IDataSource *read(size_t) = 0;

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

/**
 * Stream-based input data source which does not own the stream.
 */
class IStreamDataSource: public IDataSource {
protected:
	std::istream *in;
public:
	explicit IStreamDataSource(std::istream *data_stream)
		: in(data_stream) {}

	IStreamDataSource(IStreamDataSource&&) noexcept = default;
	IStreamDataSource& operator=(IStreamDataSource&&) noexcept = default;
	~IStreamDataSource() noexcept override = default;

	uint32 peek() override final {
		return in->peek();
	}

	uint32 read1() override final {
		return Read1(in);
	}

	uint16 read2() override final {
		return Read2(in);
	}

	uint16 read2high() override final {
		return Read2high(in);
	}

	uint32 read4() override final {
		return Read4(in);
	}

	uint32 read4high() override final {
		return Read4high(in);
	}

	void read(void *b, size_t len) override final {
		in->read(static_cast<char *>(b), len);
	}

	void read(std::string& s, size_t len) override final {
		s.resize(len);
		in->read(&s[0], len);
	}

	IDataSource *read(size_t len) override final;

	void seek(size_t pos) override final {
		in->seekg(pos);
	}

	void skip(std::streamoff pos) override final {
		in->seekg(pos, std::ios::cur);
	}

	size_t getSize() override final {
		size_t pos = in->tellg();
		in->seekg(0, std::ios::end);
		size_t len = in->tellg();
		in->seekg(pos);
		return len;
	}

	size_t getPos() override final {
		return in->tellg();
	}

	bool eof() override final {
		return in->eof();
	}
	bool good() override final {
		return in->good();
	}
	void clear_error() override final {
		in->clear();
	}
};

/**
 * File-based input data source does owns the stream.
 */

class IFileDataSource : public IStreamDataSource {
public:
	explicit IFileDataSource(std::ifstream *data_stream)
		: IStreamDataSource(data_stream) {}

	explicit IFileDataSource(const File_spec &spec)
		: IStreamDataSource(0) {
		std::ifstream *fin = new std::ifstream();
		in = fin;
		if (U7exists(spec.name)) {
			U7open(*fin, spec.name.c_str());
		} else {
			// Set fail bit
			fin->seekg(0);
		}
	}

	IFileDataSource(IFileDataSource&& other) noexcept
		: IStreamDataSource(other.in) {
		other.in = nullptr;
	}

	IFileDataSource& operator=(IFileDataSource&& other) noexcept {
		std::swap(in, other.in);
		return *this;
	}

	~IFileDataSource() override final {
		delete in;
	}
};

/**
 * Buffer-based input data source which does not own the buffer.
 */
class IBufferDataView: public IDataSource {
protected:
	const unsigned char *buf;
	const unsigned char *buf_ptr;
	std::size_t size;
public:
	IBufferDataView(const void *data, size_t len) {
		// data can be NULL if len is also 0
		assert(data != 0 || len == 0);
		buf_ptr = buf = static_cast<const unsigned char *>(data);
		size = len;
	}

	IBufferDataView(IBufferDataView&&) noexcept = default;
	IBufferDataView& operator=(IBufferDataView&&) noexcept = default;
	~IBufferDataView() noexcept override = default;

	uint32 peek() override final {
		return *buf_ptr;
	}

	uint32 read1() override final {
		return Read1(buf_ptr);
	}

	uint16 read2() override final {
		return Read2(buf_ptr);
	}

	uint16 read2high() override final {
		return Read2high(buf_ptr);
	}

	uint32 read4() override final {
		return Read4(buf_ptr);
	}

	uint32 read4high() override final {
		return Read4high(buf_ptr);
	}

	void read(void *b, size_t len) override final {
		std::memcpy(b, buf_ptr, len);
		buf_ptr += len;
	}

	void read(std::string& s, size_t len) override final {
		s = std::string(reinterpret_cast<const char *>(buf_ptr), len);
		buf_ptr += len;
	}

	IDataSource *read(size_t len) override final;

	void seek(size_t pos) override final {
		buf_ptr = buf + pos;
	}

	void skip(std::streamoff pos) override final {
		buf_ptr += pos;
	}

	size_t getSize() override final {
		return size;
	}

	size_t getPos() override final {
		return buf_ptr - buf;
	}

	const unsigned char *getPtr() {
		return buf_ptr;
	}

	bool eof() override final {
		return buf_ptr >= buf + size;
	}

	bool good() override final {
		return buf && size;
	}
};

/**
 * Buffer-based input data source which owns the stream.
 */
class IBufferDataSource: public IBufferDataView {
protected:
	char *data;
public:
	IBufferDataSource(void *data_, size_t len)
		: IBufferDataView(0, 0), data(static_cast<char*>(data_)) {
		// data can be NULL if len is also 0
		assert(data != 0 || len == 0);
		buf_ptr = buf = reinterpret_cast<const unsigned char *>(data);
		size = len;
	}

	IBufferDataSource(IBufferDataSource&& other) noexcept
		: IBufferDataView(std::move(other)), data(std::move(other.data)) {
		other.data = nullptr;
	}
	IBufferDataSource& operator=(IBufferDataSource&& other) noexcept {
		std::swap(buf, other.buf);
		std::swap(buf_ptr, other.buf_ptr);
		std::swap(data, other.data);
		return *this;
	}

	~IBufferDataSource() override {
		delete [] data;
	}
};

/**
 * Buffer-based input data source which opens an U7 object or
 * multiobject, and reads into an internal buffer.
 */
class IExultDataSource: public IBufferDataSource {
public:
	IExultDataSource(const File_spec &fname, int index)
		: IBufferDataSource(0, 0) {
		U7object obj(fname, index);
		data = obj.retrieve(size);
		buf = buf_ptr = reinterpret_cast<unsigned char *>(data);
	}

	IExultDataSource(const File_spec &fname0, const File_spec &fname1, int index)
		: IBufferDataSource(0, 0) {
		U7multiobject obj(fname0, fname1, index);
		data = obj.retrieve(size);
		buf = buf_ptr = reinterpret_cast<unsigned char *>(data);
	}

	IExultDataSource(const File_spec &fname0, const File_spec &fname1,
	                 const File_spec &fname2, int index)
		: IBufferDataSource(0, 0) {
		U7multiobject obj(fname0, fname1, fname2, index);
		data = obj.retrieve(size);
		buf = buf_ptr = reinterpret_cast<unsigned char *>(data);
	}
};

inline IDataSource *IStreamDataSource::read(size_t len) {
	char *buf = new char[len];
	read(buf, len);
	return new IBufferDataSource(buf, len);
}

inline IDataSource *IBufferDataView::read(size_t len) {
	size_t avail = getSize() - getPos();
	if (avail < len) {
		len = avail;
	}
	const unsigned char *ptr = getPtr();
	skip(len);
	return new IBufferDataView(ptr, len);
}

/**
 * Abstract output base class.
 */
class ODataSource {
public:
	ODataSource() = default;
	ODataSource(const ODataSource&) = delete;
	ODataSource& operator=(const ODataSource&) = delete;
	ODataSource(ODataSource&&) noexcept = default;
	ODataSource& operator=(ODataSource&&) noexcept = default;
	virtual ~ODataSource() noexcept = default;

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

/**
 * Stream-based output data source which does not own the stream.
 */
class OStreamDataSource: public ODataSource {
protected:
	std::ostream *out;
public:
	explicit OStreamDataSource(std::ostream *data_stream)
		: out(data_stream) {}

	OStreamDataSource(OStreamDataSource&&) noexcept = default;
	OStreamDataSource& operator=(OStreamDataSource&&) noexcept = default;
	~OStreamDataSource() noexcept override = default;

	void write1(uint32 val) override final {
		Write1(out, static_cast<uint16>(val));
	}

	void write2(uint16 val) override final {
		Write2(out, val);
	}

	void write2high(uint16 val) override final {
		Write2high(out, val);
	}

	void write4(uint32 val) override final {
		Write4(out, val);
	}

	void write4high(uint32 val) override final {
		Write4high(out, val);
	}

	void write(const void *b, size_t len) override final {
		out->write(static_cast<const char *>(b), len);
	}

	void write(const std::string &s) override final {
		out->write(&s[0], s.size());
	}

	void seek(size_t pos) override final {
		out->seekp(pos);
	}

	void skip(std::streamoff pos) override final {
		out->seekp(pos, std::ios::cur);
	}

	size_t getSize() override final {
		size_t pos = out->tellp();
		out->seekp(0, std::ios::end);
		size_t len = out->tellp();
		out->seekp(pos);
		return len;
	}

	size_t getPos() override final {
		return out->tellp();
	}

	void flush() override final {
		out->flush();
	}
	bool good() override final {
		return out->good();
	}
	void clear_error() override final {
		out->clear();
	}
};

/**
 * File-based output data source which owns the stream.
 */
class OFileDataSource : public OStreamDataSource {
public:
	explicit OFileDataSource(std::ofstream *data_stream)
		: OStreamDataSource(data_stream) {}

	OFileDataSource(OFileDataSource&& other) noexcept
		: OStreamDataSource(std::move(other)) {
		other.out = nullptr;
	}
	OFileDataSource& operator=(OFileDataSource&& other) noexcept {
		std::swap(out, other.out);
		return *this;
	}

	~OFileDataSource() override final {
		delete out;
	}
};

/**
 * Buffer-based output data source which does not own the buffer.
 */
class OBufferDataSpan: public ODataSource {
protected:
	unsigned char *buf;
	unsigned char *buf_ptr;
	std::size_t size;
public:
	OBufferDataSpan(void *data, size_t len) {
		// data can be NULL if len is also 0
		assert(data != 0 || len == 0);
		buf_ptr = buf = static_cast<unsigned char *>(data);
		size = len;
	}

	OBufferDataSpan(OBufferDataSpan&&) noexcept = default;
	OBufferDataSpan& operator=(OBufferDataSpan&&) noexcept = default;
	~OBufferDataSpan() noexcept override = default;

	void write1(uint32 val) override final {
		Write1(buf_ptr, val);
	}

	void write2(uint16 val) override final {
		Write2(buf_ptr, val);
	}

	void write2high(uint16 val) override final {
		Write2high(buf_ptr, val);
	}

	void write4(uint32 val) override final {
		Write4(buf_ptr, val);
	}

	void write4high(uint32 val) override final {
		Write4high(buf_ptr, val);
	}

	void write(const void *b, size_t len) override final {
		std::memcpy(buf_ptr, b, len);
		buf_ptr += len;
	}

	void write(const std::string &s) override final {
		write(&s[0], s.size());
	}

	void seek(size_t pos) override final {
		buf_ptr = buf + pos;
	}

	void skip(std::streamoff pos) override final {
		buf_ptr += pos;
	}

	size_t getSize() override final {
		return size;
	}

	size_t getPos() override final {
		return buf_ptr - buf;
	}

	unsigned char *getPtr() {
		return buf_ptr;
	}
};

/**
 * Buffer-based output data source which owns the buffer.
 */
class OBufferDataSource: public OBufferDataSpan {
public:
	OBufferDataSource(void *data, size_t len)
		: OBufferDataSpan(data, len) {}

	OBufferDataSource(OBufferDataSource&& other) noexcept
		: OBufferDataSpan(std::move(other)) {
		other.buf = nullptr;
	}
	OBufferDataSource& operator=(OBufferDataSource&& other) noexcept {
		std::swap(buf, other.buf);
		return *this;
	}

	~OBufferDataSource() override final {
		delete [] buf;
	}
};

#endif
