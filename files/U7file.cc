#include "U7file.h"
#include "Flex.h"
#include "Table.h"

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
	U7file	*uf;
	if(file_list.count(s))
		{
		// Found it in our cache. Return it
		return file_list[s];
		}
	// Not in our cache. Attempt to figure it out.
	
	TRY_FILE_TYPE(uf,Flex);
	TRY_FILE_TYPE(uf,Table);

	return uf;
}

