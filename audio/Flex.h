#ifndef	__Flex_h_
#define	__Flex_h_

#include <vector>
#include <string>

typedef	unsigned long uint32;

struct	Flex
	{
	string	filename;
	char	title[0x50];
	uint32	magic1;
	uint32	count;
	uint32	magic2;
	uint32	padding[8];
	struct Reference
		{
		uint32 offset;
		uint32 size;
		Reference() : offset(0),size(0) {};
		};
	vector<Reference> object_list;
	char *read_object(int objnum,uint32 &length);
	};

extern Flex AccessFlexFile(const char *);

#endif
