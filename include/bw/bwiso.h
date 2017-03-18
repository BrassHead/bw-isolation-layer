/* bwiso.h -- Interface definition for windowing system isolation layer

Copyright (C) 1998-2013, Brian Bray

*/


/*
#include <set>
#include "bw/countable.h"
*/

// Downward partial definitions
typedef struct _XDisplay Display;
typedef struct _XRegion* Region;
typedef struct _XImage XImage;
typedef struct _XGC *GC;

namespace bw {

// References to user exposed classes
class FontSpec;
class KeyNum;
class Event;
typedef int Pixel;

// Forward References
class DisplayHand;
class WinHand;
class CursorHand;
class PixmapHand;
class BitmapHand;
class PrinterHand;
class GCHand;
class GCSpec;
class FontHand;
class RegionHand;
class PhysicalPalette;
class PBitmap;

// Upward partial definitions
class Icon;
class Figure;
class TextExtent;
class Colourmap;
class BWImage;
class BWBitmap;
class BWPixmap;
class BWTrueImage;
class BWFloatImage;


class VisualInfo;

// Simple types

struct Point {
	short int x;
	short int y;
	bool operator==(const Point& p) const {
		return x==p.x && y==p.y;
	}
};

class TextExtent {	// TODO: Shouldn't be here
public:
	int	width;
	int height;
	int ascent;
	int	descent;
};

class Colour {
public:
	Colour(unsigned short int r, unsigned short int g, unsigned short int b) : red(r), green(g), blue(b) {};
	Colour() : red(0), green(0), blue(0) {};
	unsigned short int red;
	unsigned short int green;
	unsigned short int blue;
};

enum VisualType {
	PrintOnly,
	Monochrome,
	Grayscale,
	FixedPalette,
	DynamicPalette,
	PrivatePalette,
	TrueColour
};

enum EventMask {
	EMNone		=0x00000000,
	EMKeyPress		=0x00000001,
	EMKeyRelease	=0x00000002,
	EMButtonPress	=0x01000004,	// includes OwnerGrabButtonMask
	EMButtonRelease	=0x00000008,
	EMEnterWindow	=0x00000010,
	EMLeaveWindow	=0x00000020,
	EMMouseMotion	=0x00000040,
	EMButtonMotion	=0x00002000,
	EMExposure		=0x00008000,
	EMVisibility	=0x00010000,
	EMStructure		=0x00020000,
	EMSubstructure	=0x00080000,
	EMFocus		=0x00200000,
};

// Looks like these trip gcc bugs
//inline EventMask operator|(const EventMask a,const EventMask b)
//{
//  return (EventMask)(a | b);
//}

//inline EventMask operator&(const EventMask a,const EventMask b)
//{
//   return (EventMask)(a & b);
//}

enum EventType {
	NoEvent			=0,	// Will occur, should be ignored

	EVKeyPress			=2,	// Note can occur before focus event if user
	EVKeyRelease		=3,	// has typed ahead
	EVButtonPress		=4,
	EVButtonRelease		=5,
	EVMotionNotify		=6,
	EVEnterNotify		=7,
	EVLeaveNotify		=8,
	EVFocusIn			=9,
	EVFocusOut			=10,
	EVKeymapNotify		=11,	// Handled by iso layer
	EVExpose			=12,
	EVGraphicsExpose		=13,	// Should not occur
	EVNoExpose			=14,	// Should not occur
	EVVisibilityNotify		=15,
	EVCreateNotify		=16,
	EVDestroyNotify		=17,
	EVUnmapNotify		=18,
	EVMapNotify			=19,
	EVMapRequest		=20,	// Should not occur
	EVReparentNotify		=21,
	EVConfigureNotify		=22,
	EVConfigureRequest		=23,	// Should not occur
	EVGravityNotify		=24,
	EVResizeRequest		=25,	// Should not occur
	EVCirculateNotify		=26,
	EVCirculateRequest		=27,	// Should not occur
	EVPropertyNotify		=28,	// Should not occur
	EVSelectionClear		=29,
	EVSelectionRequest		=30,
	EVSelectionNotify		=31,
	EVColormapNotify		=32,
	EVClientMessage		=33,
	EVMappingNotify		=34,
	EVExtensionEvent		=35,   // not used currently (may change)
	EVCloseRequest		=36,   // mapped from client message WM_DELETE_WINDOW
	EVAlarm			=37,   // mapped from XSYNC Alarm message
	EVError			=38,   // Returned for XErrorEvents

