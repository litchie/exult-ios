/*
 * This file implements some Win32 OLE2 specific Drag and Drop structures.
 * Most of the code is a modification of the OLE2 SDK sample programs
 * "drgdrps" and "drgdrpt". If you want to see them, don't search at
 * Microsoft (I didn't find them there, seems they reorganised their servers)
 * but instead at
 *  http://ftp.se.kde.org/pub/vendor/microsoft/Softlib/MSLFILES/
 */
#include <iostream>

#include <glib.h>
#include <gtk.h>

#include "windrag.h"
#include "u7drag.h"

static UINT CF_EXULT = RegisterClipboardFormat("ExultData");

// IDropTarget implementation

Windnd::Windnd(HWND hgwnd,	Drop_shape_handler_fun shapefun,
	Drop_chunk_handler_fun cfun
)
:gamewin(hgwnd), shape_handler(shapefun), chunk_handler(cfun)
{
  m_cRef = 1;
};

Windnd::~Windnd()
{
};

STDMETHODIMP
Windnd::QueryInterface(REFIID iid, void ** ppvObject)
{
    *ppvObject=NULL;
	if (IID_IUnknown==iid || IID_IDropTarget==iid)
	    *ppvObject=this;
	if (NULL==*ppvObject)
	    return E_NOINTERFACE;
	((LPUNKNOWN)*ppvObject)->AddRef();
	return NOERROR;
};

STDMETHODIMP_(ULONG)
Windnd::AddRef(void)
{
	return ++m_cRef;
};

STDMETHODIMP_(ULONG)
Windnd::Release(void)
{
    if (0!=--m_cRef)
	    return m_cRef;
	delete this;
	return 0;
};

STDMETHODIMP
Windnd::DragEnter(IDataObject * pDataObject,
	  DWORD grfKeyState,
	  POINTL pt,
	  DWORD * pdwEffect)
{
    if (!is_valid(pDataObject)) {
	    *pdwEffect = DROPEFFECT_NONE;
	} else {
		*pdwEffect = DROPEFFECT_COPY;
	};
	return S_OK;
};

STDMETHODIMP
Windnd::DragOver(DWORD grfKeyState,
	  POINTL pt,
	  DWORD * pdwEffect)
{
	*pdwEffect = DROPEFFECT_COPY;
	return S_OK;
};

STDMETHODIMP
Windnd::DragLeave(void)
{
	return S_OK;
};

STDMETHODIMP
Windnd::Drop(IDataObject * pDataObject,
	  DWORD grfKeyState,
	  POINTL pt,
	  DWORD * pdwEffect)
{
	*pdwEffect = DROPEFFECT_COPY;
	windragdata data;

	// retrieve the dragged data
	FORMATETC fetc;
	fetc.cfFormat = CF_EXULT;
	fetc.ptd = NULL;
	fetc.dwAspect = DVASPECT_CONTENT;
	fetc.lindex = -1;
	fetc.tymed = TYMED_HGLOBAL;
	STGMEDIUM med;

	pDataObject->GetData(&fetc, &med);
	data = *(windragdata *)GlobalLock(med.hGlobal);
	do_handle_drop(&data);
	GlobalUnlock(med.hGlobal);
	ReleaseStgMedium(&med);

	return S_OK;
};

bool Windnd::is_valid(IDataObject * pDataObject)
{
	FORMATETC fetc;
	fetc.cfFormat = CF_EXULT;
	fetc.ptd = NULL;
	fetc.dwAspect = DVASPECT_CONTENT;
	fetc.lindex = -1;
	fetc.tymed = TYMED_HGLOBAL;

	if (FAILED(pDataObject->QueryGetData(& fetc))) {
	  return false;
	} else {
	  return true;
	}
};

// WIN32 equivalent to "Xdnd::select_msg"
void Windnd::do_handle_drop(windragdata *data)
{
	// get mouse position in game window client area
	LPPOINT pnt = new POINT;
	GetCursorPos(pnt);
	ScreenToClient(gamewin, pnt);

	if (data->id == U7_TARGET_SHAPEID) {
		int file,shape,frame;
		Get_u7_shapeid(data->data, file, shape, frame);
		if (file == U7_SHAPE_SHAPES)
				// For now, just allow "shapes.vga".
			(*shape_handler)(shape, frame, pnt->x, pnt->y);
	} else if (data->id == U7_TARGET_CHUNKID) {
		int chunknum;
		Get_u7_chunkid(data->data, chunknum);
		(*chunk_handler)(chunknum, pnt->x, pnt->y);
	}

	delete pnt;
};

