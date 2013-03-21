/* size.cc -- Implementation of the Size mix-in Figure class

Copyright (C) 1999-2013, Brian Bray

*/

#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/figure.h"
#include "bw/context.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include "event.h"
#include "ffigure.h"

namespace bw {

/*: class Size

  This is a mix-in class for Figures that indicates that the Figure wants
  notification of size and location changes or wants to request size and
  location changes.
*/

/*: Size::Size()

  Constructor.

  Prototype: Size::Size()
*/


/*: Size::~Size()

  Destructor.

  Prototype: Size::~Size()
*/


/*: Size::requestResize()

  Request the parent figure to resize this one.  The request is in logical
  terms and can be relative to the parent size or a request to call
  onQuerySize dynamically.
*/
void Size::requestResize( LogPos lposWidth, LogPos lposHeight )
{
	Event ev;
	EventRet evr;
	ev.type = SizeRequest;
	ev.evSizeReq.pfig = getpFigure();
	ev.evSizeReq.lposWidth = lposWidth;
	ev.evSizeReq.lposHeight = lposHeight;
	FigureImp::doEvent( m_pParent, ev, evr );
}

/*: Size::requestMove()

  Request the parent figure to move this one.  The request is in logical
  terms and can be relative to the parent size or a request to call
  onQueryLocation dynamically.
*/
void Size::requestMove( LogPos lposX, LogPos lposY )
{
	Event ev;
	EventRet evr;
	ev.type = MoveRequest;
	ev.evSizeReq.pfig = getpFigure();
	ev.evSizeReq.lposX = lposX;
	ev.evSizeReq.lposY = lposY;
	FigureImp::doEvent( m_pParent, ev, evr );
}


/*: Size::requestRelocate()

  Request the parent figure to move and resize this one.  The request
  is in logical terms and can be relative to the parent size or can be
  a request to call onQuerySize and onQueryLocation dynamically.
*/
void Size::requestRelocate(
    LogPos lposX, LogPos lposY,
    LogPos lposWidth, LogPos lposHeight )
{
	Event ev;
	EventRet evr;
	ev.type = RelocateRequest;
	ev.evSizeReq.pfig = getpFigure();
	ev.evSizeReq.lposX = lposX;
	ev.evSizeReq.lposY = lposY;
	ev.evSizeReq.lposWidth = lposWidth;
	ev.evSizeReq.lposHeight = lposHeight;
	FigureImp::doEvent( m_pParent, ev, evr );
}


/* Size::doSizeEvent()

   Size event handling.
*/
void Size::doSizeEvent( const Event& ev, EventRet& )
{
	switch( ev.type ) {
	case SizeNotify:
		bwassert( m_pfi );
		onResize();
		break;

	case MoveNotify:
		onMove();
		break;

	case RelocateNotify:
		onRelocate();
		break;

	default:
		break;
	}
}

}	// namespace bw
