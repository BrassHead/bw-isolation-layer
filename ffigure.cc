/* ffigure.cc -- Implementation of internal helpers classes for Figure

Copyright (C) 1997-2013, Brian Bray

*/

#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/context.h"
#include "bw/exception.h"
#include "bw/figure.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include "ffigure.h"
#include "event.h"

namespace bw {

//
// Statics
//
XContext	FigureImp::m_ctxPWINDOW = 0;

/* class FigureImp

   This class is attached to a Figure class when the figure is "activated".

   It holds the X Windows parameters related to window status and it is the
   location of a variety of internal routines common for activated Figures.
*/

/* FigureImp::FigureImp()

   Constructor.
*/
FigureImp::FigureImp()
	:   m_pDisplay( 0 ),
	    m_pgdev( 0 ),
	    m_pscene( 0 ),
	    m_wnd( 0 ),
	    m_wndParent( 0 ),
	    m_pfigThis( 0 ),
	    m_pfigParent( 0 ),
	    m_pfignParent( 0 ),
	    m_x( 0 ),
	    m_y( 0 ),
	    m_width( 0 ),
	    m_height( 0 )
{
	static bool isFirstCall = true;
	if (isFirstCall) {
		FigureImp::m_ctxPWINDOW = XUniqueContext();

		isFirstCall = false;
	}
}

/* FigureImp::~FigureImp()

   Destructor
*/
FigureImp::~FigureImp()
{
	// TODO: Final verification that everything is cleaned up.
}

/* FigureImp::findFigure()

   This routine does a reverse lookup from a Figure from an X window.

   0 is returned if no Figure is attached.
*/
Figure* FigureImp::findFigure( Display* pDisplay, Window wnd )
{
	Figure* pfig;
	if (XFindContext(pDisplay,
	                 wnd,
	                 m_ctxPWINDOW,
	                 (char**)&pfig )) {
		pfig = 0;
	}
	return pfig;
}

/* FigureImp::setFigure()

   Set reverse pointer used by findFigure.
*/
void FigureImp::setFigure()
{
	if (XSaveContext( m_pDisplay, m_wnd, m_ctxPWINDOW, (char*)m_pfigThis ))
		throw BGUIException( BGUIException::SystemError, "Cannot set Context." );
}

/* FigureImp::unsetFigure()

   Removes reverse pointer used by findFigure.
*/
void FigureImp::unsetFigure() throw()
{
	XDeleteContext( m_pDisplay, m_wnd, m_ctxPWINDOW );
}

}   // namespace