	Activate			=40,
	PreActivate			=41,
	PostActivate		=42,
	InvalidateRect		=43,
	InvalidateParentRect	=44,
	Draw			=45,
	Flash			=46,
	Deactivate			=47,
	PreDeactivate		=48,
	AppSpecific			=49,
	MaxEvent
};

enum MouseButton {
	noButton	=0,
	leftButton	=1,
	midButton	=2,
	rightButton	=3
};

enum Modifier {
	ModNone	=0x0000,
	ModShift	=0x0001,
	ModControl	=0x0004,
	ModAlt	=0x0008,
	ModLeft	=0x0100,
	ModMid	=0x0200,
	ModRight	=0x0400,
	ModAnyKey	=0x000D,
	ModAnyMouse	=0x0700,
	ModMask	=0x070D,
	ModAny	=0x8000
};

//bool operator==(const Modifier m1, const Modifier m2)
//{
//    return (m1==ModAny) || (m2==ModAny) || (m1&ModMask==m2&ModMask);
//}

enum PrintQuality {
	TextOnly,
	ScreenCompatible,
	MaxResolution
};

enum DefinedCursor {
	DefaultCursor=132,
	TopLeftArrow=132,
	XCursor=0,
	Crosshair=130,
	Move=52,
	TopRightArrow=2,
	Wait=150,
	Text=152,
	Pencil=86,
	Spraycan=124,
	Finger=59,
	Hold=36
};

enum AbsRel {
	Absolute,
	Relative
};

enum ShapeHint {
	SHConvex,
	SHClosed,
	SHComplex
};

enum JoinStyle {
	Miter,
	Round,
	Bevel
};

typedef cptr<Event> EventRef;

typedef void (*FatalErrorProc)(DisplayHand);
typedef void (*ErrorProc)(DisplayHand, EventRef);
typedef void (*WinProc)(Figure*, EventRef);


class DisplayHand {
public:
	static void setWinProc(WinProc);
	static void setErrorHandler(ErrorProc);
	static void setFatalErrorHandler(FatalErrorProc);
	static bool localeIsSet(const char *modifiers="");

	DisplayHand();
	~DisplayHand();

	void open( int& argc, char* argv[],
	           VisualType pref=DynamicPalette, const char* displayName=0);
	void close();

	bool isValid();

	long planeMask();
	int numPlanes();
	const char& screenName();
	WinHand rootWindow();
	int screenHeight();
	int screenWidth();
	int assumedPixelsPerMeter();
	int assumedPixelsPerInch();
	int assumedScreenHeightMM();
	int assumedScreenWidthMM();
	int colormapSize();
	VisualType typeOfVisual();
	WinHand newFrameWindow(EventMask, Figure* pfig, const char* pszTitle,
	                       int x, int y, int w, int h);
	PhysicalPalette& getScreenPalette();
	PixmapHand newPixmap(int w, int h);
	BitmapHand newBitmap(int w, int h);
	//PrinterHand newPrinter(const char* name, PrintQuality);
	FontHand newFont(const FontSpec&);
	std::set<FontSpec> findFonts(const FontSpec& pattern);
	CursorHand newCursor(DefinedCursor);
	void flush();
	void sync();
	int countEventsPending();
	EventRef nextEvent();
	//void addFileWatch(int fd);
	void addTimer(int msec);
	void sendEvent(const EventRef);
	void dispatchEvent(const EventRef);
	void setTextClipboard(const char*);
	int getTextClipboard(char* buf, int buflen);
	void undoTextClipboard();

public:	// Internal use
	DisplayHand(Display* pdis);
	static void callErrorProc(Display*, EventRef);
	static void callFatalErrorProc(Display*);

private:
	void makeFontName(char*,const FontSpec&);

	Display*		pd;

	static WinProc	pwp;
	static ErrorProc	pep;
	static FatalErrorProc pfep;
};


struct RGBA {
	unsigned char	alpha;
	unsigned char	red;
	unsigned char	green;
	unsigned char	blue;
};


class IImage {
public:
	IImage() {};
	virtual ~IImage() {};

