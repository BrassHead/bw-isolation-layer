/* brush.cc -- implementation of the Brush drawing tool

Copyright (C) 1997-2013, Brian Bray

*/

#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/tools.h"
#include "bw/figure.h"
#include "bw/context.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "bgc.h"
#include "fcolour.h"
#include "event.h"
#include "ffigure.h"
#include "fcanvas.h"


namespace bw {

/*: class Brush

  Drawing object for lines and filled areas.
*/

/*: Brush::Brush()

  Constructors.

  The parameters are the Canvas to draw on and optionally the colour of the
  pen.  If no colour is specified, the default foreground Text colour will
  be used.

  Prototype: Brush::Brush( Canvas& cvs, const Colour& c)
  Prototype: Brush::Brush( Canvas& cvs, const StyleIndex ic=Text )
*/
Brush::Brush( Canvas& cvs, const Colour& clr )
{
	makeBrush( cvs, clr );
}


Brush::Brush( Canvas& cvs, const StyleIndex ic )
{
	makeBrush( cvs, getSysColour(ic) );
}


void Brush::makeBrush( Canvas& cvs, const Colour& clr )
{
	XGCValues xgcv;
	FCanvas::setGCV( cvs, m_vm, xgcv );
	FColour::setGCV( clr, m_vm, xgcv );
	xgcv.function = GXcopy;
	xgcv.plane_mask = AllPlanes;
	xgcv.line_width = 1;
	xgcv.line_style = LineSolid;
	xgcv.cap_style = CapNotLast;
	xgcv.join_style = JoinMiter;
	m_vm |= (GCFunction | GCPlaneMask | GCLineWidth | GCLineStyle |
	         GCCapStyle | GCJoinStyle);
	m_pbgc = BGC::getPBGC( cvs, m_vm, xgcv );
}



Brush::~Brush()
{}

/*: Brush::fillRectangle()

  Fills the interior of a rectangle.  The parameters specify the upper left
  corner and size of the rectangle.  Only the interior is filled, the sides
  are not drawn.
*/
void Brush::fillRectangle( int x, int y, int w, int h )
{
	bwassert( m_pbgc );
	bwassert( m_pbgc->m_pDisplay );
	bwassert( m_pbgc->m_drw );
	bwassert( m_pbgc->m_gc );
	XFillRectangle( m_pbgc->m_pDisplay, m_pbgc->m_drw, m_pbgc->m_gc,
	                ++x, ++y, --w, --h );
}


} // namespace
