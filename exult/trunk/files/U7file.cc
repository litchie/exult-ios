#include "U7file.h"
#include "Flex.h"
#include "IFF.h"
#include "Table.h"
#include "Flat.h"
#include <cstdio>
#include <iostream>
#include "exceptions.h"
#include "utils.h"

using std::cerr;
using std::endl;
using std::size_t;
using std::FILE;
using std::fclose;
using std::fwrite;
using std::string;


#define	TRY_FILE_TYPE(uf,CLASS_NAME)	\
	if(!uf) \
	try {	\
		uf=new CLASS_NAME(s);	\
		file_list[s]=uf;	\
		return uf;	\
	} catch(...)	\
		{	\
		;	\
		}

U7file  *U7FileManager::get_file_object(const string &s)
{
	U7file	*uf=0;
	if(file_list.count(s))
		{
		return file_list[s];
		}
	// Not in our cache. Attempt to figure it out.
	
	TRY_FILE_TYPE(uf,IFF);
	TRY_FILE_TYPE(uf,Flex);
	TRY_FILE_TYPE(uf,Table);
	TRY_FILE_TYPE(uf,Flat);

	// Failed
	if (!uf)
	{
		std::cerr << "Unable to find/open U7file " << s << endl;
		throw file_not_found_error(s);
	}
	return uf;
}

U7FileManager	*U7FileManager::get_ptr(void)
{
	if(!self)
		new U7FileManager();	// self gets the pointer, so it's okay
					// This might look like it creates a
					// leak, but this is a singleton object
	return self;
}
	

U7FileManager::~U7FileManager() {}

U7FileManager   *U7FileManager::self=0;

U7FileManager::U7FileManager()
{
	if(self)
		throw U7file::exclusive();
	else
		self=this;
}


U7file::~U7file()
{}



U7object::U7object(const char *f,int o)	:	filename(f),objnumber(o) {}
//U7object::U7object(const U7object &u)	:	filename(u.filename),objnumber(u.objnumber) {}

U7object::~U7object()	{}


void	U7object::retrieve(char **buf,size_t &len)
{
	U7file *uf=U7FileManager::get_ptr()->get_file_object(filename);
#if 0
	// This code here can not possible be reached since get_file_object *NEVER* returns NULL
	if(!uf)
		{
		throw U7file::file_error();
		return 0;
		}
#endif
	uf->retrieve(objnumber,buf,&len);
}

void	U7object::retrieve(const char *fname)
{
	FILE	*fp=U7open(fname,"wb");
	if(!fp)
		throw file_not_found_error(fname);

	char	*n;
	size_t	l;

	try
	{
		retrieve(&n,l);
	}
	catch( const std::exception & err )
	{
		fclose(fp);
		throw err;
	}
	fwrite(n,l,1,fp);	// &&&& Should check return value
	fclose(fp);
	delete [] n;
}
