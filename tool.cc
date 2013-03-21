/* tool.cc -- implementation of common drawing tool methods

Copyright (C) 1997-2013, Brian Bray

*/

#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/tools.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "bgc.h"
#include "fcolour.h"

namespace bw {

/*: class Tool

  Base class for Drawing tool objects.  Contains common routines.
*/

Tool::Tool()
	:  m_vm( 0 ),
	   m_pbgc( 0 )
{}

Tool::~Tool()
{
	if (m_pbgc)
		m_pbgc->release();
}


/*: Tool::setColour()

  Changes the tool colour.
*/
void Tool::setColour( const Colour& clr )
{
	unsigned long vm = 0;
	XGCValues xgcv;
	FColour::setGCV( clr, vm, xgcv );
	BGC::changeValues( m_pbgc, vm, xgcv );
	m_vm |= vm;
}

}   // namespace
