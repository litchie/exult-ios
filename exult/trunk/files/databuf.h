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

#ifndef ALPHA_LINUX_CXX
#  include <cstdio>
#  include <cstring>
#endif
#include <cassert>
#include <fstream>
#include <iomanip>
#include "U7file.h"
#include "utils.h"

typedef char * charptr;

class DataSource
{
public:
	DataSource() {};
	virtual ~DataSource() {};

	virtual uint32 peek() =0;
	
	virtual uint32 read1() =0;
	virtual uint16 read2() =0;
	virtual uint16 read2high() =0;
	virtual uint32 read4() =0;
	virtual uint32 read4high() =0;
	virtual void read(char *, int) =0;
	
	virtual void write1(uint32) =0;
	virtual void write2(uint16) =0;
	virtual void write2high(uint16) =0;
	virtual void write4(uint32) =0;
	virtual void write4high(uint32) =0;
	virtual void write(char *, int) =0;
	
	virtual void seek(unsigned int) =0;
	virtual void skip(int) =0;
	virtual unsigned int getSize() =0;
	virtual unsigned int getPos() =0;
	virtual bool eof() =0;
	virtual void flush() { }
	virtual bool good() { return true; }
};

class StreamDataSource: public DataSource
{
private:
	std::ifstream *in;
	std::ofstream *out;
public:
	StreamDataSource(std::ifstream *data_stream) : in(data_stream), out(0)
	{
	};
	
	StreamDataSource(std::ofstream *data_stream) : in(0), out(data_stream)
	{
	};
	
	virtual ~StreamDataSource() {};

	virtual uint32 peek()		{ return in->peek(); };
	
	virtual uint32 read1()      { return Read1(*in); };
	
	virtual uint16 read2()      { return Read2(*in); };
	
	virtual uint16 read2high()  { return Read2high(*in); };
	
	virtual uint32 read4()      { return Read4(*in); };
	
	virtual uint32 read4high()  { return Read4high(*in); };
	
	void read(char *b, int len) { in->read(b, len); };
	
	virtual void write1(uint32 val)      { Write1(*out, val); };
	
	virtual void write2(uint16 val)      { Write2(*out, val); };

	virtual void write2high(uint16 val)  { Write2high(*out, val); };

	virtual void write4(uint32 val)      { Write4(*out, val); };

	virtual void write4high(uint32 val)  { Write4high(*out, val); };

	virtual void write(char *b, int len) { out->write(b, len); };
	
	virtual void seek(unsigned int pos)
	{
		if (in) in->seekg(pos);
		else out->seekp(pos);
	};

	virtual void skip(int pos) { in->seekg(pos, std::ios::cur); };
	
	virtual unsigned int getSize()
	{
		if (in) {
			long pos = in->tellg();
			in->seekg(0, std::ios::end);
			long len = in->tellg();
			in->seekg(pos);
			return len;
		}
		else {
			long pos = out->tellp();
			out->seekp(0, std::ios::end);
			long len = out->tellp();
			out->seekp(pos);
			return len;
		}
	};
	
	virtual unsigned int getPos() { return in?in->tellg():out->tellp(); };
	
	virtual bool eof() { return in->eof(); }
	virtual void flush() { if (out) out->flush(); }
	virtual bool good() { return in ? in->good() : out->good(); }
};

class FileDataSource: public DataSource
{
private:
	std::FILE *f;
public:
	FileDataSource(std::FILE *fp)
	{
		f = fp;
	};
	
	virtual ~FileDataSource() {};
	
	virtual uint32 peek()
	{ 
		unsigned char b0;
		b0 = fgetc(f);
		fseek(f, -1, SEEK_CUR);
		return (b0);
	};
	
	virtual uint32 read1() 
	{ 
		unsigned char b0;
		b0 = fgetc(f);
		return (b0);
	};
	
	virtual uint16 read2()
	{
		unsigned char b0, b1;
		b0 = fgetc(f);
		b1 = fgetc(f);
		return (b0 | (b1 << 8));
	};
	
	virtual uint16 read2high()
	{
		unsigned char b0, b1;
		b1 = fgetc(f);
		b0 = fgetc(f);
		return (b0 | (b1 << 8));
	};
	
	virtual uint32 read4()
	{
		unsigned char b0, b1, b2, b3;
		b0 = fgetc(f);
		b1 = fgetc(f);
		b2 = fgetc(f);
		b3 = fgetc(f);
		return (b0 | (b1<<8) | (b2<<16) | (b3<<24));
	};
	
	virtual uint32 read4high()
	{
		unsigned char b0, b1, b2, b3;
		b3 = fgetc(f);
		b2 = fgetc(f);
		b1 = fgetc(f);
		b0 = fgetc(f);
		return (b0 | (b1<<8) | (b2<<16) | (b3<<24));
	};
	
	void read(char *b, int len) {
		fread(b, 1, len, f);
	};
	
	virtual void write1(uint32 val)
	{
		fputc(static_cast<char>(val&0xff),f);
	};
	
	virtual void write2(uint16 val)
	{
		fputc(static_cast<char>(val&0xff),f);
		fputc(static_cast<char>((val>>8)&0xff),f);
	};
	
