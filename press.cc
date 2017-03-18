/* press.cc -- implementation of the Press drawing tool for text

Copyright (C) 1997-2013, Brian Bray

*/


#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/tools.h"
#include "bw/figure.h"
#include "bw/context.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include "bgc.h"
#include "ffont.h"
#include "fcolour.h"
#include "event.h"
#include "ffigure.h"
#include "fcanvas.h"


namespace bw {

/*: class Press

  Drawing object for text.
*/

/*: Press::Press()

  Constructors.

  The parameters are the Canvas to draw on and optionally the font of the
  text face.  If no font is specified, the default foreground Text font
  will be used.  The text colour is set to the default foreground text
  colour and the text background colour is set to the default background.

  Prototype: Press::Press( Canvas& cvs, FontIndex findx=FaceFont )
*/
Press::Press( Canvas& cvs, FontIndex findx )
	: FontMetric( cvs, findx ),
	  m_pcvs( &cvs )
{
	XGCValues xgcv;
	Colour clrFore = getSysColour( Text );
	Colour clrBack = getSysColour( TextBack );
	FCanvas::setGCV( cvs, m_vm, xgcv );
	FColour::setGCV( clrFore, m_vm, xgcv );
	FColour::setGCVBack( clrBack, m_vm, xgcv );
	xgcv.function = GXcopy;
	xgcv.plane_mask = AllPlanes;
	xgcv.font = m_pfs->fid;
	m_vm |= (GCFunction | GCPlaneMask | GCFont);
	m_pbgc = BGC::getPBGC( cvs, m_vm, xgcv );
}

Press::~Press()
{}

/*: Press::drawText()

  Draws the given string.  Only the pixels within the font are affected
  (no background box is drawn).
*/
void Press::drawText( int x, int y, String str )
{
	bwassert( m_pbgc );
	bwassert( m_pbgc->m_pDisplay );
	bwassert( m_pbgc->m_drw );
	bwassert( m_pbgc->m_gc );
	XDrawString( m_pbgc->m_pDisplay, m_pbgc->m_drw, m_pbgc->m_gc,
	             x, y, str, str.length() );
}

/*: Press::changeFont()

  Changes the font used for printing.
*/
void Press::changeFont( FontIndex findx )
{
	FontMetric::FmChangeFont( m_pcvs, findx );
	XGCValues xgcv;
	xgcv.font = m_pfs->fid;;
	BGC::changeValues( m_pbgc, GCFont, xgcv );
}


}   // namespace
