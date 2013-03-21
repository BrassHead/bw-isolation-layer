/* pen.cc -- implementation of the Pen drawing tool

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
#include "fcolour.h"
#include "event.h"
#include "ffigure.h"
#include "fcanvas.h"

namespace bw {

/*: class Pen

  Drawing object for narrow lines with solid colours.
*/

/*: Pen::Pen()

  Constructors.

  The parameters are the Canvas to draw on and optionally the colour of the
  pen.  If no colour is specified, the default foreground Text colour will
  be used.

  Prototype: Pen::Pen( Canvas& cvs, const Colour& c)
  Prototype: Pen::Pen( Canvas& cvs, const StyleIndex ic=Text )
*/
Pen::Pen( Canvas& cvs, const Colour& clr )
{
	makePen( cvs, clr );
}


Pen::Pen( Canvas& cvs, const StyleIndex ic )
{
	makePen( cvs, getSysColour(ic) );
}


void Pen::makePen( Canvas& cvs, const Colour& clr )
{
	XGCValues xgcv;
	FCanvas::setGCV( cvs, m_vm, xgcv );
	FColour::setGCV( clr, m_vm, xgcv );
	xgcv.function = GXcopy;
	xgcv.plane_mask = AllPlanes;
	xgcv.line_width = 0;
	xgcv.line_style = LineSolid;
	xgcv.cap_style = CapNotLast;
	xgcv.join_style = JoinMiter;
	m_vm |= (GCFunction | GCPlaneMask | GCLineWidth | GCLineStyle |
	         GCCapStyle | GCJoinStyle);
	m_pbgc = BGC::getPBGC( cvs, m_vm, xgcv );
}


Pen::~Pen()
{}


}   // namespace
