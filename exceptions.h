/*
Copyright (C) 2000  Max Horn

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

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <cerrno>
#include <stdexcept>
#include <string>

/*
 *  Base class of all our exceptions, providing a storage for the error message
 */

class	exult_exception : public std::exception {
	std::string  what_;
	int errno_;
public:
	exult_exception (const char *what_arg): what_ (what_arg), errno_(errno) {  }	
	exult_exception (const std::string& what_arg): what_ (what_arg), errno_(errno) {  }
	const char *what(void) const { return what_.c_str(); }
	int get_errno(void) const { return errno_; }
};


/*
 *  Classes which should not be replicatable throw an replication_exception
 */

class replication_exception : public exult_exception
{
public:
	replication_exception (const char *what_arg): exult_exception (what_arg) {  }	
	replication_exception (const std::string& what_arg): exult_exception (what_arg) {  }
};

// Some handy macros which you can use to make a class non-replicable
#define	UNREPLICATABLE_CLASS(NAME)	NAME(const NAME &) { throw replication_exception( #NAME " cannot be replicated"); }; \
					NAME &operator=(const NAME &) { throw replication_exception( #NAME " cannot be replicated"); return *this; }

#define	UNREPLICATABLE_CLASS_I(NAME,INIT)	NAME(const NAME &) : INIT { throw replication_exception( #NAME " cannot be replicated"); }; \
					NAME &operator=(const NAME &) { throw replication_exception( #NAME " cannot be replicated"); return *this; }



/*
 *  File errors
 */

class	file_open_exception : public exult_exception {
	static const std::string  prefix_;
public:
	file_open_exception (const std::string& file): exult_exception("Error opening file "+file) {  }
};

class	file_write_exception : public exult_exception {
	static const std::string  prefix_;
public:
	file_write_exception(const std::string& file): exult_exception("Error writing to file "+file) {  }
};

class	file_read_exception : public exult_exception {
	static const std::string  prefix_;
public:
	file_read_exception(const std::string& file): exult_exception("Error reading from file "+file) {  }
};

class	wrong_file_type_exception : public exult_exception {
public:
	wrong_file_type_exception (const std::string& file, const std::string& type): exult_exception("File "+file+"is not of type "+type) {  }
};


#endif
