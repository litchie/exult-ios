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

#include <exception>
#include <string>

/*
 * Base class of all our exceptions, providing a storage for the error message
 */

class	exult_exception : public std::exception {
	std::string  what_;
public:
	exult_exception (const char *what_arg): what_ (what_arg) {  }	
	exult_exception (const std::string& what_arg): what_ (what_arg) {  }
	const char *what(void) { return what_.c_str(); }
};


/*
 * Classes which should not be replicatable throw an replication_exception
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
 * File errors
 */

class	file_not_found_error : public exult_exception {
	static const std::string  prefix_;
public:
	file_not_found_error (const std::string& what_arg): exult_exception("Unable to find/open U7file "+what_arg) {  }
};

class	wrong_file_type_error : public exult_exception {
public:
	wrong_file_type_error (): exult_exception("Wrong file type") {  }
};


#endif
