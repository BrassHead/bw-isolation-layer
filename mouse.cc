/* mouse.cc -- Implementation of the Mouse mix-in Figure class

Copyright (C) 1997-2013, Brian Bray

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


/*: class Mouse

  This is a mix-in class for Figures that indicates that the Figure wants
  notification of when the mouse is over the Figure and mouse clicks within
  the figure.
*/

/*: Mouse::Mouse()

  Constructor.

  Prototype: Mouse::Mouse()
*/


/*: Mouse::~Mouse()

  Destructor.

  Prototype: Mouse::~Mouse()
*/

/*: Mouse::onMouseEnter()

  This overridable method is called whenever the mouse pointer enters the
  area covered by the Figure.

  The default action is to ignore this event.

  Prototype: Mouse::onMouseEnter()
*/


/*: Mouse::onMouseExit()

  This overridable method is called whenever the mouse pointer leaves the
  area covered by the Figure.

  The default action is to ignore this event.

  Prototype: Mouse::onMouseExit()
*/


/*: Mouse::onMouseButton()

  This overridable method is called whenever a mouse button is pushed
  while the mouse pointer is over the area covered by the Figure.

  The default action is to ignore this event.

  Prototype: Mouse::onMouseButton(int button, bool isPress)
*/


/* Mouse::doMouseEvent()

   Mouse event handling.
*/
void Mouse::doMouseEvent( const Event& ev, EventRet& )
{
	switch( ev.type ) {
	case PreActivate:
		bwassert( m_pfi );
		m_pfi->m_xswa.event_mask |= ButtonPressMask | ButtonReleaseMask;
		m_pfi->m_xswa.event_mask |= EnterWindowMask | LeaveWindowMask;
		break;

	case ButtonPress:
		onMouseButton(ev.xbutton.button, true);
		break;

	case ButtonRelease:
		onMouseButton(ev.xbutton.button, false);
		break;

	case EnterNotify:
		onMouseEnter();
		break;

	case LeaveNotify:
		onMouseExit();
		break;

	default:
		break;
	}
}

}	// namespace bw
