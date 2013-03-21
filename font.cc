/* font.cc -- Implementation of Font related classes

Copyright (C) 1997-2013, Brian Bray

*/


#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/figure.h"
#include "bw/scene.h"
#include "bw/tools.h"
#include "bw/context.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include "event.h"
#include "ffigure.h"
#include "fcanvas.h"
#include "ffont.h"
#include "gdevice.h"



namespace bw {


/*: class FontMetric

  Represents a Font specialized for a particular device.

  Methods enable the font to be queried for sizes, attributes, and text
  extents.
*/

/*: FontMetric::FontMetric()

  Constructor.

  Parameters are the Canvas for which the font is desired along with parameters
  to identify the font.

  Prototype: FontMetric( const Canvas& cvs, FontIndex findx )
  Prototype: FontMetric( const Figure& fig, FontIndex findx )
*/
FontMetric::FontMetric( const Canvas& cvs, FontIndex findx )
{
	m_pfs = static_cast<CXFontStruct*>(FCanvas::getDevice( cvs )->getFontStruct( findx ));
}

FontMetric::FontMetric( const Figure& fig, FontIndex findx )
{
	m_pfs = static_cast<CXFontStruct*>(FigureImp::getDevice( fig )->getFontStruct( findx ));
}

FontMetric::~FontMetric()
{}


/*: FontMetric::height()

  Returns the height of a bounding box sufficient to render any character in
  the font.
*/
int FontMetric::height()
{
	return m_pfs->ascent + m_pfs->descent;
}


/*: FontMetric::width()

  Returns the width of a bounding box sufficient to render any character
  in the font.  This is the width of the widest character.
*/
int FontMetric::width()
{
	return m_pfs->max_bounds.width;
}



/*: FontMetric::ascent()

  Returns the largest ascent of any character in the font.
*/
int FontMetric::ascent()
{
	return m_pfs->ascent;
}


/*: FontMetric::descent()

  Returns the largest descent of any character in the font.
*/
int FontMetric::descent()
{
	return m_pfs->descent;
}


/*: FontMetric::getTextExtent

  Returns the size required to render a string.

  Fields in the TextExtent indicate the width, height, ascent and descent
  of the rendering.
*/
TextExtent FontMetric::getTextExtent( const String& str )
{
	int direction_return;
	int font_ascent_return;
	int font_descent_return;
	XCharStruct xcs;
	TextExtent tex;

	XTextExtents( m_pfs,
	              str, str.length(),
	              &direction_return,
	              &font_ascent_return, &font_descent_return,
	              &xcs );

	tex.width = xcs.width;
	tex.height = xcs.ascent + xcs.descent;
	tex.ascent = xcs.ascent;
	tex.descent = xcs.descent;

	return tex;
}


/*: FontMetric::getTextLength()

  Returns the length in pixels of the rendering of the specified string.
*/
int FontMetric::getTextLength( const String& str )
{
	return XTextWidth( m_pfs, str, str.length() );
}



int FontMetric::fitString( const String str, int w )
{
	int nChar = str.length();
	int wOver;
	const int maxW = m_pfs->max_bounds.width;
	bwassert( maxW>0 );

	while ( nChar>0 && (wOver=XTextWidth(m_pfs, str, nChar)-w) > 0 ) {
		if (wOver>maxW)
			nChar -= wOver/maxW;
		else
			--nChar;
	}
	if (nChar>0)
		return nChar;
	return 0;
}


/* FontMetric::changeFont()

   Internal routine so that Press can change fonts
*/
void FontMetric::changeFont( Canvas* pcvs, FontIndex findx )
{
	m_pfs = static_cast<CXFontStruct*>(FCanvas::getDevice( *pcvs )->getFontStruct( findx ));
}


}	// Namespace

