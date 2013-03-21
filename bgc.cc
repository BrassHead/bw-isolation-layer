/* bgc.cc -- Implementation of the X GC wrapper class

Copyright 1997-2013, Brian Bray

*/

#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/figure.h"
#include "bw/context.h"
#include "bw/tools.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "bgc.h"
#include "event.h"
#include "ffigure.h"
#include "fcanvas.h"
#include "gdevice.h"


namespace bw {

BGC* BGC::getPBGC( const Canvas& cvs, unsigned long vm, XGCValues& xgcv )
{
	// TODO: Ask the device for a Device specific GC.  It should search it's
	// list for a matching GC to re-use and create a new one only if all are
	// used.
	//
	// Right now, I create a new one here every time

	return new BGC( FCanvas::getDevice(cvs)->m_pDisplay, cvs.m_pfi->m_wnd, vm, xgcv );
}

void BGC::changeValues( BGC*& pbgc, unsigned long vm, XGCValues& xgcv )
{
	// TODO: This should ask the device for a new GC if the current one
	// is referenced more than once.  Right now, I just make the change.
	XChangeGC( pbgc->m_pDisplay, pbgc->m_gc, vm, (XGCValues*)&xgcv );
}

BGC::BGC( Display* pDisplay, Drawable drw, unsigned long vm, XGCValues& xgcv )
	: m_nRefs( 1 ),
	  m_pDisplay( pDisplay ),
	  m_drw( drw )
{
	m_gc = XCreateGC( m_pDisplay, m_drw, vm, &xgcv );
	bwassert( m_gc );
}

BGC::~BGC()
{
	XFreeGC( m_pDisplay, m_gc );
}

void BGC::release()
{
	delete this;
}

}   // namespace bw