	enum FieldType {
		FTBitmap, FTIndex, FTTrueColour, FTGrayscale, FTAlpha, FTDepth, FTOther,
		FTFloatColour, FTFloatGray, FTFloatAlpha, FTFloatDepth, FTFloatOther,
		FTNonStandard
	};
	enum Endian {MSB,LSB};
	enum ScanLineOrder {TopFirst, BottomFirst};

	virtual int w() const =0;
	virtual int h() const =0;
	virtual int depth(int field) const =0;
	virtual int colourmapSize() const =0;
	virtual int numFields() const =0;
	virtual FieldType type(int field) const =0;

	virtual Pixel getPixel(int x, int y) const =0;
	Pixel operator()(int x, int y) const {
		return getPixel(x,y);
	}
	virtual void setPixel(int x, int y, Pixel pxl) =0;
	virtual Colour getColour(int x, int y) const =0;
	virtual void setColour(int x, int y, Colour) =0;

	virtual void addPixel(Pixel) =0;
	virtual void mapPixels(Pixel*) =0;

	struct DataLayout {
		FieldType 	fieldType;
		ScanLineOrder	scanLineOrder;
		Endian		byteOrder;
		Endian		bitOrder;
		int		width;
		int		height;
		int		bitsPerPixel;
		int		depth;
		int		xOffset;
		int		lineSize;
		int		unitSize;
		int		lineAlignment;
		Pixel		alphaMask;
		Pixel		redMask;
		Pixel		greenMask;
		Pixel		blueMask;
		int		majorColours;
	};

	virtual DataLayout describeData(int field) const =0;
	virtual void* getData(int field) =0;
	virtual Colourmap getColourmap() =0;
	virtual void getRGBAMap(RGBA* buf, int nBuf) const =0;

private:
	IImage(const IImage&);
	IImage& operator=(const IImage&);
};

class PImage : public IImage {
public:
	PImage(DisplayHand);
	PImage(DisplayHand, const BWImage&);
	virtual ~PImage();

	virtual int w() const;
	virtual int h() const;
	virtual int depth(int field) const;
	virtual int colourmapSize() const;
	virtual int numFields() const;
	virtual FieldType type(int field) const;

	virtual Pixel getPixel(int x, int y) const;
	virtual void setPixel(int x, int y, Pixel pxl);
	virtual Colour getColour(int x, int y) const;
	virtual void setColour(int x, int y, Colour);

	virtual void addPixel(Pixel);
	virtual void mapPixels(Pixel*);

	virtual DataLayout describeData(int field) const;
	virtual void* getData(int field);
	virtual Colourmap getColourmap();
	virtual void getRGBAMap(RGBA* buf, int nBuf) const;

	void convertTo(BWImage&);
	void convertTo(BWBitmap&, Pixel threashold);
	void convertTo(PBitmap&, Pixel threashold);
	void convertTo(BWPixmap&);
	void convertTo(BWTrueImage&);
	void convertTo(BWFloatImage&);

private:
	friend class WinHand;
	PImage(const PImage&);
	PImage& operator=(const PImage&);

	void setImage(XImage*);

	XImage*	ximg;
};


class PBitmap : public IImage {
public:
	PBitmap(DisplayHand);
	PBitmap(DisplayHand, const BWBitmap&);
	virtual ~PBitmap();

	virtual int w() const;
	virtual int h() const;
	virtual int depth(int field) const;
	virtual int colourmapSize() const;
	virtual int numFields() const;
	virtual FieldType type(int field) const;

	virtual Pixel getPixel(int x, int y) const;
	virtual void setPixel(int x, int y, Pixel pxl);
	virtual Colour getColour(int x, int y) const;
	virtual void setColour(int x, int y, Colour);

	virtual void addPixel(Pixel);
	virtual void mapPixels(Pixel*);

	virtual DataLayout describeData(int field) const;
	virtual void* getData(int field);
	virtual Colourmap getColourmap();
	virtual void getRGBAMap(RGBA* buf, int nBuf) const;

	void convertTo(BWImage&);
	void convertTo(BWBitmap&);
	void convertTo(PImage&, Pixel pxlZero, Pixel pxlOne);
	void convertTo(BWPixmap&, Pixel pxlZero, Pixel pxlOne);
	void convertTo(BWTrueImage&, Pixel pxlZero, Pixel pxlOne);
	void convertTo(BWFloatImage&, Colour clrZero, Colour clrOne);

private:
	PBitmap(const PBitmap&);
	PBitmap& operator=(const PBitmap&);
};


class WinHand {
public:
	//enum WinType {Invalid, Frame, Dialog, Popup, Child};
	enum Modality {Concurrent, Modal};

