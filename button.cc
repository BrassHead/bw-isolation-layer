/* button.cc -- Implementation of the Button Figure

Copyright (C) 1997-2013, Brian Bray

*/

#define NOTRACE
#include "bw/trace.h"

#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/figure.h"
#include "bw/button.h"
#include "bw/tools.h"
#include "bw/styletools.h"

namespace bw {

// Compile time options
//
// onQuerySize returns the size needed to "comfortably" fit the button text.
// These parameters determine the level of "comfort"
//
static const int nVerticalPad = 2;
static const int nHorizontalPad = 20;
static const int nHorizontalMin = 50;


///////////////////////////////////////////////////////////////////////////
/*: class Button

  This is a basic button Figure.  It is written as a true BW figure without
  using a native control.

  Button is a subclass of <A HREF=Figure.html>Figure</A>,
  <A HREF=Focus.html>Focus</A>, and <A HREF=Mouse.html>Mouse</A> and defines
  event handling functions declared in these base classes.
*/


///////////////////////////////////////////////////////////////////////////
/*: Button::Button()

  Constructor.  The arguments give a constant label for the button and
  indicate the action that will be signalled if the button is pushed.
*/
Button::Button( const char* pszLabel, const char* pszAction )
	:   m_strLabel( pszLabel ),
	    m_strAction( pszAction ),
	    m_state( Inactive ),
	    m_isDefault( false ),
	    m_hasFocus( false )
{
	// Intentionally empty
}


///////////////////////////////////////////////////////////////////////////
/*: Button::~Button()

  Destructor.
*/
Button::~Button()
{}


///////////////////////////////////////////////////////////////////////////
/*: Button::makeDefault()

  Makes this button the default button in a group.

  Prototype: void Button::makeDefault( bool isDefault=true )
*/
void Button::makeDefault( bool isDefault )
{
	if (m_isDefault != isDefault)
		invalidate();
	m_isDefault = isDefault;
}


///////////////////////////////////////////////////////////////////////////
/*: Button::onQuerySize()

  Returns the minimum size button that will confortably fit the text.
*/
void Button::onQuerySize( LogPos& lposWidth, LogPos& lposHeight )
{
	FontMetric fm(*this,FaceFont);
	TextExtent tex = fm.getTextExtent(m_strLabel);
	lposHeight = fm.height() + Bevel::borderWidth()*2 + 2 + nVerticalPad;
	lposWidth = tex.width + Bevel::borderWidth()*2 + nHorizontalPad;
	if (lposWidth<nHorizontalMin)
		lposWidth = nHorizontalMin;
}


///////////////////////////////////////////////////////////////////////////
/*: Button::draw()

  Draws button.  This buttons style has quite generic 3D look.  Focus is
  indicated by a dashed line just inside the button face.  The standard
  style colours (Face, ButtonFace, etc) are used.  The default button is
  indicated with a black border.
*/
void Button::draw( Canvas& cvs )
{
	Bevel bvl(cvs);

	int w = cvs.width();
	int h = cvs.height();

	// wBorder is the full border width including:
	//  1 pixel for the default indicator
	//	Several pixels for the Bevel
	//	2 pixels for the Focus indicator
	int wBorder = Bevel::borderWidth() + 3;

	// Max text size
	int maxW = w - 2*wBorder;
	int maxH = h - 2*wBorder - 1;

	//bwassert( maxW>0 );		// TODO: Handle this case cleanly
	//bwassert( maxH>0 );

	Pen penBlack(cvs, Black);

	if (m_isDefault) {
		penBlack.rectangle( 0, 0, w, h );
	} else {
		Brush brBack(cvs, ControlSurface);
		brBack.rectangle( 0, 0, w, h );
	}

	String strLabel = m_strLabel;
	Press prsLabel(cvs, FaceFont);
	TextExtent tex=prsLabel.getTextExtent(strLabel);

	// Pick font size
	if (tex.height>maxH) {
		// Font too big, try a smaller one
		prsLabel.changeFont( SmallFaceFont );
		tex = prsLabel.getTextExtent(strLabel);
		if (tex.height>maxH)
			strLabel = "";		// I assume that "..." is not too high
	}

	// cut label to fit
	if (tex.width>maxW || tex.height>maxH) {
		int lDots = prsLabel.getTextLength("...");
		int nChar = prsLabel.fitString( strLabel, maxW-lDots );
		strLabel = strLabel.substring(0,nChar) + "...";
		// I need to do it again in case the "..." is itself too long
		nChar = prsLabel.fitString( strLabel, maxW );
		strLabel = strLabel.substring(0,nChar);
		tex = prsLabel.getTextExtent( strLabel );
	}

	int xt = (w-tex.width)/2;
	int yt = (h-tex.height)/2 + tex.ascent;

	switch (m_state) {
	case Released:
	case Inactive:
		bvl.drawRaised( 1, 1, w-2, h-2);
		prsLabel.drawText( xt, yt, strLabel );
		break;

	case Pushed:
		bvl.drawLowered( 1, 1, w-2, h-2 );
		prsLabel.drawText( xt, yt+1, strLabel );
		break;
	}

	if (m_hasFocus) {
		penBlack.setDashes(2,2);
		penBlack.rectangle( wBorder-2, wBorder-2, maxW+4, maxH+5 );
	}
}

void Button::onMouseButton(int nButton, bool isPush)
{
	using std::endl;

	if (nButton==1) {
		if (isPush) {
			m_state = Pushed;
			trace << "Pushed" << endl;
			invalidate();
		} else {
			if (m_state==Pushed) {
				doAction( m_strAction );
				invalidate();
			}

			m_state = Inactive;
			trace << "Inactive" << endl;
		}
	}
}

void Button::onMouseEnter()
{
	if (m_state==Released) {
		m_state = Pushed;
		trace << "Pushed" << std::endl;
		invalidate();
	}
}

void Button::onMouseExit()
{
	if (m_state==Pushed) {
		m_state = Released;
		trace << "Released" << std::endl;
		invalidate();
	}
}

void Button::onGotFocus()
{
	m_hasFocus = true;
	invalidate();
}

void Button::onLostFocus()
{
	m_hasFocus = false;
	invalidate();
}

} // namespace
