#ifndef INCL_WINDRAG
#define INCL_WINDRAG

#include <ole2.h>

typedef void (*Drop_shape_handler_fun)(int shape, int frame, int x, int y, void *udata);
typedef void (*Drop_chunk_handler_fun)(int chunk, int x, int y, void *udata);

// A usefull structure for Winstudioobj
struct windragdata {
	int id;
	unsigned char data[30];
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

	Drop_shape_handler_fun shape_handler;
	Drop_chunk_handler_fun chunk_handler;
	Drop_shape_handler_fun face_handler;

public:
	Windnd(HWND hgwnd, 	Drop_shape_handler_fun shapefun,
	Drop_chunk_handler_fun cfun);
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
    void do_handle_drop(windragdata *data);

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