/* tools.h -- Interface to standard drawing tools

Copyright (C) 1997-2013, Brian Bray

*/

/* Needs:
#include "bw/string.h"
#include "bw/canvas.h"
#include "bw/colour.h"
#include "bw/font.h"
*/

namespace bw {

class BGC;
class FontMetric;
class FigureImp;
class Scene;
typedef int Pixel;
class String;
class Figure;
struct CXFontStruct;

class Canvas {
public:
	Canvas(FigureImp* pfi);
	virtual ~Canvas();

	// Device wide convenience functions
	virtual Scene& getScene() const;

	// Canvas size
	virtual unsigned int width() const;
	virtual unsigned int height() const;

private:
	friend class FCanvas;
	friend class BGC;
	FigureImp*	m_pfi;
};


class Colour {
public:
	Colour(unsigned char red=0, unsigned char green=0, unsigned char blue=0)
		: r(red), g(green), b(blue) {}

	unsigned char	r;
	unsigned char	g;
	unsigned char	b;
};

class Palette {
public:
	Palette() {}
	virtual ~Palette() {}

	virtual int pixelRange() const =0;
	virtual int numColours() const =0;
	virtual Colour operator[](Pixel p) const =0;
	virtual Pixel lookup(Colour c) const =0;
	virtual Pixel allocate(Colour c) =0;
	virtual void set(Pixel p, Colour c) =0;
};

class IndexPalette : public Palette {
public:
	IndexPalette(int pixelRange=256);
	virtual ~IndexPalette();

	virtual int pixelRange() const;
	virtual int numColours() const =0;
	virtual Colour operator[](Pixel p) const;
	virtual Pixel lookup(Colour c) const;
	virtual Pixel allocate(Colour c);
	virtual Pixel allocateDynamic(Colour c);
	virtual void set(Pixel p, Colour c);

private:
	//???
};


class MapPalette : public Palette {
public:
	MapPalette();
	virtual ~MapPalette();

	virtual int pixelRange() const;
	virtual int numColours() const =0;
	virtual Colour operator[](Pixel p) const;
	virtual Pixel lookup(Colour c) const;
	virtual Pixel allocate(Colour c);
	virtual Pixel allocateDynamic(Colour c);
	virtual void set(Pixel p, Colour c);

private:
	//???
};


// System colours
enum StyleIndex {
	Black=0, White,

	DarkShadow, Shadow, Face, Light, BrightLight,
	ButtonText, GreyText,
	ControlSurface,

	Workspace,
	TextBack, Text,
	HighlightBack, Highlight,

	TipBack, TipText,

	MenuBack, MenuText,
	MenuHighlightBack, MenuHighlightText,
	MenuGreyText,

	ScrollSurface,

	HotLinkText, VisitedText,

	NUMSYSCOLOUR
};

const Palette& getSysPalette();
Colour getSysColour( StyleIndex cindx );


class TextExtent {
public:
	int	width;
	int height;
	int ascent;
	int	descent;
};

enum FontIndex {FaceFont=0, SmallFaceFont, MaxFontIndex=1023};


class FontMetric {
public:
	FontMetric( const Canvas& cvs, FontIndex findx );
	FontMetric( const Figure& fig, FontIndex findx );
	virtual ~FontMetric();

	virtual int height();
	virtual int width();
	virtual int ascent();
	virtual int descent();

	virtual TextExtent getTextExtent( const String& str );
	virtual int getTextLength( const String& str );
	virtual int fitString( const String str, int w );

protected:
	virtual void changeFont( Canvas* pcvs, FontIndex findx );
	CXFontStruct*	m_pfs;
};


class Tool {
public:
	virtual void setColour( const Colour& c );

protected:
	Tool();
	virtual ~Tool();
	unsigned long	m_vm;
	BGC*		m_pbgc;
};

class LineTool : public Tool {
public:
	virtual void line(int x1, int y1, int x2, int y2);
	virtual void rectangle(int x, int y, int w, int h);
	virtual void setDashes(int on, int off);
protected:
	LineTool() {}
	virtual ~LineTool() {}
};

class Pen : public LineTool {
public:
	Pen(Canvas& cvs, const Colour& c);
	Pen(Canvas& cvs, const StyleIndex ic=Text);
	~Pen();

private:
	void makePen(Canvas& cvs, const Colour& c);
};

class Brush : public LineTool {
public:
	Brush(Canvas& cvs, const Colour& c);
	Brush(Canvas& cvs, const StyleIndex ic=Text);
	~Brush();

	virtual void fillRectangle(int x, int y, int w, int h);

private:
	void makeBrush(Canvas& cvs, const Colour& c);
};

class Press : public Tool, public FontMetric {
public:
	Press( Canvas& cvs, FontIndex findx=FaceFont );
	virtual ~Press();

	void drawText( int x, int y, String str );
	void changeFont( FontIndex findx );

private:
	Canvas*	m_pcvs;
};


}   // namespace

