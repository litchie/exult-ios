

#if __GNUG__ >= 2
#  pragma implementation
#endif


#include "Configuration.h"

#include <cstdio>



Configuration::Configuration() :filename("")
{}

Configuration::Configuration(const char *s) : filename("")
{
	filename=s;
	read_config_file(s);
}

Configuration::~Configuration()
{}


string	&Configuration::value(const char *key)
{
	string	s=key;
	return xmltree.reference(s);
}

void	Configuration::value(const char *key,string &s)
{
	s=value(key);
}

void	Configuration::value(const char *key,int &n)
{
	n=0;
	string	s;

	s=value(key);
	n=atoi(s.c_str());
}


extern	void    xmlparse(string &s,size_t &pos,XMLnode *x);

bool	Configuration::read_config_file(const char *n)
{
        char    buf[4096];
        string  sbuf;

	FILE	*fp=fopen(n,"r");
	if(!fp)
		return false;

        while(fgets(buf,sizeof(buf),fp))
                {
                sbuf+=buf;
                }

        size_t  nn=1;
        xmlparse(sbuf,nn,&xmltree);

	fclose(fp);
	return true;
}


