#ifndef	__ARGS___H___
#define	__ARGS___H___

// Handy argument processor. I'm certain the implementation could be better
// but it suffices quite well at the moment.

#include <string>
#include <vector>

using namespace std;

class	Args
	{
	struct Opts
		{
		std::string	option;
		bool	*bval;
		std::string  *sval;
		int	*ival;
		unsigned long *uval;

		bool	dbval;
		std::string	dsval;
		int	dival;
		unsigned long duval;
		enum { no_type=0,type_bool,type_string,type_int,type_unsigned } valuetype;
		Opts() :option(""),valuetype(no_type) {};
		~Opts() {};
		};
	vector<Opts> options;
	public:
	Args() {};
	~Args() {};
	void	declare(const char *s,bool *b,bool defval=true);
	void	declare(const char *s,string *b,const char *defval=0);
	void	declare(const char *s,int *b,int defval=0);
	void	declare(const char *s,unsigned long *b,unsigned long defval=0);
	void	process(int argc,char **argv);
	};

#endif
