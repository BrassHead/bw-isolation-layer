/* styletools.cc -- implementation of style tools for "native" style controls

Copyright (C) 1997-2013, Brian Bray

*/


#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/tools.h"
#include "bw/styletools.h"

namespace bw {


/*: class Bevel

  Drawing tool for beveled effects, primarily buttons.
*/

/*: Bevel::Bevel()

  Constructor.  Constructs a Bevel drawing tool specialized for the
  given Canvas.
*/
Bevel::Bevel(Canvas& cvs)
	:  m_brFace( cvs, Face ),
	   m_penWhite( cvs, White ),
	   m_penBlack( cvs, Black ),
	   m_penLight( cvs, Light ),
	   m_penDark( cvs, Shadow ),
	   m_width( cvs.width() ),
	   m_height( cvs.height() )
{
}

Bevel::~Bevel()
{
}

/*: Bevel::drawRaised()

  Draws a raised bevel effect bordering the specified rectangle and fills
  the interior with the "Face" colour.
*/
void Bevel::drawRaised( int x, int y, int w, int h )
{
	if (w>4 && h>4 ) {
		// Outer rectangle
		m_penWhite.line( x, y, x+w-1, y );
		m_penWhite.line( x, y+1, x, y+h-1 );
		m_penBlack.line( x, y+h-1, x+w, y+h-1 );
		m_penBlack.line( x+w-1, y, x+w-1, y+h-1 );

		// Inner rectangle
		m_penLight.line( x+1, y+1, x+w-2, y+1 );
		m_penLight.line( x+1, y+2, x+1, y+h-2 );
		m_penDark.line( x+1, y+h-2, x+w-1, y+h-2 );
		m_penDark.line( x+w-2, y+1, x+w-3, y+h-2 );

		// Fill
		m_brFace.fillRectangle( x+1, y+1, w-2, h-2 );
	} else if (w>0 && h>0) {
		// Too small for 3D
		m_brFace.fillRectangle( x-1, y-1, w+2, h+2 );
	}
}


/*: Bevel::drawLowered()

  Draws a lowered bevel effect bordering the specified rectangle and fills
  the interior with the "Face" colour.
*/
void Bevel::drawLowered( int x, int y, int w, int h )
{
	m_penDark.rectangle( x, y, w, h );
	m_brFace.fillRectangle( x, y, w, h );
}


}   // namespace
