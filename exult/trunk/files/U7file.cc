#include "U7file.h"
#include "Flex.h"
#include "IFF.h"
#include "Table.h"
#include <cstdio>
#include <iostream>
#include "utils.h"

using std::string;

using std::cerr;
using std::endl;
using std::size_t;
using std::FILE;
using std::fclose;
using std::fwrite;


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
		throw 0;
	else
		self=this;
}


U7file::~U7file()
{}



U7object::U7object(const char *f,int o)	:	filename(f),objnumber(o) {}
//U7object::U7object(const U7object &u)	:	filename(u.filename),objnumber(u.objnumber) {}

U7object::~U7object()	{}


int	U7object::retrieve(char **buf,size_t &len)
{
	U7file *uf=U7FileManager::get_ptr()->get_file_object(filename);
	if(!uf)
		return 0;
	return uf->retrieve(objnumber,buf,&len);
}

int	U7object::retrieve(const char *fname)
{
	FILE	*fp=U7open(fname,"wb");
	if(!fp)
		return 0;

	char	*n;
	size_t	l;

	if(!retrieve(&n,l))
		{
		fclose(fp);
		return 0;
		}
	fwrite(n,l,1,fp);	// &&&& Should check return value
	fclose(fp);
	delete [] n;
	return !0;
}
