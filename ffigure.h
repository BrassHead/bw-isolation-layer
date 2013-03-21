/* ffigure.h -- Definition of internal classes for Figure implementation

Copyright (C) 1997-2013, Brian Bray

*/


/*
#include "bw/figure.h"
#include "bw/context.h"
#include <X11/Xlib.h>
#include "event.h"
*/

//typedef Display;		// Can't use these without g++ warnings
//typedef Window;

namespace bw {

class Scene;
class GDevice;

// This class holds all the extra information needed when a figure is "active"
// It also holds internal routines used by Figure and it's mix-ins.
class FigureImp {
public:
	FigureImp();
	~FigureImp();

	static void doEvent( Figure* pfig, const Event& ev, EventRet& evr ) {
		bwassert(pfig);
		pfig->doEvent( ev, evr );
	}
	static void doEvent( Parent* pfigb, const Event& ev, EventRet& evr ) {
		bwassert(pfigb);
		pfigb->getpFigure()->doEvent( ev, evr );
	}
	void doEvent( const Event& ev, EventRet& evr ) {
		bwassert(m_pfigThis);
		m_pfigThis->doEvent( ev, evr );
	}

	static Figure* findFigure( Display* pDisplay, Window wnd );
	void setFigure();
	void unsetFigure() throw();

	static void setParent( Figure* pfig, Parent* pParent ) {
		bwassert(pfig);
		pfig->m_pParent = pParent;
	}

	static GDevice* getDevice( const Figure& fig ) {
		return fig.m_pfi->m_pgdev;
	}

	static XContext	m_ctxPWINDOW;

public:
	Display*		m_pDisplay;
	GDevice*		m_pgdev;
	Scene*		m_pscene;
	Window		m_wnd;
	Context		m_ctx;
	Window      	m_wndParent;
	Figure*		m_pfigThis;
	Figure*		m_pfigParent;
	FigureNode* 	m_pfignParent;
	XSetWindowAttributes m_xswa;
	int			m_x;
	int			m_y;
//    unsigned int	m_width;
//    unsigned int        m_height;
	int			m_width;
	int 	        m_height;
};

// This class forms a linked list of child Figures along with their locations
class FigureNode {
public:
	Figure*		m_pfigParent;
	Figure*		m_pfigChild;
	LogPos		m_lposX;
	LogPos		m_lposY;
	LogPos		m_lposWidth;
	LogPos		m_lposHeight;
	FigureNode*		m_pfignNext;
	FigureNode*		m_pfignPrev;
};


}   // namespace





