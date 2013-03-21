/* frame.cc -- Implementation of Frame mix-in for Figure class

Copyright (C) 1997-2013, Brian Bray

*/


#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/exception.h"
#include "bw/process.h"
#include "bw/figure.h"
#include "bw/context.h"
#include "bw/scene.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include "event.h"
#include "ffigure.h"
#include "fscene.h"

namespace bw {

/*: Frame::Frame

  Constructor
*/
Frame::Frame()
{}

/*: Frame::~Frame()

  Destructor
*/
Frame::~Frame()
{}

/*: Frame::setTitle()

  Sets the title to be displayed on the Frame's adornments and the icon
  (if any).
*/
void Frame::setTitle( String strTitle )
{
	m_strTitle = strTitle;

	if (getpFigure()->isActive()) {
		bwassert( m_pfi );
		bwassert( m_pfi->m_wnd );

		XStoreName( m_pfi->m_pDisplay, m_pfi->m_wnd, m_strTitle );
		XSetIconName( m_pfi->m_pDisplay, m_pfi->m_wnd, m_strTitle );
	}
}


/*: Frame::onClose()

  Overidable interface to Frame Figures.

  This routine is called when the user has selected the "close" or "quit" or
  whatever button, menu, and/or key from their host OS.

  The correct response is to cleanly exit.  Additional dialog boxes can be
  raised to query the user about saving data, etc.

  The default action is 'doAction("exit(0)")'
*/
void Frame::onClose()
{
	// Note: I can't actually do a 'doAction("quit")' because that
	// function is not on the base class.
	getpFigure()->doAction("exit(0)");
}

/* Frame::doFrameEvent()

   This is the event handler for event of interest to frames.
*/
void Frame::doFrameEvent( const Event& ev, EventRet& )
{
	// This needs to hande the close event and any window setup related to
	// interfacing with a window manager.  A lot of this code is probably
	// in BFrameWindow
	switch (ev.type) {
	case PreActivate:
		bwassert( m_pfi );
		m_pfi->m_xswa.event_mask |= KeyPressMask;
		break;

	case PostActivate: {
		//
		// Set window manager "hints" since this is a frame window
		//

		// Window name
		XTextProperty tpName;
		const char *pszName = m_strTitle;
		XStringListToTextProperty((char**)&pszName, 1, &tpName);

		// Icon name
		XTextProperty tpIconName;
		const char *pszIconName = Process::getAppName();
		XStringListToTextProperty((char**)&pszIconName, 1, &tpIconName);

		// Size Hints
		XSizeHints*	psh = XAllocSizeHints();
		psh->x = m_pfi->m_x;
		psh->y = m_pfi->m_y;
		psh->width = m_pfi->m_width;
		psh->height = m_pfi->m_height;
		psh->flags = PSize | PPosition;

		// Window Manager Hints
		XWMHints*  pwmh = XAllocWMHints();
		pwmh->input = True;
		pwmh->initial_state = NormalState;
		// TODO: Setting the ICON
		pwmh->flags = StateHint | InputHint;

		// Class Hints
		XClassHint* pclh = XAllocClassHint();;
		pclh->res_name = (char*)(const char*)Process::getAppName();
		pclh->res_class = (char*)(const char*)Process::getAppClass();

		XSetWMProperties(
		    m_pfi->m_pDisplay,
		    m_pfi->m_wnd,
		    &tpName,
		    &tpIconName,
		    Process::getArgv(), Process::getArgc(),
		    psh,
		    pwmh,
		    pclh );
		// TODO: The argv,argc options which set the command should really
		//       be NULL here and set by the Main mix-in.  Right now,
		//	 multiple Frame windows will confuse the Session manager.

		XFree( tpName.value );
		XFree( tpIconName.value );
		XFree( pclh );
		XFree( pwmh );
		XFree( psh );

		//
		// Participate in some WM protocols
		//
		Atom atoms[2];
		atoms[0] = FScene::atomsOf(m_pfi->m_pscene).m_atomDelete;
		atoms[1] = FScene::atomsOf(m_pfi->m_pscene).m_atomFocus;
		bwassert( atoms[0] );
		bwassert( atoms[1] );

		if (!XSetWMProtocols( m_pfi->m_pDisplay, m_pfi->m_wnd, atoms, 2 ))
			throw BGUIException( BGUIException::SystemError, "Cannot set WM Protocols." );
		break;
	}

	case ClientMessage:
		if ( ev.xclient.message_type==FScene::atomsOf(m_pfi->m_pscene).m_atomProtocol
		        && ev.xclient.format==32 ) {
			if ((Atom)ev.xclient.data.l[0]==FScene::atomsOf(m_pfi->m_pscene).m_atomDelete) {
				onClose();
			} else if ((Atom)ev.xclient.data.l[0]==FScene::atomsOf(m_pfi->m_pscene).m_atomFocus) {
				bwassert( m_pfi );
				bwassert( m_pfi->m_wnd );
				// TODO: set focus to appropiate subwindow
				XSetInputFocus( m_pfi->m_pDisplay,
				                m_pfi->m_wnd,
				                RevertToParent,
				                ev.xclient.data.l[1] );
			}
		}
		break;

	case KeyPress:
		// TODO: check menu's and hotkeys
		break;

	default:
		break;
	}
}

}   // namespace