	WinHand();
	void destroy();

	bool isValid();

	void move(int x, int y);
	void resize(int w, int h);
	void reshape(RegionHand);
	void moveresize(int x, int y, int w, int h);
	void show();
	void hide();
	void raise();
	void lower();
	void setBackground(Pixel);
	void setBackground(PixmapHand);
	void unsetBackground();
	void setBackgroundToParent();
	void setEventMask(EventMask);
	void setProcArg(Figure*);
	Figure* getProcArg();

	GCHand newGC(const GCSpec&);
	void getImage(int x, int y, int w, int h, PImage&);
	void getSubImage(int x, int y, int w, int h, PImage&, int destX, int destY);

	void setCursor(CursorHand);
	void grabPointer(EventMask em, CursorHand);
	void releasePointer();
	void grabKeyboard();
	void releaseKeyboard();
	void takeFocus();
	WinHand newChild(EventMask, Figure* pfig, int x, int y, int w, int h);
	WinHand newChild(EventMask, Figure* pfig, int x, int y, int w, int h, RegionHand);
	WinHand newPopup(EventMask, Figure* pfig, int x, int y, int w, int h);
	WinHand newPopup(EventMask, Figure* pfig, int x, int y, int w, int h, RegionHand);

	// Frame and dialog windows only
	WinHand newDialog(EventMask, Figure* pfig, Modality,
	                  int x, int y, int w, int h);
	void setTitle(const char*);
	void setMinSize(int w, int h);
	void setMaxSize(int w, int h);

	// Frame Windows only
	WinHand newSecondary(EventMask, Figure* pfig, const char* pszTitle,
	                     int x, int y, int w, int h);
//    WinHand newIconWindow(EventMask, Figure* pfig);
//    void setIconTitle(const char*);

public:	// Internal use
	WinHand(Display* pDisplay, unsigned long window);
	bool isAncestor(unsigned long anc, unsigned long dec);

private:
	friend class DisplayHand;
	friend class XGC;
	Display*		pd;
	unsigned long	wnd;
};

class PixmapHand {
public:
	PixmapHand();
	void destroy();

	bool isValid();

	int w();
	int h();

	GCHand newGC(const GCSpec&);
	void getImage(int x, int y, int w, int h, PImage&);
	void getSubImage(int x, int y, int w, int h, PImage&, int destX, int destY);
	bool operator==(const PixmapHand&) const;

private:
	friend class DisplayHand;
	friend class WinHand;
	friend class GCSpec;
	friend class XGC;
	PixmapHand(Display*,unsigned long,int w,int h);

	Display*		m_pDisplay;
	unsigned long	m_pixmap;
	int			m_w;
	int			m_h;
};


class BitmapHand {
public:
	BitmapHand();
	void destroy();

	bool isValid();

	int w();
	int h();

	GCHand newGC(const GCSpec&);
	void getImage(int x, int y, int w, int h, PImage&);
	void getSubImage(int x, int y, int w, int h, PImage&, int destX, int destY);
	bool operator==(const BitmapHand&) const;

private:
	friend class DisplayHand;
	friend class GCSpec;
	friend class XGC;
	BitmapHand(Display*,unsigned long,int w,int h);

	Display*		m_pDisplay;
	unsigned long	m_bitmap;
	int			m_w;
	int			m_h;
};


class FontHand {
public:
	FontHand();
	void release();

	int maxH() const;
	int maxW() const;
	int maxAscent() const;
	int maxDescent() const;

	void getTextExtent(const char*, TextExtent&) const;
	int getTextLength(const char*) const;

	//void getTextExtent(const wchar_t*, TextExtent&);
	//int getTextLength(const wchar_t*);

	bool operator==(const FontHand&) const;

private:
	friend class DisplayHand;
	friend class XGC;
	FontHand(Display*, void*);

	Display*		pDisplay;
	void*		pFontSet;
};


class RegionHand {
public:
	RegionHand();
	RegionHand(Point*, int nPoints);
	RegionHand(const RegionHand& rh);
	RegionHand& operator=(const RegionHand& rh);
	~RegionHand();

