/* linetool.cc -- implementation of the general line drawing tool

Copyright (C) 1997-2013, Brian Bray

*/

#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/tools.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "bgc.h"

namespace bw {

/*: class LineTool

  Base class for line Drawing objects.
*/


/*: LineTool::line()

  Draws a line from (x1,y1) to (x2,y2).
*/
void LineTool::line(int x1, int y1, int x2, int y2)
{
	bwassert( m_pbgc );
	bwassert( m_pbgc->m_pDisplay );
	bwassert( m_pbgc->m_drw );
	bwassert( m_pbgc->m_gc );
	XDrawLine( m_pbgc->m_pDisplay, m_pbgc->m_drw, m_pbgc->m_gc,
	           x1, y1, x2, y2 );
}



/*: LineTool::rectangle()

  Draws a rectangle given upper left corner and size.

*/
void LineTool::rectangle( int x, int y, int w, int h )
{
	bwassert( m_pbgc );
	bwassert( m_pbgc->m_pDisplay );
	bwassert( m_pbgc->m_drw );
	bwassert( m_pbgc->m_gc );
	XDrawRectangle( m_pbgc->m_pDisplay, m_pbgc->m_drw, m_pbgc->m_gc,
	                x, y, w-1, h-1 );
}

/*: LineTool::setDashes()

  Sets the tool for drawing dashed lines.  The two parameters give the dash
  on and off lengths in pixels.  If 'off' is 0, then a solid line is drawn.
  This is the default for a new LineTool.
*/
void LineTool::setDashes( int on, int off )
{
	// TODO: Set dash GC Attribute.
}

}   // namespace
