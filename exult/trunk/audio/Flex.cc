#include "Flex.h"

#include <cstdio>
#include <iostream>

Flex	AccessFlexFile(const char *name)
{
	Flex	ret;
	FILE	*fp;
	ret.filename=name;
	fp=fopen(name,"rb");
	if(!fp)
		{
		return ret;
		}
	fread(ret.title,0x50,1,fp);
	fread(&ret.magic1,sizeof(uint32),1,fp);
	fread(&ret.count,sizeof(uint32),1,fp);
	fread(&ret.magic2,sizeof(uint32),1,fp);
	for(int i=0;i<8;i++)
		fread(&ret.padding[i],sizeof(uint32),1,fp);
	cout << "Title: " << ret.title << endl;
	cout << "Count: " << ret.count << endl;

	fseek(fp,128,SEEK_SET);
	for(unsigned int i=0;i<ret.count;i++)
		{
		Flex::Reference f;
		fread(&f.offset,sizeof(uint32),1,fp);
		fread(&f.size,sizeof(uint32),1,fp);
		cout << "Item " << i << ": " << f.size << " bytes @ " << f.offset << endl;
		ret.object_list.push_back(f);
		}
	fclose(fp);
	return ret;
}

char	*Flex::read_object(int objnum,uint32 &length)
{
	if((unsigned)objnum>=object_list.size())
		{
		cerr << "objnum too large in read_object()" << endl;
		return 0;
		}
	FILE	*fp=fopen(filename.c_str(),"rb");
	if(!fp)
		{
		cerr << "File open failed in read_object" << endl;
		return 0;
		}
	fseek(fp,object_list[objnum].offset,SEEK_SET);
	length=object_list[objnum].size;
	char	*ret=new char[length];
	fread(ret,length,1,fp);
	fclose(fp);
	return ret;
}