	bool isEmpty() const;
	bool isInside(int x, int y) const;
	bool isInside(int x, int y, int w, int h) const;
	bool operator==(const RegionHand&) const;

	void addRect(int x, int y, int w, int h);
	void subRect(int x, int y, int w, int h);
	void intersectRect(int x, int y, int w, int h);
	void add(const RegionHand&);
	void sub(const RegionHand&);
	void intersect(const RegionHand&);

private:
	friend class WinHand;
	friend class GCSpec;
	Region	areg() const;

	mutable Region	reg;
};


class GCSpec {
public:
	GCSpec();
	~GCSpec();

	enum Func {FCopy, FXor};
	enum Cap {Default, Butt, Round, Square};
	enum Dash {Solid, Broken, DashDotDash, DotDash,
	           LongDash, MediumDash, FineDash,
	           Dotted, FineDots
	          };
	enum Mask {
		mnone=	0x000,
		mfg=	0x001,
		mbg=	0x002,
		mtile=	0x004,
		mstip=	0x008,
		mwidth=	0x010,
		mfunc=	0x020,
		mcap=	0x040,
		mdash=	0x080,
		mclip=	0x100,
		morig=	0x200
	};

	void reset();

	Mask getMask() {
		return m_mask;
	}

	Pixel& foreground() {
		m_mask = (Mask)(m_mask | mfg);
		return m_fg;
	}
	Pixel& background() {
		m_mask = (Mask)(m_mask | mbg);
		return m_bg;
	}
	PixmapHand& tile() {
		m_mask = (Mask)(m_mask | mtile);
		return m_tile;
	}
	BitmapHand& stipple() {
		m_mask = (Mask)(m_mask | mstip);
		return m_stip;
	}
	int& lineWidth() {
		m_mask = (Mask)(m_mask | mwidth);
		return m_width;
	}
	Func& function() {
		m_mask = (Mask)(m_mask | mfunc);
		return m_func;
	}
	Cap& capStyle() {
		m_mask = (Mask)(m_mask | mcap);
		return m_cap;
	}
	Dash& dashStyle() {
		m_mask = (Mask)(m_mask | mdash);
		return m_dash;
	}
	Point& TSOrigin() {
		m_mask = (Mask)(m_mask | morig);
		return m_orig;
	}
	RegionHand& clip() {
		m_mask = (Mask)(m_mask | mclip);
		return m_clip;
	}

	bool compatible(const GCSpec&) const;
	void add(const GCSpec&);
	static GCSpec subtract(const GCSpec& orig,const GCSpec& imprvd);

private:
	friend class XGC;
	friend class GCPool;

	GC createGC(Display* pd,unsigned long wnd) const;
	void changeGC(Display* pd,GC gc) const;

	unsigned long xvalues(void* xgcv,Region& reg) const;

	Mask	m_mask;
	Pixel	m_fg;
	Pixel	m_bg;
	PixmapHand	m_tile;
	BitmapHand	m_stip;
	Point	m_orig;
	int		m_width;
	Func	m_func;
	Cap		m_cap;
	Dash	m_dash;
	RegionHand	m_clip;
};

class IGC {
public:
	IGC() {};
	virtual ~IGC() {};

	virtual bool isValid() =0;
	virtual bool isEmpty() =0;

	virtual void change(const GCSpec&) =0;
	virtual void setSpec(const GCSpec&) =0;
	virtual GCSpec getSpec() =0;

	virtual void clearAll() =0;
	virtual void clearArea(int x, int y, int w, int h) =0;
	virtual void copy(int x, int y, WinHand, int srcX, int srcY, int w, int h) =0;
	virtual void copy(int x, int y, PixmapHand, int srcX, int srcY, int w, int h) =0;
	virtual void copy(int x, int y, BitmapHand, int srcX, int srcY, int w, int h) =0;
	virtual void copy(int x, int y, PImage&, int srcX, int srcY, int w, int h) =0;