// IDropSource implementation

Windropsource::Windropsource(HBITMAP pdrag_bitmap, int x0, int y0)
:drag_bitmap(pdrag_bitmap)
{
    m_cRef = 1;
	drag_shape = 0;

#if 0
// doesn't work yet
	SIZE bsize;
	GetBitmapDimensionEx(drag_bitmap, &bsize);
	bsize.cx = 32; bsize.cy = 32;
	int shw = bsize.cx, shh = bsize.cy;
	drag_shape = CreateWindowEx(WS_EX_TRANSPARENT,
	  "STATIC",
	  NULL,
	  WS_POPUP, // | SS_OWNERDRAW, /*SS_OWNERDRAW,*/
	  x0, y0,
	  shw, shh,
	  GetForegroundWindow(), // TODO: owner window
	  NULL,
	  (HINSTANCE)GetWindowLong(GetForegroundWindow(), GWL_HINSTANCE),
	  NULL);

	if (FAILED(drag_shape)) {
	  g_warning("Create Window FAILED !");
	};
#endif
};

Windropsource::~Windropsource()
{
	DestroyWindow(drag_shape);
};

STDMETHODIMP
Windropsource::QueryInterface(REFIID iid, void ** ppvObject)
{
    *ppvObject=NULL;
	if (IID_IUnknown==iid || IID_IDropSource==iid)
	    *ppvObject = this;
	if (NULL==*ppvObject)
	    return E_NOINTERFACE;
	((LPUNKNOWN)*ppvObject)->AddRef();
	return NOERROR;
};

STDMETHODIMP_(ULONG)
Windropsource::AddRef(void)
{
	m_cRef = m_cRef + 1;

	return m_cRef;
};

STDMETHODIMP_(ULONG)
Windropsource::Release(void)
{
    if (0!=--m_cRef)
	    return m_cRef;
	delete this;
	return 0;
};

STDMETHODIMP
Windropsource::QueryContinueDrag(
	  BOOL fEscapePressed,
	  DWORD grfKeyState
	)
{
#if 0
// doesn't work yet
	POINT pnt;
	GetCursorPos(&pnt);

	MoveWindow(drag_shape,
	  pnt.x, pnt.y,
	  shw, shh,
	  true);
#endif

    if (fEscapePressed)
        return DRAGDROP_S_CANCEL;
    else if (!(grfKeyState & MK_LBUTTON))
        return DRAGDROP_S_DROP;
    else
        return NOERROR;                  
};

STDMETHODIMP
Windropsource::GiveFeedback(
	  DWORD dwEffect
	)
{
	return DRAGDROP_S_USEDEFAULTCURSORS;
};

// IDataObject implementation

Winstudioobj::Winstudioobj(windragdata pdata)
:data(pdata)
{
    m_cRef = 1;
	drag_image = 0;
};

Winstudioobj::~Winstudioobj()
{
};


STDMETHODIMP
Winstudioobj::QueryInterface(REFIID iid, void ** ppvObject)
{
    *ppvObject=NULL;
	if (IID_IUnknown==iid || IID_IDataObject==iid)
	    *ppvObject=this;
	if (NULL==*ppvObject)
	    return E_NOINTERFACE;
	((LPUNKNOWN)*ppvObject)->AddRef();
	return NOERROR;
};

STDMETHODIMP_(ULONG)
Winstudioobj::AddRef(void)
{
	return ++m_cRef;
};

STDMETHODIMP_(ULONG)
Winstudioobj::Release(void)
{
    if (0!=--m_cRef)
	    return m_cRef;
	delete this;
	return 0;
};

