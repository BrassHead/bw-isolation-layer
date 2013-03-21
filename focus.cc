/* focus.cc -- Implementation of the Focus Mix-in for Figures

Copyright (C) 1997-2013, Brian Bray

*/

#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/context.h"
#include "bw/figure.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include "event.h"
#include "ffigure.h"

namespace bw {

/*: class Focus

  This is a mix-in class for Figures that indicates that the Figure accepts
  keyboard input and/or it can be the input focus for the application.
*/

/*: Focus::Focus()

  Constructor.
*/
Focus::Focus()
{}

/*: Focus::~Focus()

   Destructor.
*/
Focus::~Focus()
{}

/*: Focus::takeFocus()

  Causes the Figure to become the active input focus.
*/
void Focus::takeFocus()
{
	bwassert(false);		// TODO
}

/*: Focus::releaseFocus()

  Causes the Figure to cease to be the input focus.  The location for input
  focus after the call is undefined.
*/
void Focus::releaseFocus()
{
	bwassert(false);		// TODO
}

/*: Focus::tabFocus()

  Moves the input focus to the next Figure in the current tab group.

  TODO: How are tab groups defined?
*/
void Focus::tabFocus()
{
	bwassert(false);		// TODO
}

/* Focus::doFocusEvent

   Event handler for the Focus mix-in.
*/
void Focus::doFocusEvent( const Event& ev, EventRet& )
{
	switch( ev.type ) {
	case PreActivate:
		bwassert( m_pfi );
		m_pfi->m_xswa.event_mask |= KeyPressMask | FocusChangeMask;
		break;

	case KeyPress:
		bwassert( m_pfi );
		// TODO: Propagate to parent?  HotKeys?
		{
			const int 	BufferSize=10;
			char 	pszBuffer[BufferSize];

			int nch = XLookupString( (XKeyEvent*)&ev, pszBuffer, BufferSize, 0, 0 );
			for (int i=0; i<nch; i++)
				onCharacter( pszBuffer[i] );
		}
		break;

	case FocusIn:
		// TODO: How to handle parents?
		onGotFocus();
		break;

	case FocusOut:
		// TODO: How to handle parents?
		onLostFocus();
		break;

	default:
		break;
	}
}

}	// namespace bw