	virtual void drawPoint(int x, int y) =0;
	virtual void drawPoints(Point*, int nPoints, AbsRel) =0;
	virtual void drawLine(int x1, int y1, int x2, int y2) =0;
	virtual void drawRectangle(int x, int y, int w, int h) =0;
	virtual void fillRectangle(int x, int y, int w, int h) =0;
	virtual void drawArc(int x, int y, int w, int h, int angle1, int angle2) =0;
	virtual void fillPie(int x, int y, int w, int h, int angle1, int angle2) =0;
	virtual void fillChord(int x, int y, int w, int h, int angle1, int angle2) =0;
	virtual void drawPolygon(Point*, int nPoints, AbsRel, JoinStyle=Miter) =0;
	virtual void fillPolygon(Point*, int nPoints, AbsRel, ShapeHint=SHComplex) =0;
	virtual void drawString(FontHand fh,int x, int y, const char*) =0;
	//virtual void drawString(int x, int y, const wchar_t*) =0;
	virtual void fillString(FontHand fh,int x, int y, const char*) =0;
	//virtual void fillString(int x, int y, const wchar_t*) =0;
};

class DGC : public IGC, public Countable {
};
typedef cptr<DGC> DGCRef;


class GCHand : public IGC {
public:
	GCHand() : pGC(0) {}
	GCHand(const GCHand& gch) : IGC(),pGC(gch.pGC) {}
	GCHand(DGC * igc) : pGC(igc) {}
	~GCHand() {}
	bool operator==(const GCHand& gch) {
		return pGC==gch.pGC;
	}

	bool isValid() {
		return pGC->isValid();
	}
	bool isEmpty() {
		return pGC->isEmpty();
	}

	void change(const GCSpec& gcs) {
		pGC->change(gcs);
	}
	void setSpec(const GCSpec& gcs) {
		pGC->setSpec(gcs);
	}
	GCSpec getSpec() {
		return pGC->getSpec();
	}

	void clearAll() {
		pGC->clearAll();
	}
	void clearArea(int x, int y, int w, int h) {
		pGC->clearArea(x,y,w,h);
	}
	void copy(int x, int y, WinHand wh, int srcX, int srcY, int w, int h) {
		pGC->copy(x,y,wh,srcX,srcY,w,h);
	}
	void copy(int x, int y, PixmapHand ph, int srcX, int srcY, int w, int h) {
		pGC->copy(x,y,ph,srcX,srcY,w,h);
	}
	void copy(int x, int y, BitmapHand bh, int srcX, int srcY, int w, int h) {
		pGC->copy(x,y,bh,srcX,srcY,w,h);
	}
	void copy(int x, int y, PImage& img, int srcX, int srcY, int w, int h) {
		pGC->copy(x,y,img,srcX,srcY,w,h);
	}

	void drawPoint(int x, int y) {
		pGC->drawPoint(x,y);
	}
	void drawPoints(Point* pts, int nPoints, AbsRel ar) {
		pGC->drawPoints(pts,nPoints,ar);
	}
	void drawLine(int x1, int y1, int x2, int y2) {
		pGC->drawLine(x1,y1,x2,y2);
	}
	void drawRectangle(int x, int y, int w, int h) {
		pGC->drawRectangle(x,y,w,h);
	}
	void fillRectangle(int x, int y, int w, int h) {
		pGC->fillRectangle(x,y,w,h);
	}
	void drawArc(int x, int y, int w, int h, int angle1, int angle2) {
		pGC->drawArc(x,y,w,h,angle1,angle2);
	}
	void fillPie(int x, int y, int w, int h, int angle1, int angle2) {
		pGC->fillPie(x,y,w,h,angle1,angle2);
	}
	void fillChord(int x, int y, int w, int h, int angle1, int angle2) {
		pGC->fillChord(x,y,w,h,angle1,angle2);
	}
	void drawPolygon(Point* pts, int nPoints, AbsRel ar, JoinStyle js=Miter) {
		pGC->drawPolygon(pts,nPoints,ar,js);
	}
	void fillPolygon(Point* pts, int nPoints, AbsRel ar, ShapeHint sh=SHComplex) {
		pGC->fillPolygon(pts,nPoints,ar,sh);
	}
	void drawString(FontHand fh, int x, int y, const char* psz) {
		pGC->drawString(fh,x,y,psz);
	}
	//void drawString(int x, int y, const wchar_t* pwsz)
	//	{pGC->drawStringPolygon(x,y,pwsz);}
	void fillString(FontHand fh, int x, int y, const char* psz) {
		pGC->fillString(fh,x,y,psz);
	}
	//void fillString(int x, int y, const wchar_t* pwsz)
	//	{pGC->fillStringPolygon(x,y,pwsz);}

protected:
	DGCRef	pGC;
};


class PrinterHand {
public:
	PrinterHand();
	void close();