STDMETHODIMP
Winstudioobj::GetData(
	  FORMATETC * pFormatetc,
	  STGMEDIUM * pmedium
	  )
{
	cout << "In GetData"<< endl;

    HGLOBAL hText;
	windragdata *ldata;
    
    pmedium->tymed = 0;
    pmedium->pUnkForRelease = NULL;
    pmedium->hGlobal = NULL;
    
    // This method is called by the drag-drop target to obtain the data
    // that is being dragged.
    if (pFormatetc->cfFormat == CF_EXULT &&
       pFormatetc->dwAspect == DVASPECT_CONTENT &&
       pFormatetc->tymed == TYMED_HGLOBAL)
    {
        hText = GlobalAlloc(GMEM_SHARE | GMEM_ZEROINIT, sizeof(data));
        if (!hText)
            return E_OUTOFMEMORY;

		// This provides us with a pointer to the allocated memory
        ldata = (windragdata *)GlobalLock(hText);
		*ldata = data;
        GlobalUnlock(hText);
        
        pmedium->tymed = TYMED_HGLOBAL;
        pmedium->hGlobal = hText; 
 
        return S_OK;
    }

    return DATA_E_FORMATETC;
};

STDMETHODIMP
Winstudioobj::GetDataHere(
	  FORMATETC * pFormatetc,
	  STGMEDIUM * pmedium
	)
{
	return DATA_E_FORMATETC;
};

STDMETHODIMP
Winstudioobj::QueryGetData(
	  FORMATETC * pFormatetc
	)
{
    // This method is called by the drop target to check whether the source
    // provides data is a format that the target accepts.
    if (pFormatetc->cfFormat == CF_EXULT
        && pFormatetc->dwAspect == DVASPECT_CONTENT
        && pFormatetc->tymed & TYMED_HGLOBAL)
        return S_OK; 
    else return S_FALSE;
};

STDMETHODIMP
Winstudioobj::GetCanonicalFormatEtc(
	  FORMATETC * pFormatetcIn,
	  FORMATETC * pFormatetcOut
	  )
{
    pFormatetcOut->ptd = NULL; 
    return E_NOTIMPL;
};

STDMETHODIMP
Winstudioobj::SetData(
	  FORMATETC * pFormatetc,
	  STGMEDIUM * pmedium,
	  BOOL fRelease
	  )
{
	return E_NOTIMPL;
};

STDMETHODIMP
Winstudioobj::EnumFormatEtc(
	  DWORD dwDirection,
	  IEnumFORMATETC ** ppenumFormatetc
	  )
{
    SCODE sc = S_OK;
    FORMATETC fmtetc;
    
    fmtetc.cfFormat = CF_EXULT;
    fmtetc.dwAspect = DVASPECT_CONTENT;
    fmtetc.tymed = TYMED_HGLOBAL;
    fmtetc.ptd = NULL;
    fmtetc.lindex = -1;

    if (dwDirection == DATADIR_GET){
		cout << "EnumFmt" << endl;
	    // I was too lazy to implement still another OLE2 interface just for something
		//  we don't even use. This function is supposed to be called by a drop target
		//  to find out if it can accept the provided data type. I suppose with
		//  E_NOTIMPL, other drop targets don't care about Exult data.
        /**ppenumFormatetc = OleStdEnumFmtEtc_Create(1, &fmtetc);
        if (*ppenumFormatetc == NULL)
            sc = E_OUTOFMEMORY;*/
			sc = E_NOTIMPL;

    } else if (dwDirection == DATADIR_SET){
        // A data transfer object that is used to transfer data
        //    (either via the clipboard or drag/drop does NOT
        //    accept SetData on ANY format.
        sc = E_NOTIMPL;
        goto error;
    } else {
        sc = E_INVALIDARG;
        goto error;
    }

error:
    return sc;
};

STDMETHODIMP
Winstudioobj::DAdvise(
	  FORMATETC * pFormatetc,
	  DWORD advf,
	  IAdviseSink * pAdvSink,
	  DWORD * pdwConnection
	  )
{
	return OLE_E_ADVISENOTSUPPORTED;
};

STDMETHODIMP
Winstudioobj::DUnadvise(
	  DWORD dwConnection
	  )
{
	return OLE_E_ADVISENOTSUPPORTED;
};

STDMETHODIMP
Winstudioobj::EnumDAdvise(
	  IEnumSTATDATA ** ppenumAdvise
	  )
{
	return OLE_E_ADVISENOTSUPPORTED;
};
