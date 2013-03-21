/* gdevice.cc -- Implementation of graphic device

Copyright (C) 1998-2013, Brian Bray

*/

#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/tools.h"

#include <X11/Xlib.h>
#include <cstdlib>
#include "gdevice.h"

namespace bw {


/* class GDevice

   Represents a graphic device and keeps track of device specific resources
   such as fonts, colours, palettes, etc.
*/

// Constructor
GDevice::GDevice( Display* pDisplay )
	:  m_pDisplay( pDisplay )
{}


GDevice::~GDevice()
{
	// Unload fonts
	// TODO: Full list
	XFreeFont( m_pDisplay, m_vpfs[0] );
	if (m_vpfs[1]!=m_vpfs[0])
		XFreeFont( m_pDisplay, m_vpfs[1] );
}

XFontStruct* GDevice::getFontStruct( FontIndex findx )
{
	// This is a hacked up version to get things running
	// TODO: Check if this font is loaded yet.  If not, refer to FontPalette
	//	     for details and load the font.
	static bool isFirstCall = true;

	if (isFirstCall) {
		isFirstCall = false;
		m_vpfs[0] = XLoadQueryFont( m_pDisplay, "-*-clean-*-16-*" );
		m_vpfs[1] = XLoadQueryFont( m_pDisplay, "5x7" );
		if (!m_vpfs[1])
			m_vpfs[1] = XLoadQueryFont( m_pDisplay, "fixed" );
		if (!m_vpfs[1])
			exit(1);
		if (!m_vpfs[0])
			m_vpfs[0] = m_vpfs[1];
	}

	return m_vpfs[findx];
}


}   // Namespace bw