	bool isValid();

	int w();
	int h();

	GCHand newGC(const GCSpec&);

private:
	void*		m_ptr;
	unsigned long	m_ulong;
};


class CursorHand {
	CursorHand();
	void release();

	bool isValid();

private:
	friend class DisplayHand;
	friend class WinHand;
	CursorHand(Display*,unsigned long);

	Display*		pd;
	unsigned long	cursor;
};

class PhysicalPalette {
public:
	VisualType visualType();
	int pixelRange() const;
	int numColours() const;
	Colour operator[](Pixel p) const;
	Pixel lookup(Colour c) const;
	Pixel allocate(Colour c);
	Pixel allocateDynamic(Colour c);
	void set(Pixel p, Colour c);

private:
	friend class DisplayHand;
	PhysicalPalette(Display*, VisualInfo* pvis, unsigned long cmap, VisualType pref);
	~PhysicalPalette();
	static PhysicalPalette* find(Display*);

	static PhysicalPalette* 	pPalFirst;
	PhysicalPalette* 		pPalNext;
	Display*			pd;
	VisualInfo*			pvi;
	unsigned long		cm;
};


class FontSpec {
public:
	enum Weight {AnyWeight, Medium, Bold};
	enum Slant  {AnySlant, Roman, Italic, Oblique};

	FontSpec();				// FindAll
	FontSpec(const char* pszName);	// Note: friendly or X
	FontSpec(const char* pszFamily,
	         int tenPoints,
	         const char* pszFoundry=0,
	         Weight w=Medium,
	         Slant s=Roman,
	         const char* pszCharset=0);

	void getName(char* buf, int maxLen) const;
	const char* family() const;
	int tenPoints() const;
	const char* foundry() const;
	const char* charset() const;
	Weight weight() const;
	Slant slant() const;

	bool operator==(const FontSpec&) const;
	bool operator<(const FontSpec&) const;
	bool match(const FontSpec& pattern) const;

private:
	int		m_tenths;
	Weight	m_weight;
	Slant	m_slant;
	char	m_pszFamily[50];
	char	m_pszFoundry[50];
	char	m_pszCharset[50];
};

class KeyNum {
public:
	enum KeySym {
		KS_NoSymbol	=0x0000,

		KS_Escape	=0xff1b,

		KS_F1		=0xffbe,
		KS_F2		=0xffbf,
		KS_F3		=0xffc0,
		KS_F4		=0xffc1,
		KS_F5		=0xffc2,
		KS_F6		=0xffc3,
		KS_F7		=0xffc4,
		KS_F8		=0xffc5,
		KS_F9		=0xffc6,
		KS_F10		=0xffc7,
		KS_F11		=0xffc8,
		KS_F12		=0xffc9,

		// The 3 keys to the right of F12 may be unavailable or mapped to something else
		KS_Print	=0xff61,
		KS_Scroll_Lock	=0xff14,
		KS_Pause	=0xff13,

		KS_grave	=0x0060,
		KS_1		=0x0031,
		KS_2		=0x0032,
		KS_3		=0x0033,
		KS_4		=0x0034,
		KS_5		=0x0035,
		KS_6		=0x0036,
		KS_7		=0x0037,
		KS_8		=0x0038,
		KS_9		=0x0039,
		KS_0		=0x0030,
		KS_minus	=0x002d,
		KS_equal	=0x003d,
		KS_BackSpace	=0xff08,

		KS_Insert	=0xff63,
		KS_Home		=0xff50,
		KS_Page_Up	=0xff55,

		KS_Num_Lock	=0xff7f,
		KS_KP_Divide	=0xffaf,
		KS_KP_Multiply	=0xffaa,
		KS_KP_Subtract	=0xffad,

		KS_Tab		=0xff09,
		KS_q		=0x0071,
		KS_w		=0x0077,
		KS_e		=0x0065,
		KS_r		=0x0072,
		KS_t		=0x0074,
		KS_y		=0x0079,
		KS_u		=0x0075,
		KS_i		=0x0069,
		KS_o		=0x006f,
		KS_p		=0x0070,
		KS_bracketleft	=0x005b,
		KS_bracketright	=0x005d,
		KS_backslash	=0x005c,

