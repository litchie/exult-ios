#include "args.h"
#include <iostream>


void	Args::declare(const char *s,bool *b,bool defval)
{
	string	ss;

	ss=s;

	Opts o;
	o.option=ss;
	o.bval=b;
	o.dbval=defval;
	o.valuetype=Opts::type_bool;
	options.push_back(o);
}

void	Args::declare(const char *s,string *b,const char *defval)
{
	string	ss;

	ss=s;

	Opts o;
	o.option=ss;
	o.sval=b;
	o.dsval=defval?defval:"";
	*o.sval=defval?defval:"";
	o.valuetype=Opts::type_string;
	options.push_back(o);
}

void	Args::declare(const char *s,int *b,int defval)
{
	string	ss;

	ss=s;

	Opts o;
	o.option=ss;
	o.ival=b;
	o.dival=defval;
	*o.ival=defval;
	o.valuetype=Opts::type_int;
	options.push_back(o);
}

void	Args::declare(const char *s,unsigned long *b,unsigned long defval)
{
	string	ss;

	ss=s;

	Opts o;
	o.option=ss;
	o.uval=b;
	o.duval=defval;
	*o.uval=defval;
	o.valuetype=Opts::type_unsigned;
	options.push_back(o);
}

void	Args::process(int argc,char **argv)
{
	for(int i=1;i<argc;i++)
		{
		for(unsigned int j=0;j<options.size();j++)
			{
			switch(options[j].valuetype)
				{
				case Opts::no_type:
					continue;
				case Opts::type_bool:
					if(options[j].option==argv[i])
						*(options[j].bval)=options[j].dbval;
					break;
				case Opts::type_string:
					{
					if(options[j].option==argv[i])
						{
						// We want the _next_ argument
						if(++i>=argc)
							{
							cerr << "Data not specified for argument '" << options[j].option <<"'. Using default." << endl;
							break;
							}
						*(options[j].sval)=argv[i];
						}
					break;
					}
				case Opts::type_int:
					{
//					char buf[64];
					if(options[j].option==argv[i])
						{
						// We want the _next_ argument
						if(++i>=argc)
							{
							cerr << "Data not specified for argument '" << options[j].option <<"'. Using default." << endl;
							break;
							}
						*(options[j].ival)=strtol(argv[i],0,10);
						}
					break;
					}
				case Opts::type_unsigned:
					{
//					char buf[64];
					if(options[j].option==argv[i])
						{
						// We want the _next_ argument
						if(++i>=argc)
							{
							cerr << "Data not specified for argument '" << options[j].option <<"'. Using default." << endl;
							break;
							}
						*(options[j].uval)=strtoul(argv[i],0,10);
						}
					break;
					}
				}
			}
		}

}
