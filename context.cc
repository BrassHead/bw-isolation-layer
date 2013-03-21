/* context.cc -- Implementation for the Context class

Copyright (C) 1997-2013, Brian Bray

*/


//#include <X11/Xlib.h>
//#include <X11/Xutil.h>
#include "bw/context.h"
//#include "bw/exception.h"
//#include "bw/bwassert.h"

namespace bw {

/* class Context

  Drawing Context.  Maintains the clip region for a Figure.

  Right now, this doesn't keep a clip region, it just keeps a dirty flag.
*/

/* Context::Context()

  Constructor
*/
Context::Context()
	:	m_isDirty( false )
{}


Context::~Context()
{}

/*: Context::addClip()

  Adds a rectangle to the clipping region for the context.
*/
void Context::addClip( int x, int y, unsigned int width, unsigned int height )
{
	(void)x,(void)y,(void)width,(void)height; // TODO: Clipping
	m_isDirty = true;
}

void Context::invalidate()
{
	// TODO: Clipping
	m_isDirty = true;
}


/* Context::reset()

   Resets the Context to a stable state after a 'draw' operation.  It sets
   the clipping region to empty.
*/
void Context::reset()
{
	// TODO: Clipping region
	m_isDirty = false;
}


}	//namespace bw