		KS_Delete	=0xffff,
		KS_End		=0xff57,
		KS_Page_Down	=0xff56,

		KS_KP_7		=0xffb7,
		KS_KP_8		=0xffb8,
		KS_KP_9		=0xffb9,
		KS_KP_Add	=0xffab,

		KS_Caps_Lock	=0xffe5,
		KS_a		=0x0061,
		KS_s		=0x0073,
		KS_d		=0x0064,
		KS_f		=0x0066,
		KS_g		=0x0067,
		KS_h		=0x0068,
		KS_j		=0x006a,
		KS_k		=0x006b,
		KS_l		=0x006c,
		KS_semicolon	=0x003b,
		KS_apostrophe	=0x0027,
		KS_Return	=0xff0d,

		KS_KP_4		=0xffb4,
		KS_KP_5		=0xffb5,
		KS_KP_6		=0xffb6,

		KS_Shift_L	=0xffe1,
		KS_z		=0x007a,
		KS_x		=0x0078,
		KS_c		=0x0063,
		KS_v		=0x0076,
		KS_b		=0x0062,
		KS_n		=0x006e,
		KS_m		=0x006d,
		KS_comma	=0x002c,
		KS_period	=0x002e,
		KS_slash	=0x003f,
		KS_Shift_R	=0xffe2,

		KS_Up		=0xff52,
		KS_Left		=0xff51,
		KS_Down		=0xff54,
		KS_Right	=0xff53,

		KS_KP_1		=0xffb1,
		KS_KP_2		=0xffb2,
		KS_KP_3		=0xffb3,
		KS_KP_Enter	=0xff8d,
		KS_KP_0		=0xffb0,
		KS_KP_Decimal	=0xffae,

		KS_Control_L	=0xffe3,
		KS_Alt_L	=0xffe9,
		KS_space	=0x0020,
		KS_Alt_R	=0xffea,
		KS_Control_R	=0xffe4,

		// For "Windows" keys
		KS_Meta_L	=0xffe7,
		KS_Meta_R	=0xffe8,
		KS_Menu		=0xff67,

		// Special Mappings
		KS_Multi_Key	=0xff20	// Often assigned instead of Scroll Lock or Menu
	};

	KeyNum(const char* name);
	KeyNum(KeySym);

	KeySym keysym();
	const char* name();
	// map Prior -> Page_Up, Next -> Page_Down
	void fullName(Modifier m, char* retbuf, int nChars);
	bool isCursorKey();
	// (or PFKey)
	bool isFunctionKey();
	bool isKeypadKey();
	bool isMiscFunctionKey();
	bool isModifierKey();

private:
	KeySym	m_keysym;
};


class Event : public Countable {
public:
	enum EVOrigin {Local, App, System, SystemFailure, Failure};
	struct UserData {
		UserData(int f, const long ls[5])
			: format(f) {
			for (int i=0; i<5; i++) data.l[i]=ls[i];
		}
		UserData()
			: format(0) {
			for (int i=0; i<5; i++) data.l[i]=0L;
		}
		int	format;		// 0, 8, 16 or 32
		union {
			char b[20];
			short s[10];
			long l[5];
		} data;
	};

	virtual EVOrigin origin() const {
		return Local;
	}
	virtual EventType type() const {
		return NoEvent;
	}
	virtual int subType() const {
		return 0;
	}
	virtual const char* name() const {
		return "NullEvent";
	}

	virtual DisplayHand display() const {
		return DisplayHand();
	}
	virtual WinHand destination() const {
		return WinHand();
	}
	virtual WinHand object() const {
		return WinHand();
	}
	virtual int x() const {
		return 0;
	}
	virtual int y() const {
		return 0;
	}
	virtual int w() const {
		return 0;
	}
	virtual int h() const {
		return 0;
	}

	virtual MouseButton button() const {
		return noButton;
	};
	virtual Modifier modifiers() const {
		return ModNone;
	}
	virtual const KeyNum keyNum() const {
		return KeyNum(KeyNum::KS_NoSymbol);
	}
	virtual UserData userData() const {
		return UserData();
	}
	virtual int cookKeyNum(char*, int) const {
		return 0;
	}

protected:
	Event() {}
	virtual ~Event() {}

private:
	Event(const Event&);
	void operator=(const Event&);
};


}   // Namespace