	virtual void write2high(uint16 val)
	{
		fputc(static_cast<char>((val>>8)&0xff),f);
		fputc(static_cast<char>(val&0xff),f);
	};
	
	virtual void write4(uint32 val)
	{
		fputc(static_cast<char>(val&0xff),f);
		fputc(static_cast<char>((val>>8)&0xff),f);
		fputc(static_cast<char>((val>>16)&0xff),f);
		fputc(static_cast<char>((val>>24)&0xff),f);
	};

	virtual void write4high(uint32 val)
	{
		fputc(static_cast<char>((val>>24)&0xff),f);
		fputc(static_cast<char>((val>>16)&0xff),f);
		fputc(static_cast<char>((val>>8)&0xff),f);
		fputc(static_cast<char>(val&0xff),f);
	};

	virtual void write(char *b, int len)
	{
		fwrite(b, 1, len, f);
	};
	
	
	virtual void seek(unsigned int pos) { fseek(f, pos, SEEK_SET); };
	
	virtual void skip(int pos) { fseek(f, pos, SEEK_CUR); };
	
	virtual unsigned int getSize()
	{
		long pos = ftell(f);
		fseek(f, 0, SEEK_END);
		long len = ftell(f);
		fseek(f, pos, SEEK_SET);
		return len;
	};
	
	virtual unsigned int getPos()
	{
		return ftell(f);
	};

	virtual bool eof() { return feof(f) != 0; } 
};

class BufferDataSource: public DataSource
{
protected:
	/* const solely so that no-one accidentally modifies it.
		data is being passed 'non-const' anyway */
	const unsigned char *buf;
	unsigned char *buf_ptr;
	std::size_t size;
public:
	BufferDataSource(char *data, unsigned int len)
	{
		// data can be NULL if len is also 0
		assert(data!=0 || len==0);
		buf = buf_ptr = reinterpret_cast<unsigned char*>(data);
		size = len;
	};
	
	void load(char *data, unsigned int len)
	{
		// data can be NULL if len is also 0
		assert(data!=0 || len==0);
		buf = buf_ptr = reinterpret_cast<unsigned char*>(data);
		size = len;
	};
	
	virtual ~BufferDataSource() {};
	
	virtual uint32 peek() 
	{ 
		unsigned char b0;
		b0 = static_cast<unsigned char>(*buf_ptr);
		return (b0);
	};
	
	virtual uint32 read1() 
	{ 
		unsigned char b0;
		b0 = static_cast<unsigned char>(*buf_ptr++);
		return (b0);
	};
	
	virtual uint16 read2()
	{
		unsigned char b0, b1;
		b0 = static_cast<unsigned char>(*buf_ptr++);
		b1 = static_cast<unsigned char>(*buf_ptr++);
		return (b0 | (b1 << 8));
	};
	
	virtual uint16 read2high()
	{
		unsigned char b0, b1;
		b1 = static_cast<unsigned char>(*buf_ptr++);
		b0 = static_cast<unsigned char>(*buf_ptr++);
		return (b0 | (b1 << 8));
	};
	
	virtual uint32 read4()
	{
		unsigned char b0, b1, b2, b3;
		b0 = static_cast<unsigned char>(*buf_ptr++);
		b1 = static_cast<unsigned char>(*buf_ptr++);
		b2 = static_cast<unsigned char>(*buf_ptr++);
		b3 = static_cast<unsigned char>(*buf_ptr++);
		return (b0 | (b1<<8) | (b2<<16) | (b3<<24));
	};
	
	virtual uint32 read4high()
	{
		unsigned char b0, b1, b2, b3;
		b3 = static_cast<unsigned char>(*buf_ptr++);
		b2 = static_cast<unsigned char>(*buf_ptr++);
		b1 = static_cast<unsigned char>(*buf_ptr++);
		b0 = static_cast<unsigned char>(*buf_ptr++);
		return (b0 | (b1<<8) | (b2<<16) | (b3<<24));
	};
	
	void read(char *b, int len) {
		std::memcpy(b, buf_ptr, len);
		buf_ptr += len;
	};
	
	virtual void write1(uint32 val)
	{
		*buf_ptr++ = val & 0xff;
	};
	
	virtual void write2(uint16 val)
	{
		*buf_ptr++ = val & 0xff;
		*buf_ptr++ = (val>>8) & 0xff;
	};

	virtual void write2high(uint16 val)
	{
		*buf_ptr++ = (val>>8) & 0xff;
		*buf_ptr++ = val & 0xff;
	};

	
	virtual void write4(uint32 val)
	{
		*buf_ptr++ = val & 0xff;
		*buf_ptr++ = (val>>8) & 0xff;
		*buf_ptr++ = (val>>16)&0xff;
		*buf_ptr++ = (val>>24)&0xff;
	};
	
	virtual void write4high(uint32 val)
	{
		*buf_ptr++ = (val>>24)&0xff;
		*buf_ptr++ = (val>>16)&0xff;
		*buf_ptr++ = (val>>8) & 0xff;
		*buf_ptr++ = val & 0xff;
	};

