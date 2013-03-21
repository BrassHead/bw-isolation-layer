/* colour.cc -- Implementation of device colour objects

Copyright (C) 1998-2013, Brian Bray

*/


#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/tools.h"

#include <X11/Xlib.h>
#include "fcolour.h"

namespace bw {

// Totally unimplemented functions

void FColour::setGCV( const Colour& clr, unsigned long& vm, XGCValues& xgcv )
{
	vm |= GCForeground;
	xgcv.foreground = ((clr.r&0xff) << 16) |
	                  ((clr.g&0xff) << 8) |
	                  (clr.b&0xff);

	// TODO: More complex colours and visuals.  This could affect:
	//	     tile, stipple, ts_[xy]_origin
}

void FColour::setGCVBack( const Colour& clr, unsigned long& vm, XGCValues& xgcv )
{
	vm |= GCBackground;
	xgcv.background = ((clr.r&0xff) << 16) |
	                  ((clr.g&0xff) << 8) |
	                  (clr.b&0xff);

	// TODO: More complex colours and visuals.
}

class SysPalette : public Palette {
public:
	SysPalette();
	~SysPalette();

	virtual int pixelRange() const {
		bwassert(false);
		return 0;
	}; // TODO
	virtual int numColours() const {
		bwassert(false);
		return 0;
	}; // TODO
	virtual Pixel lookup(Colour c) const {
		bwassert(false);
		return Pixel();
	}; // TODO
	virtual Pixel allocate(Colour c) {
		bwassert(false);
		return Pixel();
	}; // TODO
	virtual void set(Pixel p, Colour c) {
		bwassert(false);
	}; // TODO

	virtual Colour operator[](const Pixel p) const;
	virtual Colour& operator[](const Pixel p);

private:
	Colour	m_aclr[NUMSYSCOLOUR];
};

SysPalette::SysPalette()
{
	// TODO:  Get these from configuration options
	m_aclr[Black] 	= 		Colour(   0,   0,   0 );
	m_aclr[White] 	= 		Colour( 255, 255, 255 );
	m_aclr[DarkShadow]	= 	Colour(  64,  64,  64 );
	m_aclr[Shadow]	= 		Colour( 128, 128, 128 );
	m_aclr[Face]	= 		Colour( 160, 160, 160 );
	m_aclr[Light]	= 		Colour( 192, 192, 192 );
	m_aclr[BrightLight]	= 	Colour( 224, 224, 224 );
	m_aclr[ButtonText]	= 	Colour(   0,   0,   0 );
	m_aclr[GreyText]	= 	Colour(  96,  96,  96 );
	m_aclr[ControlSurface] = Colour( 160, 160, 160 );
	m_aclr[Workspace]	= 	Colour(  47,  79,  79 );	// DarkSlateGrey
	m_aclr[TextBack]	= 	Colour( 160, 160, 160 );
	m_aclr[Text]	= 		Colour(   0,   0,   0 );
	m_aclr[HighlightBack] = Colour( 255, 250, 205 );
	m_aclr[Highlight]	= 	Colour(   0,   0,   0 );
	m_aclr[TipBack]	= 		Colour( 255, 255, 224 );
	m_aclr[TipText]	= 		Colour(   0,   0,   0 );
	m_aclr[MenuBack] = 		Colour( 160, 160, 160 );
	m_aclr[MenuText] = 		Colour(   0,   0,   0 );
	m_aclr[MenuHighlightBack] = Colour(   0,   0, 255 );
	m_aclr[MenuHighlightText] = Colour( 255, 255, 255 );
	m_aclr[MenuGreyText] = 	Colour( 128, 128, 128 );
	m_aclr[ScrollSurface] = Colour( 160, 160, 160 );
	m_aclr[HotLinkText] = 	Colour(   0,   0, 255 );
	m_aclr[VisitedText] = 	Colour( 255,   0, 255 );
}

SysPalette::~SysPalette()
{}

Colour SysPalette::operator[](const Pixel p) const
{
	bwassert( p>=0 && p<NUMSYSCOLOUR );
	return m_aclr[p];
}


Colour& SysPalette::operator[](const Pixel p)
{
	bwassert( false );		// Should not occur
	throw "Attempt to modify system colour.";	//TODO Correct exception
}




const Palette& getSysPalette()
{
	static SysPalette TheSysPalette;

	return TheSysPalette;
}


Colour getSysColour( StyleIndex cindx )
{
	return getSysPalette()[cindx];
}



} // NAMESPACE
