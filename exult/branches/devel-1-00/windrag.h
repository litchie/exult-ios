#ifndef INCL_WINDRAG
#define INCL_WINDRAG

#if defined(WIN32) && defined(USE_EXULTSTUDIO)

#include "u7drag.h"
#include <ole2.h>
#include "utils.h"

// A useful structure for Winstudioobj
class windragdata {
	sint32 id;
	uint32 size;			// Size of data
	unsigned char * data;
public:

	inline unsigned char *get_data() {
		return data;
	}
	inline const int get_id() {
		return id;
	}
	inline const int get_size() {
		return size;
	}

	// Default constructor
	inline windragdata() : size(0), data(0) {
	}
	// Copy constructor
	inline windragdata(windragdata &o) : id(o.id), size(o.size), data(new unsigned char [o.size]) {
		std::memcpy(data, o.data, size);
	}
	// Read from buffer
	inline windragdata(unsigned char *buf) :data(0) {
		operator = (buf);
	}
	inline windragdata(sint32 i, uint32 s, unsigned char * d) :
		id(i), size(s), data(new unsigned char [s]) {
		std::memcpy(data, d, size);
	}

	// Destructor
	inline ~windragdata() {
		delete [] data;
	}

	inline void serialize(unsigned char *buf) {
		Write4(buf, id);
		Write4(buf, size);
		std::memcpy(buf, data, size);
	}
	inline windragdata & operator = (unsigned char *buf) {
		delete [] data;

		id = Read4(buf);
		size = Read4(buf);
		data = new unsigned char [size];
		std::memcpy(data, buf, size);
		return *this;
	}
	// Copy constructor
	inline windragdata & operator = (windragdata &o) {
		delete [] data;
		id = o.id;
		size = o.size;
		data = new unsigned char [size];
		std::memcpy(data, o.data, size);
		return *this;
	}
	inline void assign(sint32 i, uint32 s, unsigned char * d) {
		delete [] data;
		id = i;
		size = s;
		data = new unsigned char [s];
		std::memcpy(data, d, size);
	}

};

/*
 * The 'IDropTarget' implementation
 */
class FAR Windnd : public IDropTarget
	{
private:
	HWND gamewin;

	DWORD m_cRef;

	void *udata;

	Move_shape_handler_fun move_shape_handler;
	Move_combo_handler_fun move_combo_handler;
	Drop_shape_handler_fun shape_handler;
	Drop_chunk_handler_fun chunk_handler;
	Drop_shape_handler_fun face_handler;
	Drop_combo_handler_fun combo_handler;

	// Used for when dragging the mouse over the exult window
	int drag_id;
	int prevx, prevy;
	union {
		struct {
			int file,shape,frame;
		} shape;
		struct  {
			int chunknum;
		} chunk;
		struct {
			int xtiles, ytiles;
			int right, below, cnt;
			U7_combo_data *combo;
		} combo;
	} data;

public:
	Windnd(HWND hgwnd, Move_shape_handler_fun, Move_combo_handler_fun,
			Drop_shape_handler_fun, Drop_chunk_handler_fun,
			Drop_combo_handler_fun);
	Windnd(HWND hgwnd, 	Drop_shape_handler_fun shapefun,
	Drop_chunk_handler_fun cfun, Drop_shape_handler_fun facefun, void *d);
	~Windnd();

	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);
	STDMETHOD_(ULONG,AddRef)(void);
	STDMETHOD_(ULONG,Release)(void);

	STDMETHOD(DragEnter)(IDataObject * pDataObject,
	  DWORD grfKeyState,
	  POINTL pt,
	  DWORD * pdwEffect);
	STDMETHOD(DragOver)(DWORD grfKeyState,
	  POINTL pt,
	  DWORD * pdwEffect);
	STDMETHOD(DragLeave)(void);
	STDMETHOD(Drop)(IDataObject * pDataObject,
	  DWORD grfKeyState,
	  POINTL pt,
	  DWORD * pdwEffect);

	bool is_valid(IDataObject * pDataObject);

	static void CreateStudioDropDest(Windnd *& windnd, HWND &hWnd,
					Drop_shape_handler_fun shapefun,
					Drop_chunk_handler_fun cfun,
					Drop_shape_handler_fun facefun,
					void *udata);

	static void DestroyStudioDropDest(Windnd *& windnd, HWND &hWnd);
};

/*
 * The IDropSource implementation
 */

class FAR Windropsource :  public IDropSource
{
private:
    DWORD m_cRef;

	HWND drag_shape;
	HBITMAP drag_bitmap;
	int shw, shh;

public:
	Windropsource(HBITMAP pdrag_bitmap, int x0, int y0);
	~Windropsource();

	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);
	STDMETHOD_(ULONG,AddRef)(void);
	STDMETHOD_(ULONG,Release)(void);

	STDMETHOD(QueryContinueDrag)(BOOL fEscapePressed,DWORD grfKeyState);
	STDMETHOD(GiveFeedback)(DWORD dwEffect);

};

/*
 * The IDataObject implementation
 */
class FAR Winstudioobj :  public IDataObject
{
private:
    DWORD m_cRef;

	HBITMAP drag_image;

	windragdata data;

public:
	Winstudioobj(windragdata pdata);
	~Winstudioobj();

	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);
	STDMETHOD_(ULONG,AddRef)(void);
	STDMETHOD_(ULONG,Release)(void);

	STDMETHOD(GetData)(FORMATETC * pFormatetc, STGMEDIUM * pmedium);
	STDMETHOD(GetDataHere)(FORMATETC * pFormatetc, STGMEDIUM * pmedium);
	STDMETHOD(QueryGetData)(
	  FORMATETC * pFormatetc
	);
	STDMETHOD(GetCanonicalFormatEtc)(
	  FORMATETC * pFormatetcIn,
	  FORMATETC * pFormatetcOut
	  );
	STDMETHOD(SetData)(
	  FORMATETC * pFormatetc,
	  STGMEDIUM * pmedium,
	  BOOL fRelease
	  );
	STDMETHOD(EnumFormatEtc)(
	  DWORD dwDirection,
	  IEnumFORMATETC ** ppenumFormatetc
	  );
	STDMETHOD(DAdvise)(
	  FORMATETC * pFormatetc,
	  DWORD advf,
	  IAdviseSink * pAdvSink,
	  DWORD * pdwConnection
	  );
	STDMETHOD(DUnadvise)(
	  DWORD dwConnection
	  );
	STDMETHOD(EnumDAdvise)(
	  IEnumSTATDATA ** ppenumAdvise
	  );
};

#endif

#endif
