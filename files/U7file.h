
#ifndef	_u7file_h_
#define	_u7file_h_

#include <string>
#include <map>

class	U7file
	{
protected:
	string	filename;
public:
	U7file() {};
	U7file(const char *name) : filename(name) {};
	U7file(const U7file &f) : filename(f.filename)
		{  }
	U7file &operator=(const U7file &u) { filename=u.filename; return *this; }
	virtual	int	number_of_objects(const char *)=0;
	virtual	int	retrieve(int objnum,char **,size_t *len)=0; // To a memory block
	virtual	int	retrieve(int objnum,const char *)=0;	// To a file
	virtual	~U7file();
	};

class	U7FileManager
	{
	static	U7FileManager	*self;
protected:
	struct ltstr
	{
	  bool operator()(const string &s1, const string &s2) const
	  {
	    return s1<s2;
	  }
	};
	map<const string,U7file *,ltstr> file_list;
public:
	U7FileManager();
	~U7FileManager();

	U7file	*get_file_object(const string &s);
	static U7FileManager *get_ptr(void);
	};

class	U7object
	{
protected:
	string	filename;
	int	objnumber;
public:
	virtual	int retrieve(char **,size_t &len);	// Retrieve to a memory block
	virtual int retrieve(const char *);		// Retrieve to a filename

	U7object(const char *file,int objnum);
	virtual	~U7object();
	};

#endif
