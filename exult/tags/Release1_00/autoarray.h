/*
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

#include <exception>
#include <string>

#include "exult_types.h"

template<class T>
class autoarray
	{
private:
	std::size_t	size_;
	T *data_;
public:
	class range_error : public std::exception
		{
		std::string	what_;
		public:
		 range_error (const std::string& what_arg): what_ (what_arg) { }
		 const char *what(void) const throw () { return what_.c_str(); }
		 virtual ~range_error() throw () { }
		};
	autoarray() : size_(0), data_(0) 
		{  }
	autoarray(std::size_t n) : size_(n),data_(n?new T[n]:0)
		{  }
	T &operator[](sint32 i)	 throw(range_error)
		{
		if(i>=(sint32)size_ || i < 0)
			throw range_error("out of bounds");
		if(data_)
			return data_[i];
		throw range_error("no data");
		}
	~autoarray()
		{
		if(data_)
			delete [] data_;
		}
	autoarray(const autoarray &a) : size_(0),data_(0)
		{
		if(a.data_)
			{
			data_=new T[a.size_];
			memcpy(data_,a.data_,a.size_);
			size_=a.size_;
			}
		}
	autoarray &operator=(const autoarray &a)
		{
		if(data_)
			{
			delete [] data_;
			size_=0;
			}
		if(a.data_)
			{
			data_=new T[a.size_];
			memcpy(data_,a.data_,a.size_);
			size_=a.size_;
			}
		return *this;
		}
	void set_size(std::size_t new_size)
		{
		if(data_)
			{
			delete [] data_;
			}
		data_=new T[new_size];
		size_=new_size;
		}
	};

