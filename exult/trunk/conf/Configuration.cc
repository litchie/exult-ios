

#if __GNUG__ >= 2
#  pragma implementation
#endif


#include "Configuration.h"

#include <cstdio>



Configuration::Configuration() :filename("")
{}

Configuration::Configuration(const char *s) : filename("")
{
	read_config_file(s);
}

Configuration::~Configuration()
{}


string	&Configuration::value(const char *key,bool &exists)
{
	string	s=key;
	exists=false;
	return xmltree.reference(s,exists);
}

void	Configuration::value(const char *key,string &s,const char *defaultvalue)
{
	bool	exists;
	s=value(key,exists);
	if(!exists)
		s=defaultvalue;
}

void	Configuration::value(const char *key,int &n,int defaultvalue)
{
	n=0;
	string	s;
	bool	exists;

	s=value(key,exists);
	if(!exists)
		n=defaultvalue;
	else
		n=atoi(s.c_str());
}

// This function does not make sense here. It should be in XMLEntity
static	void	xmlassign(XMLnode *walk,string &key,string &value)
{
		cout << "xmlassign(" << key << "," << value << ")"<<endl;
		if(key.find('/')==string::npos)
			{
			// Must refer to me.
			if(walk->entity.id==key)
				{
				cout << "self-assign" << endl;
				walk->entity.content=value;
				return;
				}
			else
				{
				cerr << "Walking the XML tree failed to create a final node." << endl;
				return;
				}
		}
		string k;
		k=key.substr(key.find('/')+1);
		string k2=k.substr(0,k.find('/'));
		for(vector<XMLnode>::iterator it=walk->nodelist.begin();
			it!=walk->nodelist.end();++it)
			{
			if(it->entity.id==k2)
				{
				xmlassign(it,k,value);
				return;
				}
			}
		XMLnode t;
		t.entity.id=k2;
		walk->nodelist.push_back(t);
		cout << "New node " << k2 << endl;
		vector<XMLnode>::reverse_iterator rit=walk->nodelist.rbegin();
		xmlassign(&*rit,k,value);
		return;


}

void	Configuration::set(string &key,string &value,bool write_out)
{
	string	k=key;
	XMLnode *walk;

	walk=&xmltree;

	// Break k up into '/' separated elements.
	// start advancing walk, one element at a time, creating nodes
	// as needed.
	// At the end of that, walk is the target node, and we
	// can set the value.

	// We must also properly encode the value before writing it out.
	// Must remember that.
	xmlassign(walk,k,value);
	if(write_out)
		write_back();
}

void	Configuration::set(const char *key,const char *value,bool write_out)
{
	string	k(key),v(value);
	set(k,v,write_out);
}

void	Configuration::set(const char *key,const string &value,bool write_out)
{
	string	k(key),v(value);
	set(k,v,write_out);
}



extern	void    xmlparse(string &s,size_t &pos,XMLnode *x);

bool	Configuration::read_config_file(const char *n)
{
        char    buf[4096];
        string  sbuf;

#ifdef XWIN
	const char *f1=getenv("HOME");
	if(f1)
		{
		// User has a home directory
		filename=f1;
		filename+="/.";
		filename+=n;
		}
	else
		filename="n";
#endif
#ifdef WIN32
	// Probably something to do with deteriming the username
	// and generating a filename in their personal setup area.
#endif
	FILE	*fp=fopen(filename.c_str(),"r");
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


string	Configuration::dump(void)
{
	extern	void xmldump(string &,XMLnode *,int);
	string	out("");
	xmldump(out,&xmltree,0);
	return out;
}


void	Configuration::write_back(void)
{
	string	s=dump();
	FILE *fp=fopen(filename.c_str(),"w");
	if(!fp)
		{
		perror("Failed to write configuration file");
		return;
		}
	fwrite(s.c_str(),s.size(),1,fp);
	fclose(fp);
}


