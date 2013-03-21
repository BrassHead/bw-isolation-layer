/* canvas.cc -- Implementation of a drawing surface

Copyright (C) 1997-2013, Brian Bray

*/


#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/context.h"
#include "bw/tools.h"
#include "bw/figure.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "event.h"
#include "ffigure.h"
#include "fcanvas.h"


namespace bw {

/*: class Canvas

  The canvas is a drawing surface upon which drawing tools can operate.

  Implementation:  The Canvas, Drawing Tools, and Paint each manage
  elements of a GC as follows:
  <PRE>
  function		Paint
  plane_mask		Paint
  foreground		Paint
  background		Paint, Press, Pen, Brush
  line_width		Pen, Brush
  line_style		Brush
  cap_style		Pen, Brush
  join_style		Brush
  fill_style		Brush
  fill_rule		Brush
  arc_mode		Brush
  tile			Paint
  stipple		Paint
  ts_x_origin		Canvas
  ts_y_origin		Canvas
  font			Press
  subwindow_mode	Canvas
  graphics_exposures	???
  clip_x_origin		Canvas
  clip_y_origin		Canvas
  clip_mask		Canvas
  dash_offset		Pen, Brush
  dashes		Pen, Brush
  </PRE>

  GCs are allocated from a pool maintained by the Scene.  They are assigned
  to each Canvas and to frequently used Paints.  Client code combines
  a canvas, a drawing tool, and a Paint when constructing a tool.  The GC
  assigned to the Paint will be used if the Canvas has
  no clipping area defined.  Otherwise, the Canvas GC will be reused by all
  tools.

  GC attributes are not preserved.  Each tool sets the attributes it needs
  on each use.  Paints tend to set more attributes than Tools.

  Associated with each Figure is a Context object that holds the
  clip Region.
*/

/* Canvas::Canvas()

   Constructor.

   The single parameter is a Context object (internal) holding the clipping
   region, display, and window parameters.
*/
Canvas::Canvas(FigureImp* pfi)
	: m_pfi( pfi )
{
}

Canvas::~Canvas()
{}

// Unimplemented functions

void FCanvas::setGCV( const Canvas& cvs, unsigned long& vm, XGCValues& xgcv )
{
	// TODO: Set clipping region
	// Possibly affects:  ts_[xy]_origin, subwindow_mode, clip_[xy]_origin,
	// clip_mask

	// Nothing to do if no clipping.
}

/*: Canvas::getScene()

  Returns a reference to the Scene object that is the basis for this
  Canvas.
*/
Scene& Canvas::getScene() const
{
	return *(m_pfi->m_pscene);
}


// Canvas size
unsigned int Canvas::width() const
{
	return m_pfi->m_width;
}


unsigned int Canvas::height() const
{
	return m_pfi->m_height;
}

}   // namespace



