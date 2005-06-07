/*
 *  pent_include.h - Basic Pentagram types, and precompiled header
 *
 *  Copyright (C) 2002, 2003  The Pentagram Team
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

#ifndef	PENT_INCLUDE_H
#define	PENT_INCLUDE_H

#define PENTAGRAM_IN_EXULT

// Include config.h first if we have it
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

//
// Common/base types
//
#include "common_types.h"


//
// p_dynamic_cast support
//
//#include "p_dynamic_cast.h"


//
// assert
//
#include <cassert>


//
// Strings
//
#include "istring.h"



//
// The Console
//

//#include "Console.h"
#include <iostream>
#define pout std::cout
#define perr std::cerr




//
// Debugging
//
#ifdef DEBUG
#  define POUT(x)		do { pout << x << std::endl; pout.flush(); } while (0)
#  define PERR(x)		do { perr << x << std::endl; perr.flush(); } while (0)
#else
#  define POUT(x)		do { } while(0)
#  define PERR(x)		do { } while(0)
#endif

// Two very useful macros that one should use instead of pure delete; they 
// will additionally set the old object pointer to 0, thus helping prevent
// double deletes (note that "delete 0" is a no-op).
#define FORGET_OBJECT(x) do { delete x; x = 0; } while(0)
#define FORGET_ARRAY(x) do { delete [] x; x = 0; } while(0)


//
// Can't happen.
// For things that really can't happen. Or shouldn't anyway.
//
#define CANT_HAPPEN() do { assert(false); } while(0)

//
// Can't happen return.
// If we're guaranteed to return before this, but we want to shut the
// compiler warning up.
//
#define CANT_HAPPEN_RETURN() do { assert(false); return 0; } while(0)

// 
// Can't happen with a message 
//
// Allows a message to be supplied.
// May not work on all compilers or runtimes as expected
//
#define CANT_HAPPEN_MSG(msg) do { assert(msg && false); } while(0)


////////////////////
//                //
// Class Wrappers //
//                //
////////////////////

//
// DataSources
// 

#define IDataSource DataSource
#define ODataSource DataSource

#define IBufferDataSource BufferDataSource
#define OBufferDataSource BufferDataSource

#include "databuf.h"

class IFileDataSource : public StreamDataSource
{
public:
	public:
	IFileDataSource(std::ifstream *data_stream) : StreamDataSource(data_stream)
	{
	}
	~IFileDataSource() 
	{
		FORGET_OBJECT(in);
	}
};

class OFileDataSource : public StreamDataSource
{
public:
	public:
	OFileDataSource(std::ofstream *data_stream) : StreamDataSource(data_stream)
	{
	}
	~OFileDataSource() 
	{
		FORGET_OBJECT(out);
	}
};

//
// FileSystem
//

#include "utils.h"
#include "singles.h"

class FileSystem : protected Game_singletons
{
public:
	static FileSystem* get_instance() { 
		if (!pent_filesys) pent_filesys = new FileSystem;
		return pent_filesys; 
	}

	static IDataSource* ReadFile(const std::string &vfn, bool is_text)
	{
		std::ifstream *f = new std::ifstream();
		if(!U7open(*f, vfn.c_str(), is_text)) {
			delete f;
			return 0;
		}
		return new IFileDataSource(f);
	}

	// Open a streaming file as readable. Streamed (0 on failure)
	static ODataSource* WriteFile(const std::string &vfn, bool is_text)
	{
		std::ofstream *f = new std::ofstream();
		if(!U7open(*f, vfn.c_str(), is_text)) {
			delete f;
			return 0;
		}
		return new OFileDataSource(f);
	}
};

#endif //PENT_INCLUDE_H