	virtual void write(char *b, int len)
	{
		std::memcpy(buf_ptr, b, len);
		buf_ptr += len;
	};
	
	virtual void seek(unsigned int pos) { buf_ptr = const_cast<unsigned char *>(buf)+pos; };
	
	virtual void skip(int pos) { buf_ptr += pos; };
	
	virtual unsigned int getSize() { return size; };
	
	virtual unsigned int getPos() { return (buf_ptr-buf); };
	
	unsigned char *getPtr() { return buf_ptr; };

	virtual bool eof() { return (buf_ptr-buf) >= (int)size; } 

};

class StackBufferDataSource : protected BufferDataSource
{
	public:
		StackBufferDataSource(unsigned int len=0x1000) : BufferDataSource(new char[len], len)
		{
			buf_ptr = const_cast<unsigned char *>(buf)+len;
		};
		~StackBufferDataSource() { 
			delete [] const_cast<unsigned char *>(buf);
		};
		
		//
		// Push values to the stack
		//

		inline void push2(uint16 val)
		{
			buf_ptr-=2;
			buf_ptr[0] =  val     & 0xFF;
			buf_ptr[1] = (val>>8) & 0xFF;
		}
		inline void push4(uint32 val)
		{
			buf_ptr-=4;
			buf_ptr[0] =  val      & 0xFF;
			buf_ptr[1] = (val>>8)  & 0xFF;
			buf_ptr[2] = (val>>16) & 0xFF;
			buf_ptr[3] = (val>>24) & 0xFF;
		}
		// Push an arbitrary number of bytes of 0
		inline void push0(const uint32 size) { 
			buf_ptr -= size;
			std::memset (buf_ptr, 0, size);
		};
		// Push an arbitrary number of bytes
		inline void push(const uint8 *in, const uint32 size) { 
			buf_ptr -= size;
			std::memcpy (buf_ptr, in, size);
		};
		
		//
		// Pop values from the stack
		//

		inline uint16 pop2() { return read2(); }
		inline uint32 pop4() { return read4(); }
		inline void pop(uint8 *out, const uint32 size) { read(reinterpret_cast<char*>(out), size); };
		
		//
		// Access a value from a location in the stacck
		//

		inline uint8 access1(const uint32 offset) const
		{
			return buf[offset];
		}
		inline uint16 access2(const uint32 offset) const
		{
			return (buf[offset] | (buf[offset+1] << 8));
		}
		inline uint32 access4(const uint32 offset) const
		{
			return buf[offset] | (buf[offset+1]<<8) | (buf[offset+2]<<16) | (buf[offset+3]<<24);
		}
		inline const uint8* access(const uint32 offset) const
		{
			return buf+offset;
		}
		
		//
		// Assign a value to a location in the stack
		//

		inline void assign1(const uint32 offset, const uint8 val)
		{
			const_cast<unsigned char *>(buf)[offset]   =  val     & 0xFF;
		}
		inline void assign2(const uint32 offset, const uint16 val)
		{
			const_cast<unsigned char *>(buf)[offset]   =  val     & 0xFF;
			const_cast<unsigned char *>(buf)[offset+1] = (val>>8) & 0xFF;
		}
		inline void assign4(const uint32 offset, const uint32 val)
		{
			const_cast<unsigned char *>(buf)[offset]   =  val      & 0xFF;
			const_cast<unsigned char *>(buf)[offset+1] = (val>>8)  & 0xFF;
			const_cast<unsigned char *>(buf)[offset+2] = (val>>16) & 0xFF;
			const_cast<unsigned char *>(buf)[offset+3] = (val>>24) & 0xFF;
		}
		inline void assign(const uint32 offset, const uint8 *in, const uint32 len)
		{
			std::memcpy (const_cast<unsigned char *>(buf+offset), in, len);
		}

		inline uint32 stacksize() const { return buf+size-buf_ptr; };

		inline void resize(const uint32 newsize) { 
			buf_ptr = const_cast<unsigned char *>(buf)+size-newsize;
		};
		
		inline void addSP(const sint32 offset) {
			skip(offset);
		}

		virtual unsigned int getSP() { return getPos(); };

		inline void moveSP(unsigned int pos) {
			seek(pos);
		}

		/* temp debugging */
		inline std::ostream &print(std::ostream &o, uint32 bp)
		{
			for(const unsigned char *c=buf_ptr; c!=buf+size; ++c) {
				if (c != buf+bp)
					std::printf(" %02X", static_cast<unsigned int>(*c));
				else
					std::printf(":%02X", static_cast<unsigned int>(*c));
			}
			return o;
		}
		
	private:
};

class ExultDataSource: public BufferDataSource {
public:
	ExultDataSource(const char *fname, int index):
		BufferDataSource(0,0)
	{
		U7object obj(fname, index);
		buf = reinterpret_cast<unsigned char*>(obj.retrieve(size));
		buf_ptr = const_cast<unsigned char *>(buf);
	};
	
	~ExultDataSource()
	{
					delete [] const_cast<unsigned char *>(buf);
	}
};

#endif
