#include <exception>
#include <string>

template<class T>
class autoarray
	{
private:
	size_t	size_;
	T *data_;
public:
	class range_error : public exception
		{
		string	what_;
		public:
		 range_error (const string& what_arg): what_ (what_arg) { }
		 const char *what(void) { return what_.c_str(); }
		};
	autoarray() : size_(0), data_(0) 
		{  }
	autoarray(size_t n) : size_(n),data_(n?new T[n]:0)
		{  }
	T &operator[](size_t i)	 throw(range_error)
		{
		if(i>=size_)
			throw range_error("out of bounds");
		if(data_)
			return data_[i];
		throw range_error("no data");
		}
	~autoarray()
		{
		if(data_)
			delete [] data_;
		}
	autoarray(const autoarray &a) : size_(0),data_(0)
		{
		if(a.data_)
			{
			data_=new T[a.size_];
			memcpy(data_,a.data_,a.size_);
			size_=a.size_;
			}
		}
	autoarray &operator=(const autoarray &a)
		{
		if(data_)
			{
			delete [] data_;
			size_=0;
			}
		if(a.data_)
			{
			data_=new T[a.size_];
			memcpy(data_,a.data_,a.size_);
			size_=a.size_;
			}
		return *this;
		}
	void set_size(size_t new_size)
		{
		if(data_)
			{
			delete [] data_;
			}
		data_=new T[new_size];
		size_=new_size;
		}
	};

