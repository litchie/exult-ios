
#include <string>
#include <map>

class	U7file
	{
protected:
	string	filename;
public:
	U7file() {};
	U7file(const char *name) : filename(name) {};
	virtual	int	number_of_objects(const char *)=0;
	virtual	int	retrieve(const char *,int objnum,char *,size_t *len)=0; // To a memory block
	virtual	int	retrieve(const char *,int objnum,const char *)=0;	// To a file
	virtual	~U7file();
	};

class	U7FileManager
	{
protected:
	map<const string,U7file *> file_list;
public:
	U7FileManager();
	~U7FileManager();

	U7file	*get_file_object(const string &s);
	};

class	U7object
	{
protected:
	string	filename;
	int	objnumber;
public:
	virtual	int retrieve(char *,size_t len);	// Retrieve to a memory block
	virtual int retrieve(const char *);		// Retrieve to a filename

	U7object(const char *file,int objnum);
	virtual	~U7object();
	};
