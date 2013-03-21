/* xiso.cc -- Interface between BW and X windows

Copyright 1998-2013 Brian Bray

*/


// This file has all the interface to Xlib with the exception of:
//	1)  Xrm Resource routines
//	2)  Regions from Xutil.h
// and with the addition of:
//      1)  Timers
//      2)  Shaped windows
//      3)  Modal windows

//#define NOTRACE
#include "bw/trace.h"

#include <map>
#include <list>
#include <set>

#include "bw/bwassert.h"
#include "bw/countable.h"
//#include "bw/string.h"
//#include "bw/custom.h"
#include "bw/bwiso.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
extern "C" {
#include <X11/extensions/sync.h>
}
#include <X11/extensions/shape.h>
#include <X11/cursorfont.h>

#include <stdio.h>
#include <cstring>
#include <cctype>

namespace bw {

//
// Global routines for app customizations
//
//Custom&		TheSite();
//Custom&		TheUser();
//Custom&		TheApp();


// Local vars
static const int maxResLength = 12;  // Max length of resource and class names
static const int maxFonts = 5000;    // Max fonts to find
static const int maxErrorLen = 200;  // Max length of error message from X

static XContext ctxPFIG;
static char *pszGeometry = 0;
static long assumedDPI = 75;

// Local classes

class VisualInfo : public XVisualInfo
{};

class GCPool;
typedef cptr<GCPool> GCPoolRef;

class GCPool : public Countable {
public:
	GCPool(Display* pdis,Drawable drawable,const GCSpec& gcs);
	~GCPool() {}

	static GCPoolRef firstCompatible(Display*,Drawable,const GCSpec&);
	bool isUnused() const;
	bool isShared() const;

	operator GC() const {
		return gc;
	}

private:
	static GCPoolRef	poolroot;

	Display*		pd;
	unsigned int	depth;
	GC		gc;
	GCSpec		combined;
	GCPoolRef		next;
};


class XGC : public DGC {
public:	// From IGC
	XGC(Display*,Drawable,const GCSpec&,const GCPoolRef);
	virtual ~XGC();

	virtual bool isValid();
	virtual bool isEmpty();

	virtual void change(const GCSpec&);
	virtual void setSpec(const GCSpec&);
	virtual GCSpec getSpec();

	virtual void clearAll();
	virtual void clearArea(int x, int y, int w, int h);
	virtual void copy(int x, int y, WinHand, int srcX, int srcY, int w, int h);
	virtual void copy(int x, int y, PixmapHand, int srcX, int srcY, int w, int h);
	virtual void copy(int x, int y, BitmapHand, int srcX, int srcY, int w, int h);
	virtual void copy(int x, int y, PImage&, int srcX, int srcY, int w, int h);

	virtual void drawPoint(int x, int y);
	virtual void drawPoints(Point*, int nPoints, AbsRel);
	virtual void drawLine(int x1, int y1, int x2, int y2);
	virtual void drawRectangle(int x, int y, int w, int h);
	virtual void fillRectangle(int x, int y, int w, int h);
	virtual void drawArc(int x, int y, int w, int h, int angle1, int angle2);
	virtual void fillPie(int x, int y, int w, int h, int angle1, int angle2);
	virtual void fillChord(int x, int y, int w, int h, int angle1, int angle2);
	virtual void drawPolygon(Point*, int nPoints, AbsRel, JoinStyle js=Miter);
	virtual void fillPolygon(Point*, int nPoints, AbsRel, ShapeHint sh=SHComplex);
	virtual void drawString(FontHand fh,int x, int y, const char*);
	//virtual void drawString(int x, int y, const wchar_t*);
	virtual void fillString(FontHand fh,int x, int y, const char*);
	//virtual void fillString(int x, int y, const wchar_t*);

public:
	static XGC* fetchGC(Display*,Drawable,const GCSpec& gcs);

private:
	Display*		pd;
	Drawable		draw;
	GCSpec		gcs;
	GCPoolRef		gcpr;
};

static const char* apszNames[] = {
	"WM_PROTOCOLS",
	"WM_DELETE_WINDOW",
	"WM_TAKE_FOCUS",
	"WM_SAVE_YOURSELF"
};

static Atom aCutBufferList[] = {
	XA_CUT_BUFFER0,
	XA_CUT_BUFFER1,
	XA_CUT_BUFFER2,
	XA_CUT_BUFFER3,
	XA_CUT_BUFFER4,
	XA_CUT_BUFFER5,
	XA_CUT_BUFFER6,
	XA_CUT_BUFFER7
};

// Holds per display data
class DisplayData {
public:
	Window	focus;
	Window	modal_root;
	bool	isModal;
	struct {
		Atom	aProtocol;		// Order matches list above
		Atom	aDelete;
		Atom	aFocus;
		Atom	aSave;
	} atoms;
	struct Extension {
		int	baseEvent;
		int	baseError;
	};
	Extension	sync;
	Extension	shape;
};

static std::map<Display*,DisplayData*> ddMap;


const char* apszIsoEventNames[] = {
	"NullEvent",	    // 0
	"Unknown",		    // 1
	"KeyPress",		    // 2		XKeyEvent
	"KeyRelease",	    // 3		XKeyEvent
	"ButtonPress",	    // 4		XButtonEvent
	"ButtonRelease",	// 5		XButtonEvent
	"MotionNotify",	    // 6		XMotionEvent
	"EnterNotify",	    // 7		XCrossingEvent
	"LeaveNotify",	    // 8		XCrossingEvent
	"FocusIn",		    // 9		XFocusChangeEvent
	"FocusOut",		    // 10		XFocusChangeEvent
	"KeymapNotify",	    // 11		XKeymapEvent
	"Expose",		    // 12		XExposeEvent
	"GraphicsExposure",	// 13		XGraphicsExposeEvent
	"NoExposure",	    // 14		XNoExposeEvent
	"VisibilityNotify",	// 15		XVisibilityEvent
	"CreateNotify",	    // 16		XCreateWindowEvent
	"DestroyNotify",	// 17		XDestroyWindowEvent
	"UnmapNotify",	    // 18		XUnmapEvent
	"MapNotify",	    // 19		XMapEvent
	"MapRequest",	    // 20		XMapRequestEvent
	"ReparentNotify",	// 21		XReparentEvent
	"ConfigureNotify",	// 22		XConfigureEvent
	"ConfigureRequest",	// 23		XConfigureRequestEvent
	"GravityNotify",	// 24		XGravityEvent
	"ResizeRequest",	// 25		XResizeRequestEvent
	"CirculateNotify",	// 26		XCirculateEvent
	"CirculateRequest",	// 27		XCirculateRequestEvent
	"PropertyNotify",	// 28		XPropertyEvent
	"SelectionClear",	// 29		XSelectionClearEvent
	"SelectionRequest",	// 30		XSelectionRequestEvent
	"SelectionNotify",	// 31		XSelectionEvent
	"ColormapNotify",	// 32		XColormapEvent
	"ClientMessage",	// 33		XClientMessageEvent
	"MappingNotify",	// 34		XMappingEvent
	"Extension",	    // 35		an unexpected extension event
	"CloseRequest",	    // 36		ClientMessage WM_DELETE_WINDOW
	"Alarm",		    // 37		XSYNC Alarm
	"Error",		    // 38		X Protocol error
};


/*: class Event

  The Event class communicate information from the Windowing system to
  the BW framework code.  This is an INTERNAL class only; it should never
  be referenced from application code.

  Events are requested by calls to Display::nextEvent and are normally
  distributed to a procedure for each type of window class used.

  The Event class is abstract and derived from "countable".  Normally, events
  are accessed through an EventRef which behaves like an Event*.  The event is
  kept in memory as long as there is an EventRef that points to it.

  Events can be generated from the windowing system itself, the BW application
  framework, the calling application, or other applications.

  System events contain information specific to each event type.  Const member
  functions provide access to this information in generic terms:

  <ul>
  <li>The display
  <li>The window destination
  <li>The window that is the subject of the event (usually a child of the
      destination)
  <li>A location (x,y)
  <li>A size (w,h)
  <li>A mouse button
  <li>A keyboard/mouse modifier state (shift keys and mouse buttons)
  <li>A keyboard button
  <ul>

  If one of these pieces of information is requested for an event type that
  does not provide the information, a default value is returned.

  Events are also used to communication error conditions.

  User events can contain 20 bytes, 10 shorts, or 5 longs of arbitrary data.

*/

/*: function: Event::origin()

  Returns a code indicating the originator of the event.  One of:
  <ul>
  <li>Event::Local
  <li>Event::App
  <li>Event::System
  <li>Event::SystemFailure
  <li>Event::Failure
  </ul>

  Prototype: EVOrigin Event::origin() const
*/

/*: function: Event::type()

  Returns a code indicating the type of event.  One of:
  <ul>
  <li>NoEvent - should be ignored
  <li>EVKeyPress
  <li>EVKeyRelease
  <li>EVButtonPress - mouse button
  <li>EVButtonRelease
  <li>EVMotionNotify - mouse movement
  <li>EVEnterNotify
  <li>EVLeaveNotify
  <li>EVFocusIn - Keyboard Focus change
  <li>EVFocusOut
  <li>EVExpose - window redraw required
  <li>EVVisibilityNotify
  <li>EVCreateNotify
  <li>EVDestroyNotify
  <li>EVUnmapNotify
  <li>MapNotify
  <li>EVReparentNotify
  <li>EVConfigureNotify
  <li>EVGravityNotify
  <li>EVCirculateNotify
  <li>EVSelectionClear
  <li>EVSelectionRequest
  <li>EVSelectionNotify
  <li>EVColormapNotify
  <li>EVClientMessage
  <li>EVMappingNotify
  <li>EVCloseRequest
  <li>EVAlarm
  <li>EVError
  <li>Activate
  <li>PreActivate
  <li>PostActivate
  <li>InvalidateRect
  <li>InvalidateParentRect
  <li>Draw
  <li>Flash
  <li>Deactivate
  <li>PreDeactivate
  <li>AppSpecific
  <li>MaxEvent - 1 larger than largest event number
  </ul>

  Prototype: EventType Event::type() const
*/

/*: function Event::subType()

  Returns the subtype of the event.  The values and semantics of this field
  depend on the event type.

  Prototype: int Event::subtype() const
*/

/*: function Event::name()

  Returns a desciptive name of the event type for diagnostic purposes.

  Prototype: const char* Event::name() const
*/

/*: function Event::display()

  Returns the display for the event.

  Prototype: DisplayHand Event::display() const
*/

/*: function Event::destination()

  Returns the destination window for the event.

  Prototype: WinHand Event::destination() const
*/

/*: function Event::object()

  Returns the window that is the object of the event.  This is normally the
  destination window or one of its descendents.

  Prototype: WinHand Event::object() const
*/

/*: function Event::x()

  Returns an X co-ordinate associated with the event.  This is either a mouse
  position, a position within a window, or a window position within
  it's parent.  Zero is the default value.

  Prototype: int Event::x() const
*/

/*: function Event::y()

  Returns a Y co-ordinate associated with the event.  This is either a mouse
  position, a position within a window, or a window position within
  it's parent.  Zero is the default value.

  Prototype: int Event::y() const
*/

/*: function Event::w()

  Returns a width associated with the event.  This is either a window width
  or the width of a region within a window.  Zero is the default value.

  Prototype: int Event::w() const
*/

/*: function Event::h()

  Returns a height associated with the event.  This is either a window height
  or the height of a region within a window.  Zero is the default value.

  Prototype: int Event::h() const
*/

/*: function Event::button()

  Returns a mouse button.  Valid for button press and release events.  Default
  value is NoButton.

  Prototype: MouseButton Event::button() const
*/

/*: function Event::modifiers()

  Returns keyboard shift state and/or mouse button state that may modify the
  meaning of the event.  The default value is ModNone.

  Prototype: Modifier Event::modifiers() const
*/

/*: function Event::keyNum()

  Returns an identifier for a keyboard key.  Valid for key press and release
  events.  The default value is KeyNum::KS_NoSymbol.

  Prototype: const KeyNum Event::keyNum() const
*/

/*: function Event::userData()

  Returns the user data associated with a user defined event.  The default is
  an empty (0) UserData.

  Prototype: UserData Event::userData() const
*/

/*: function Event::cookKeyNum()

  Returns a string containing the characters entered from a keypress event in
  the default locale of the application.  Note that many keypress events do not
  generate character input and some will return multiple characters.  This call
  only supports narrow string input.

  The characters returned may be from the current event or from a combination
  of earlier events.

  The int returned is the number of 'char's returned in the buffer.  The number
  of characters may be less if a multibyte character set is in effect.  A
  return value of zero indicates no input.

  If the buffer is too short for the resulting string, it is truncated.  The
  buffer is not NULL terminated.

  Prototype: int Event::cookKeyNum(char* buf, int buflen)
*/



class SystemEvent : public Event {
	// Note: this needs changing if extensions are ever called
public:
	static SystemEvent* newSystemEvent() {
		SystemEvent* psev;
		if (psevRoot) {
			psev = psevRoot;
			psevRoot = psevRoot->psevNext;
			(void)psev->AddRef();
		} else
			psev = new SystemEvent();

		return psev;
	}

	virtual unsigned long Release() {
		if (--m_cRef) return m_cRef;
		psevNext = psevRoot;
		psevRoot = this;
		return 0;
	}

	virtual EVOrigin origin() const {
		return System;
	}
	virtual EventType type() const {
		return (EventType)xev.xany.type;
	}
	virtual int subType() const {
		if (xev.xany.type==ClientMessage) return xev.xclient.message_type;
		return 0;
	}
	// Note: already mapped to int.
	virtual const char* name() const {
		return apszIsoEventNames[xev.xany.type];
	}

	virtual DisplayHand display() const {
		return DisplayHand(xev.xany.display);
	}
	virtual WinHand destination() const {
		return WinHand(xev.xany.display,xev.xany.window);
	}
	virtual WinHand object() const {
		Window wnd;
		switch (xev.xany.type) {
		case ButtonPress:
		case ButtonRelease:
			wnd = xev.xbutton.subwindow;
			break;

		case CirculateNotify:
			wnd = xev.xcirculate.window;
			break;

		case ConfigureNotify:
			wnd = xev.xconfigure.window;
			break;

		case ConfigureRequest:
			wnd = xev.xconfigurerequest.window;
			break;

		case CreateNotify:
			wnd = xev.xcreatewindow.window;
			break;

		case DestroyNotify:
			wnd = xev.xdestroywindow.window;
			break;

		case EnterNotify:
		case LeaveNotify:
			wnd = xev.xcrossing.subwindow;
			break;

		case GravityNotify:
			wnd = xev.xgravity.window;
			break;

		case KeyPress:
		case KeyRelease:
			wnd = xev.xkey.subwindow;
			break;

		case MapNotify:
			wnd = xev.xmap.window;
			break;

		case UnmapNotify:
			wnd = xev.xunmap.window;
			break;

		case MapRequest:
			wnd = xev.xmaprequest.window;
			break;

		case MotionNotify:
			wnd = xev.xmotion.subwindow;
			break;

		case ReparentNotify:
			wnd = xev.xreparent.window;
			break;

		case SelectionRequest:
			wnd = xev.xselectionrequest.requestor;
			break;

		default:
			wnd = xev.xany.window;
		}

		return WinHand(xev.xany.display,wnd);
	}

	virtual int x() const {
		int x;

		switch (xev.xany.type) {
		case ButtonPress:
		case ButtonRelease:
			x = xev.xbutton.x;
			break;

		case ConfigureNotify:
			x = xev.xconfigure.x;
			break;

		case ConfigureRequest:
			x = xev.xconfigurerequest.x;
			break;

		case CreateNotify:
			x = xev.xcreatewindow.x;
			break;

		case EnterNotify:
		case LeaveNotify:
			x = xev.xcrossing.x;
			break;

		case Expose:
			x = xev.xexpose.x;
			break;

		case GraphicsExpose:
			x = xev.xgraphicsexpose.x;
			break;

		case GravityNotify:
			x = xev.xgravity.x;
			break;

		case KeyPress:
		case KeyRelease:
			x = xev.xkey.x;
			break;

		case MotionNotify:
			x = xev.xmotion.x;
			break;

		case ReparentNotify:
			x = xev.xreparent.x;
			break;

		default:
			x = 0;
		}
		return x;
	}

	virtual int y() const {
		int y;

		switch (xev.xany.type) {
		case ButtonPress:
		case ButtonRelease:
			y = xev.xbutton.y;
			break;

		case ConfigureNotify:
			y = xev.xconfigure.y;
			break;

		case ConfigureRequest:
			y = xev.xconfigurerequest.y;
			break;

		case CreateNotify:
			y = xev.xcreatewindow.y;
			break;

		case EnterNotify:
		case LeaveNotify:
			y = xev.xcrossing.y;
			break;

		case Expose:
			y = xev.xexpose.y;
			break;

		case GraphicsExpose:
			y = xev.xgraphicsexpose.y;
			break;

		case GravityNotify:
			y = xev.xgravity.y;
			break;

		case KeyPress:
		case KeyRelease:
			y = xev.xkey.y;
			break;

		case MotionNotify:
			y = xev.xmotion.y;
			break;

		case ReparentNotify:
			y = xev.xreparent.y;
			break;

		default:
			y = 0;
		}
		return y;
	}

	virtual int w() const {
		int w;

		switch (xev.xany.type) {
		case ConfigureNotify:
			w = xev.xconfigure.width;
			break;

		case ConfigureRequest:
			w = xev.xconfigurerequest.width;
			break;

		case CreateNotify:
			w = xev.xcreatewindow.width;
			break;

		case Expose:
			w = xev.xexpose.width;
			break;

		case GraphicsExpose:
			w = xev.xgraphicsexpose.width;
			break;

		case ResizeRequest:
			w = xev.xresizerequest.width;
			break;

		default:
			w = 0;
		}
		return w;
	}

	virtual int h() const {
		int h;

		switch (xev.xany.type) {
		case ConfigureNotify:
			h = xev.xconfigure.height;
			break;

		case ConfigureRequest:
			h = xev.xconfigurerequest.height;
			break;

		case CreateNotify:
			h = xev.xcreatewindow.height;
			break;

		case Expose:
			h = xev.xexpose.height;
			break;

		case GraphicsExpose:
			h = xev.xgraphicsexpose.height;
			break;

		case ResizeRequest:
			h = xev.xresizerequest.height;
			break;

		default:
			h = 0;
		}
		return h;
	}

	virtual MouseButton button() const {
		if (xev.xany.type==ButtonPress || xev.xany.type==ButtonRelease)
			return (MouseButton)xev.xbutton.button;
		else
			return noButton;
	}

	virtual Modifier modifiers() const {
		int mod;
		switch (xev.xany.type) {
		case ButtonPress:
		case ButtonRelease:
			mod = xev.xbutton.state;
			break;

		case EnterNotify:
		case LeaveNotify:
			mod = xev.xcrossing.state;
			break;

		case KeyPress:
		case KeyRelease:
			mod = xev.xkey.state;
			break;

		case MotionNotify:
			mod = xev.xmotion.state;
			break;

		default:
			mod = ModNone;
		}
		return (Modifier)mod;
	}

	virtual const KeyNum keyNum() const {
		if (xev.xany.type==KeyPress || xev.xany.type==KeyRelease) {
			int keysyms_per_keycode_return;
			return KeyNum(static_cast<KeyNum::KeySym>(*XGetKeyboardMapping(xev.xany.display,xev.xkey.keycode,0,&keysyms_per_keycode_return)));
			// Deprecated return KeyNum(static_cast<KeyNum::KeySym>(XKeycodeToKeysym(xev.xany.display,xev.xkey.keycode,0)));
		} else
			return KeyNum(KeyNum::KS_NoSymbol);
	}

	virtual UserData userData() const {
		if (xev.xany.type==ClientMessage)
			return UserData(xev.xclient.format,xev.xclient.data.l);
		else
			return UserData();
	}

	XEvent		xev;

private:
	SystemEvent()
		: psevNext(0) {}

	static SystemEvent*	psevRoot;

	SystemEvent*	psevNext;
};

SystemEvent* SystemEvent::psevRoot = 0;


class ErrorEvent : public Event {
public:
	ErrorEvent(const char* message)
		: msg(message) {}

	~ErrorEvent() {}

	EVOrigin origin() const {
		return Failure;
	}
	EventType type() const {
		return EVError;
	}
	int subType() const {
		return BadImplementation;
	}
	const char* name() const {
		return msg;
	}

private:
	const char*		msg;
};

class SystemErrorEvent : public Event {
public:
	SystemErrorEvent(XErrorEvent* pxev)
		: pxerr(pxev) {
		pMsg = new char[maxErrorLen];
		XGetErrorText( pxev->display, pxev->error_code, pMsg, maxErrorLen );
	}
	~SystemErrorEvent() {
		delete [] pMsg;
	}

	EVOrigin origin() const {
		return SystemFailure;
	}
	EventType type() const {
		return EVError;
	}
	int subType() const {
		return pxerr->error_code;
	}
	const char* name() const {
		return pMsg;
	}

private:
	XErrorEvent*	pxerr;
	char*		pMsg;
};


/*: class DisplayHand

  This is an INTERNAL handle class for the connection to the
  display/keyboard/mouse.

  It should only be referenced by the BW framework itself, not application
  code.

  As a handle class, it can be copied, stored, and efficiently passed as a
  parameter.

  Usually only one display connection is created for an application.
*/

//
// Static members
//
WinProc DisplayHand::pwp = 0;
ErrorProc DisplayHand::pep = 0;
FatalErrorProc DisplayHand::pfep = 0;

//
// Error callbacks from Xlib
//
extern "C" int BWXIOErrorTrap( Display* pDisplay )
{
	DisplayHand::callFatalErrorProc(pDisplay);
	return 0;		// Keep compiler happy
}

extern "C" int BWXErrorEventTrap( Display* pDisplay, XErrorEvent* pxev )
{
	DisplayHand::callErrorProc(pDisplay, EventRef(new SystemErrorEvent(pxev)));
	return 0;
}


/*: function DisplayHand::setWinProc()

  Sets the window event processing function for all windows.
  A caller specific pointer is kept for each window
  and passed back to the WinProc so that events can be distributed
  appropriately.

  Prototype: void (*WinProc)(Figure*, EventRef);
  Prototype: static void DisplayHand::setWinProc(WinProc wp)

*/
void DisplayHand::setWinProc(WinProc wp)
{
	pwp = wp;
}


/*: function DisplayHand::setErrorHandler()

  This function sets the error handler for all display connections.

  A call to the error handler indicates a failure to execute a previously
  executed drawing call.  This could be because of resource exhaustion or
  an invalid parameter.  The call may show up much later than the original
  call.

  The error event passed back indicates the type of failure.

  The isolation layer attempts to reduce the probability of this error
  type through assertions on parameters (debug mode only) and exceptions
  thrown from resource intensive routines.

  It may be possible to continue after this type of error, but the error may
  reoccur and some expected resources (eg: fonts, pixmaps) may be unavailable.
*/
void DisplayHand::setErrorHandler(ErrorProc ep)
{
	pep = ep;
}


/*: function DisplayHand::setFatalErrorHandler()

  This function sets the fatal error handler for all display connections.

  A call to the fatal error handler indicates a loss of connection to
  the display.  Further graphics calls cannot be made.

  The most common cause of this error is a user initiated "kill" of an
  application window.

  The appropriate response to this error is an orderly shutdown of the
  application on the relevent display.
*/
void DisplayHand::setFatalErrorHandler(FatalErrorProc fep)
{
	pfep = fep;
}

/*: function DisplayHand::localeIsSet()

  This function sets the locale for the windowing system according to the
  operating system locale.

  The modifers parameter provides optional additional information about the
  locale (eg: Input method).

  This routine should be called after the "setlocale" C/C++ function.
  A false return indicates that the locale is not supported by the
  windowing system and the program should exit gracefully.
*/
bool DisplayHand::localeIsSet(const char *modifiers)
{
	bool ret = XSupportsLocale();
	XSetLocaleModifiers(modifiers);
	return ret;
}


/*: function DisplayHand::DisplayHand()

  Constructor.

  The default constructor creates an "not valid" display handle that can
  only be changed by assignment from a valid DisplayHand or by calling 'open'
  to initiate a display connection.
*/
DisplayHand::DisplayHand()
	: pd(0)
{
}

DisplayHand::DisplayHand(Display* pdis)
	: pd(pdis)
{
}

DisplayHand::~DisplayHand()
{
	close();
}


/*: function DisplayHand::open()

  Opens a connection to a display/keyboard/mouse.  A copy of argc and argc
  are passed which will be modified to delete standard parameters.

  The VisualType preference indicates the type of display expected by the
  application.  The isolation layer will make the best match possible.

  The default visual type is DynamicPalette.  A visual type of PrintOnly
  indicates that the application will use the print Graphics Contexts only --
  no connection is made to a display.

  Prototype: void DisplayHand::open(int& argc, char* argv[], VisualType pref=DynamicPalette, const char* displayName=0)
*/
void DisplayHand::open(int& argc, char* argv[], VisualType pref, const char* displayName)
{
	//
	// Things I do only when called for the first time
	//
	static bool isFirstCall = true;
	if (isFirstCall) {
		//
		// Set the X IO handlers.
		//
		XSetIOErrorHandler( BWXIOErrorTrap );
		(void)XSetErrorHandler( BWXErrorEventTrap );

		//
		// Get context handles for reverse lookups
		//
		ctxPFIG = XUniqueContext();

		//
		// Squirrel away some user settings
		//
		//assumedDPI = TheSite().getNumber( "Display", "AssumedDPI", 75 );
		assumedDPI = 100;  // Remove dependency on custom

		//
		// Pull out my command line arguments (for later)
		//
		// We understand -display and -geometry
		//
		if (argc>0) {
			int oldargc = argc;
			argc = 1;
			for (int i=1; i<oldargc; i++) {
				if (strcmp(argv[i],"-display")==0)
					displayName = argv[++i];
				else if (strcmp(argv[i],"-geometry")==0)
					pszGeometry = argv[++i];
				else
					argv[argc++] = argv[i];
			}
			argv[argc] = 0;
		}

		isFirstCall = false;
	}

	if (pref!=PrintOnly) {
		//
		// Open the display connection
		//
		pd = XOpenDisplay( displayName );
		if (!pd) {
			callFatalErrorProc(0);
		}

		//
		// Initialize a DisplayData structure
		//
		DisplayData* dd = new DisplayData();
		dd->focus = 0;
		dd->modal_root = 0;
		dd->isModal = false;
		if (!XInternAtoms( pd, const_cast<char**>(apszNames), 4, False, (Atom*)&(dd->atoms) ))
			callErrorProc(pd, new ErrorEvent("Cannot intern required atoms." ));

		int	maj, min;
		if (!XShapeQueryExtension( pd, &dd->shape.baseEvent, &dd->shape.baseError ))
			callErrorProc(pd, new ErrorEvent("Need SHAPE X extension." ));

		if (!XSyncQueryExtension( pd, &dd->sync.baseEvent, &dd->sync.baseError )
		        || !XSyncInitialize( pd, &maj, &min ))
			callErrorProc(pd, new ErrorEvent("Need SYNC X extension." ));

		ddMap[pd] = dd;

		//
		// Pick favourite visual
		//
		Visual*	pvisDefault = DefaultVisual(pd,DefaultScreen(pd));
		int nVisuals;
		XVisualInfo vi;
		VisualInfo* avi = (VisualInfo*)XGetVisualInfo(pd,0,&vi,&nVisuals);
		if (!avi)
			callFatalErrorProc(pd);

		VisualInfo* pvi = avi;
		//
		// Locate default visual and max depth
		//
		for (int i=0; i<nVisuals; i++) {
			if (pvisDefault==avi[i].visual)
				pvi = &avi[i];
		}

		switch (pref) {
		case Monochrome:
		case Grayscale:
		case FixedPalette:
			// I'll go with the default for all of these
			break;

		case DynamicPalette:
		case PrivatePalette:
			// If the default works, I'll use it
			if (pvi->c_class==PseudoColor || pvi->c_class==GrayScale)
				break;
			// OK, pick the best visual that works
			for (int i=0; i<nVisuals; i++) {
				switch (avi[i].c_class) {
				case PseudoColor:
					if (pvi->c_class!=PseudoColor || avi[i].depth>pvi->depth)
						pvi = &avi[i];
					break;
				case GrayScale:
					if (pvi->c_class==PseudoColor)
						break;
					if (pvi->c_class!=GrayScale || avi[i].depth>pvi->depth)
						pvi = &avi[i];
					break;
				default:
					break;
				}
			}
			break;

		case TrueColour:
			// If the default works, I'll use it
			if (pvi->c_class==TrueColor)
				break;
			// OK, pick the best visual that works
			for (int i=0; i<nVisuals; i++) {
				if (avi[i].c_class==TrueColor) {
					if (pvi->c_class!=TrueColor || avi[i].depth>pvi->depth)
						pvi = &avi[i];
				}
			}
			break;

		case PrintOnly:
			// Todo: No idea, take default
			break;
		}

		//
		// Get the colormap
		//
		// TODO: This should create a standards colormap if needed
		//
		Colormap cm;
		if (pref==PrivatePalette || pvisDefault!=pvi->visual)
			cm = XCreateColormap(pd, DefaultRootWindow(pd), pvi->visual, AllocNone);
		else
			cm = DefaultColormap(pd,DefaultScreen(pd));

		//
		// Create my Palette
		//
		new PhysicalPalette(pd, pvi, cm, pref);

		XFree(avi);

		//
		// Make sure the cut buffers exist
		//
		for (int i=0; i<8; i++) {
			XChangeProperty
			(
			    pd,				// Display
			    DefaultRootWindow(pd),	// Window
			    aCutBufferList[i],		// Atom
			    XA_STRING,			// Type
			    8,				// Format
			    PropModeAppend,		// Action
			    0,				// Buffer
			    0				// Length
			);
		}

	}
};

/*: function DisplayHand::close()

  Closes a display connection and sets the handle to "invalid".

  Note that other copies of the original handle remain valid, but their use
  is undefined.
*/
void DisplayHand::close()
{
	if (pd) {
		// TODO: XFreeGC
		// TODO: XUnloadFont
		// TODO: XDestroyWindow

		delete PhysicalPalette::find(pd);
		XCloseDisplay(pd);
		pd = 0;
	}
}

/*: function DisplayHand::isValid()

  Returns true if the handle is valid.  I.E. If the handle has been opened or
  is a copy of an open handle.
*/
bool DisplayHand::isValid()
{
	return pd!=0;
}

/*: function DisplayHand::planeMask()

  Returns a mask for significant bits in Pixel values.
*/
long DisplayHand::planeMask()
{
	bwassert(pd);
	VisualInfo* pvi = (VisualInfo*)PhysicalPalette::find(pd)->pvi;
	return pvi->red_mask | pvi->blue_mask | pvi->green_mask;
}

/*: function DisplayHand::numPlanes()

  Returns the number of active bits in pixel values.
*/
int DisplayHand::numPlanes()
{
	bwassert(pd);
	return PhysicalPalette::find(pd)->pvi->depth;
}

/*: function DisplayHand::screenName()

  Returns a printable string with the name of the connected display.
*/
const char& DisplayHand::screenName()
{
	return *DisplayString(pd);
}

/*: function DisplayHand::rootWindow()

  Returns the root window for the display.
*/
WinHand DisplayHand::rootWindow()
{
	return WinHand(pd,DefaultRootWindow(pd));
}


/*: function DisplayHand::screenHeight()

  Returns the display height in pixels.
*/
int DisplayHand::screenHeight()
{
	return DisplayHeight(pd,DefaultScreen(pd));
}


/*: function DisplayHand::screenWidth()

  Returns the display width in pixels.
*/
int DisplayHand::screenWidth()
{
	return DisplayWidth(pd,DefaultScreen(pd));
}

/*: function DisplayHand::assumedPixelsPerMeter()

  Returns the assumed display resolution.
*/
int DisplayHand::assumedPixelsPerMeter()
{
	return (assumedDPI*394 + 5) / 10;
}


/*: function DisplayHand::assumedPixelsPerInch()

  Returns the assumed display resolution
*/
int DisplayHand::assumedPixelsPerInch()
{
	return assumedDPI;
}


/*: function DisplayHand::assumedScreenHeightMM()

  Returns the assumed height of the display.
*/
int DisplayHand::assumedScreenHeightMM()
{
	return DisplayHeight(pd,DefaultScreen(pd))*10000 / (assumedDPI*394);
}


/*: function DisplayHand::assumedScreenWidthMM()

  Returns the assumed width of the display.
*/
int DisplayHand::assumedScreenWidthMM()
{
	return DisplayWidth(pd,DefaultScreen(pd))*10000 / (assumedDPI*394);
}

/*: function DisplayHand::colormapSize()

  Returns the size of the colormap.  I.E. the number of different colours
  that can be displayed at the same time.
*/
int DisplayHand::colormapSize()
{
	bwassert(pd);
	return PhysicalPalette::find(pd)->pvi->colormap_size;
}


/* function DisplayHand::typeOfVisual()

   Returns the visual type of the display connection.  One of:
   <ul>
   <li>PrintOnly
   <li>Monochrome
   <li>Grayscale
   <li>FixedPalette
   <li>DynamicPalette
   <li>PrivatePalette
   <li>TrueColour
   </ul>
*/
VisualType DisplayHand::typeOfVisual()
{
	// TODO: PrintOnly DisplayHand?
	bwassert(pd);
	return PhysicalPalette::find(pd)->visualType();
}

/*: function DisplayHand::newFrameWindow()

  Creates a new Frame (root level) window for the application.

  The EventMask indicates which types of events should be generated for this
  window.  It is any combination of:
  <ul>
  <li>EMNone
  <li>EMKeyPress
  <li>EMKeyRelease
  <li>EMButtonPress
  <li>EMButtonRelease
  <li>EMEnterWindow
  <li>EMLeaveWindow
  <li>EMMouseMotion
  <li>EMButtonMotion
  <li>EMExposure
  <li>EMVisibility
  <li>EMStructure
  <li>EMSubstructure
  <li>EMFocus
  </ul>

  The Figure* parameter is stored transparently for subsequent event dispatch.

  The AppName parameter indicates the name that should appear on the title
  line for the window and for it's icon (if any).

  The other parameters specify the recommended size and location of the window.
  These parameters can be overriden from command line parameters and/or the
  windowing system.
*/
WinHand DisplayHand::newFrameWindow
(
    EventMask em,
    Figure* pfig,
    const char* pszAppName,
    int x, int y, int w, int h
)
{
	//
	// Fill in XSetWindowAttributes
	//
	XSetWindowAttributes xswa;
	unsigned long	xswaMask;
	PhysicalPalette* ppp = PhysicalPalette::find(pd);
	bwassert(ppp);
	xswa.colormap = ppp->cm;
	xswa.event_mask = em;
	xswa.cursor = XCreateFontCursor(pd,XC_top_left_arrow);
	xswaMask = CWColormap | CWEventMask | CWCursor;

	//
	// Has the user overriden the geometry?
	//
	int fGeoSet = 0;
	if (pszGeometry) {
		unsigned int uw, uh;
		fGeoSet = XParseGeometry(pszGeometry, &x, &y, &uw, &uh);
		if(fGeoSet&WidthValue)
			w = uw;
		if(fGeoSet&HeightValue)
			h = uh;
		if (x<0)
			x += DisplayWidth(pd,DefaultScreen(pd));
		if (y<0)
			y += DisplayHeight(pd,DefaultScreen(pd));
		pszGeometry = 0;	// Only sets first window
	}

	//
	// Create the window
	//
//    trace << "Creating window:" << endl;
//    trace << "\tm_pDisplay = " << m_pfi->m_pDisplay << endl;
//    trace << "\tm_wndParent = " << m_pfi->m_wndParent << endl;
//    trace << "\tm_x = " << m_pfi->m_x << ",  m_y = " << m_pfi->m_y << endl;
//    trace << "\tm_width = " << m_pfi->m_width << ",  m_height = " << m_pfi->m_height << endl;
//    trace << "\tDepth = " << CopyFromParent << endl;
//    trace << "\tClass = " << InputOutput << endl;
//    trace << "\tVisual = " << (Visual*)CopyFromParent << endl;
//    trace << "\tValue Mask = " << evr.retcode << endl;
//    trace << "\t&xswa = " << &(m_pfi->m_xswa) << endl << endl;

	Window wnd = XCreateWindow( pd,			// Display
	                            DefaultRootWindow(pd),	// Parent
	                            x, y, w, h,		// Geometry
	                            0,			// Border width
	                            ppp->pvi->depth,	// Depth
	                            InputOutput,		// Class
	                            ppp->pvi->visual,	// Visual
	                            xswaMask,		// Value mask
	                            &xswa			// Values
	                          );
	//
	// Reverse pointer so Figure can be found
	//
	if (XSaveContext(pd, wnd, ctxPFIG, (char*)pfig))
		callFatalErrorProc(pd);

	//
	// Set window manager "hints" since this is a frame window
	//

	// Window name
	XTextProperty tpName;
	XStringListToTextProperty((char**)&pszAppName, 1, &tpName);

	// Icon name
	XTextProperty tpIconName;
	XStringListToTextProperty((char**)&pszAppName, 1, &tpIconName);

	// Size Hints
	XSizeHints*	psh = XAllocSizeHints();
	psh->x = x;
	psh->y = y;
	psh->width = w;
	psh->height = h;

	if (fGeoSet&(XValue|YValue))
		psh->flags = USSize;
	else
		psh->flags = PSize;

	if (fGeoSet&(WidthValue|HeightValue))
		psh->flags |= USPosition;
	else
		psh->flags |= PPosition;

	// Window Manager Hints
	XWMHints*  pwmh = XAllocWMHints();
	pwmh->input = True;
	pwmh->initial_state = NormalState;
	pwmh->flags = StateHint | InputHint;

	// Class Hints
	XClassHint* pclh = XAllocClassHint();
	char	pszRes[maxResLength+1];
	char	pszClass[maxResLength+1];

	for (int i=0; i<maxResLength; i++) {
		if (!(pszRes[i] = pszClass[i] = tolower(pszAppName[i])))
			break;
	}
	pszRes[maxResLength] = pszClass[maxResLength] = '\0';
	pszClass[0] = toupper(pszClass[0]);

	pclh->res_name = pszRes;
	pclh->res_class = pszClass;

	XSetWMProperties(
	    pd,
	    wnd,
	    &tpName,
	    &tpIconName,
	    0, 0,			// Argv, argc
	    psh,
	    pwmh,
	    pclh );

	XFree( tpName.value );
	XFree( tpIconName.value );
	XFree( pclh );
	XFree( pwmh );
	XFree( psh );

	//
	// Participate in some WM protocols (Delete & Take Focus)
	//
	if (!XSetWMProtocols( pd, wnd, &ddMap[pd]->atoms.aDelete, 2 ))
		callErrorProc(pd, new ErrorEvent("Cannot set WM Protocols." ));

	return WinHand(pd,wnd);
}

/*: function DisplayHand::getScreenPalette()

  Returns the physical palette for the display.

  The physical palette can be used to interrogate and change the display
  palette or the private palette for the application.
*/
PhysicalPalette& DisplayHand::getScreenPalette()
{
	return *PhysicalPalette::find(pd);
}

/* function DisplayHand::newPixmap()

   Creates a new pixmap (drawable) for the display.

   A pixmap is a drawable area managed by the windowing system with
   the same pixel characteristics as the display.

   Pixmaps can be copied to and from the display.
*/
PixmapHand DisplayHand::newPixmap(int w, int h)
{
	return PixmapHand(pd,
	                  XCreatePixmap( pd, DefaultRootWindow(pd), w, h,
	                                 PhysicalPalette::find(pd)->pvi->depth ),
	                  w, h
	                 );
}

/*: function DisplayHand::newBitmap()

  Creates a new bitmap (drawable) for the display.

  A bitmap is drawable area with binary pixels managed by the windowing
  system.  Bitmaps can be copied onto the display or used for pixel
  masking operations.
*/
BitmapHand DisplayHand::newBitmap(int w, int h)
{
	return BitmapHand(pd,
	                  XCreatePixmap( pd, DefaultRootWindow(pd), w, h, 1 ),
	                  w, h
	                 );
}

/*: function DisplayHand::newPrinter()

  Creates a Printer Handle with characteristics that match the display.

*/
//PrinterHand DisplayHand::newPrinter(const char* name, PrintQuality)
//{
//    return PrinterHand();		// TODO: Printing
//}

/* function DisplayHand::newFont()

   Loads a font into the font palette for the display.  The font that most
   closelt matches the font specification is loaded.
*/
FontHand DisplayHand::newFont(const FontSpec& fs)
{
	char	fn[200];
	makeFontName(fn,fs);

	char **missing_list;
	int missing_count;
	char *def_string;

	void* pfs = (void*)XCreateFontSet(pd,fn,&missing_list,&missing_count,&def_string);
	XFreeStringList(missing_list);

	if (pfs)
		return FontHand(pd, pfs);
	return FontHand();
}

/* function DisplayHand::findFonts()

   Finds a list of fonts from those available for the display that match the
   font specification.

*/
std::set<FontSpec> DisplayHand::findFonts(const FontSpec& pattern)
{
	std::set<FontSpec> setfs;

	char	fn[200];
	makeFontName(fn,pattern);

	int nFonts;
	char **fontv = XListFonts(pd,fn,maxFonts,&nFonts);

	for (int i=0; i<nFonts; i++)
		setfs.insert(FontSpec(fontv[i]));

	return setfs;
}

/*: function DisplayHand::newCursor()

  Creates a new cursor for the display from a list of predefined types.
*/
CursorHand DisplayHand::newCursor(DefinedCursor cs)
{
	return CursorHand(pd,XCreateFontCursor(pd,cs));
}

/*: function DisplayHand::flush()

  Causes all queued display functions to be initiated.
*/
void DisplayHand::flush()
{
	XFlush(pd);
}

/*: function DisplayHand::sync()

  Causes all queued display functions to be executed immediately and waits
  for completion.
*/
void DisplayHand::sync()
{
	XSync(pd,False);
}

/*: function DisplayHand::countEventsPending()

  Returns an approximate count of the number of events pending.
*/
int DisplayHand::countEventsPending()
{
	// Note: result is only approximate due to event filtering
	return XEventsQueued(pd,QueuedAlready);
}

/*: function DisplayHand::nextEvent()

  Get next pending event for dispatching.  This routine may block waiting for
  user input.
*/
EventRef DisplayHand::nextEvent()
{
	//
	// Get event
	//
	SystemEvent* psev = SystemEvent::newSystemEvent();
	XNextEvent(pd,&(psev->xev));

	//
	// If it's a Keyboard event, fix X's focus problems (this is simpler than using
	// grabkey on Tab and other focus changing keys)
	//
	Window fwin = 0;
	if (psev->xev.xany.type==KeyPress || psev->xev.xany.type==KeyRelease )
		fwin = ddMap[pd]->focus;

	//
	// Allow input method first crack at the message
	//
	if (XFilterEvent(&(psev->xev),fwin)) {
		psev->xev.xany.type = NoEvent;
		return EventRef(psev);
	}

	//
	// Modal input filter
	//
	// At the moment, I'm assuming nothing needs to be done here, but I might need
	// to filter focus in events in future.  I may also need to record focus in
	// messages to keep my understanding of where the focus is consistent with X's
	//

	//
	// Events we handle ourselves
	//
	if (psev->xev.xany.type==MappingNotify) {
		XRefreshKeyboardMapping( &(psev->xev.xmapping) );
		psev->xev.xany.type = NoEvent;
	}

	//
	// We modify certain events
	//
	if (fwin)
		psev->xev.xkey.window = fwin;

	if (psev->xev.xany.type==ddMap[pd]->sync.baseEvent+XSyncAlarmNotify) {
		// Keep those cards and letters coming
		XSyncAlarmAttributes xsaa;
		xsaa.events = True;
		XSyncAlarmNotifyEvent* pxsan = (XSyncAlarmNotifyEvent*)&psev->xev;
		XSyncChangeAlarm( pd, pxsan->alarm, XSyncCAEvents, &xsaa );

		// Adjust event type
		psev->xev.xany.type = EVAlarm;		// Note: no details available
	} else if (psev->xev.xany.type>MappingNotify)
		psev->xev.xany.type = EVExtensionEvent;	// Note: can't tell which one

	if (psev->xev.xany.type==ClientMessage
	        && psev->xev.xclient.data.l[0]==static_cast<long>(ddMap[pd]->atoms.aDelete)) {
		psev->xev.xany.type = EVCloseRequest;
	}

	if (psev->xev.xany.type==ClientMessage
	        && psev->xev.xclient.data.l[0]==static_cast<long>(ddMap[pd]->atoms.aFocus)) {
		XSetInputFocus(
		    pd, 				// Display
		    ddMap[pd]->focus, 			// New focus window
		    RevertToParent, 			// Revert if focus window disappears
		    psev->xev.xclient.data.l[1] );	// Timestamp
		psev->xev.xany.type = NoEvent;
	}

	// Note: Other Client Messages are not really useful (since Atoms are not exposed)

	return EventRef(psev);
}


void DisplayHand::addTimer(int msec)
{
	int nCounters;
	XSyncSystemCounter* pxsc = XSyncListSystemCounters( pd, &nCounters);
	static XSyncCounter theCounter = 0;

	if (!theCounter) {
		for (int i=0; i<nCounters; i++) {
			if (strcmp("SERVERTIME",pxsc[i].name)==0)
				theCounter = pxsc[i].counter;
		}
		if(!theCounter)
			callErrorProc(pd, new ErrorEvent("Cannot find SERVERTIME counter"));

		XSyncFreeSystemCounterList(pxsc);
	}

	XSyncAlarmAttributes xsaa;
	xsaa.trigger.counter = theCounter;
	xsaa.trigger.value_type = XSyncAbsolute;
	XSyncQueryCounter(pd,theCounter,&xsaa.trigger.wait_value);
	xsaa.trigger.test_type = XSyncPositiveComparison;
	xsaa.delta.hi = 0;
	xsaa.delta.lo = msec;
	xsaa.events = True;

	if (None==XSyncCreateAlarm
	        (
	            pd,
	            XSyncCACounter | XSyncCAValueType | XSyncCAValue
	            | XSyncCATestType | XSyncCADelta | XSyncCAEvents,
	            &xsaa
	        ))
		callErrorProc(pd, new ErrorEvent("Cannot create alarm."));
}


void DisplayHand::sendEvent(const EventRef rev)
{
	// Only ClientMessage events can be sent
	bwassert( rev->type()==EVClientMessage );

	XEvent xev;

	xev.xclient.type = ClientMessage;
	xev.xclient.send_event = True;
	xev.xclient.display = pd;
	xev.xclient.window = rev->destination().wnd;
	xev.xclient.message_type = (Atom)rev->subType();	// Note: no way to get Atom
	Event::UserData ud = rev->userData();
	xev.xclient.format = ud.format;
	for (int i=0; i<5; i++)
		xev.xclient.data.l[i] = ud.data.l[i];

	XSendEvent( pd, xev.xclient.window, True, 0, &xev );
}


void DisplayHand::dispatchEvent(const EventRef rev)
{
	Figure* pfig;
	bwassert( pwp );
	if (!XFindContext(pd, rev->destination().wnd, ctxPFIG, (char**)&pfig)) {
		(*pwp)(pfig,rev);
	}
}


void DisplayHand::setTextClipboard(const char* buf)
{
	// These are really mickey mouse routines.  Later, I need to support:
	//		active selections
	//		Motif/Primary clipboards
	//		Multiple and client defined data types
	//		Drag and drop

	XRotateBuffers( pd, +1 );
	XStoreBytes( pd, buf, strlen(buf) );
}


int DisplayHand::getTextClipboard(char* buf, int buflen)
{
	int	nBytes;
	char* pin = XFetchBytes( pd, &nBytes );
	if (nBytes>buflen-1)
		nBytes = buflen-1;
	strncpy(buf,pin,nBytes);
	buf[buflen-1] = '\0';
	return nBytes;
}


void DisplayHand::undoTextClipboard()
{
	XRotateBuffers( pd, -1 );
}


void DisplayHand::callErrorProc(Display* pd, EventRef evr)
{
	bwassert( pep );
	(*pep)(DisplayHand(pd),evr);
}


void DisplayHand::callFatalErrorProc(Display* pd)
{
	bwassert( pfep );
	(*pfep)(DisplayHand(pd));
}


void DisplayHand::makeFontName(char* fn,const FontSpec& fs)
{
	strcpy(fn,"-");

	if (*fs.foundry())
		strcat(fn,fs.foundry());
	else
		strcat(fn,"*");
	strcat(fn,"-");

	if (*fs.family())
		//strcat(fn,TheSite().getString("FontMap",fs.family(),fs.family()));
		strcat(fn,fs.family()); //Remove dependency on custom
	else
		strcat(fn,"*");
	strcat(fn,"-");

	switch (fs.weight()) {
	case FontSpec::AnyWeight:
		strcat(fn,"*");
		break;

	case FontSpec::Medium:
		strcat(fn,"medium");
		break;

	case FontSpec::Bold:
		strcat(fn,"bold");
		break;
	}
	strcat(fn,"-");

	switch (fs.slant()) {
	case FontSpec::AnySlant:
		strcat(fn,"*");
		break;

	case FontSpec::Roman:
		strcat(fn,"r");
		break;

	case FontSpec::Italic:
		strcat(fn,"i");
		break;

	case FontSpec::Oblique:
		strcat(fn,"o");
		break;
	}
	strcat(fn,"-*-*-*-");		// SetWidth, adstyle, pixels

	if (fs.tenPoints()) {
		char psz[20];
		sprintf(psz,"%d",fs.tenPoints());
		strcat(fn,psz);
		strcat(fn,"-");

		sprintf(psz,"%ld",assumedDPI);
		strcat(fn,psz);
		strcat(fn,"-");
		strcat(fn,psz);
	} else
		strcat(fn,"*-*-*");

	strcat(fn,"-*-*-");			// Spacing, average width

	if (*fs.charset())
		strcat(fn,fs.charset());
	else
		strcat(fn,"*-*");
}

//-----------------------------------WinHand------------------------------------

WinHand::WinHand()
	: pd(0), wnd(0)
{}


WinHand::WinHand(Display* pDisplay, unsigned long window)
	: pd(pDisplay), wnd(window)
{}


void WinHand::destroy()
{
	bwassert(wnd);
	if (wnd==ddMap[pd]->modal_root) {
		hide();
		ddMap[pd]->modal_root = 0;
	}
	XDestroyWindow(pd,wnd);
	pd = 0;
	wnd = 0;
}



bool WinHand::isValid()
{
	return wnd!=0;
}


void WinHand::move(int x, int y)
{
	bwassert(wnd);
	XMoveWindow(pd,wnd,x,y);
}


void WinHand::resize(int w, int h)
{
	bwassert(wnd);
	bwassert(w>0);
	bwassert(h>0);
	XResizeWindow(pd,wnd,w,h);
}


void WinHand::reshape(RegionHand rh)
{
	bwassert(wnd);
	// TODO: Maybe bwassert(rh.isValid());
	XShapeCombineRegion(pd,wnd,ShapeClip,0,0,rh.reg,ShapeSet);
}


void WinHand::moveresize(int x, int y, int w, int h)
{
	bwassert(wnd);
	bwassert(w>0);
	bwassert(h>0);
	XMoveResizeWindow(pd,wnd,x,y,w,h);
}


void WinHand::show()
{
	bwassert(wnd);
	XMapWindow(pd,wnd);

	if (wnd==ddMap[pd]->modal_root) {
		// This might be a little harsh (and doesn't really work)
		// TODO: Eat ButtonPress and ButtonRelease events when Modal
		// This means that I need a quick way to tell if a window is part
		// of the current scene.  Also, note that errors are not checked here
		XGrabPointer(pd,wnd,False,0,GrabModeAsync,GrabModeAsync,None,None,CurrentTime);
	}
}


void WinHand::hide()
{
	bwassert(wnd);
	if (wnd==ddMap[pd]->modal_root) {
		XUngrabPointer(pd,CurrentTime);
	}
	XUnmapWindow(pd,wnd);
}


void WinHand::raise()
{
	bwassert(wnd);
	XRaiseWindow(pd,wnd);
}


void WinHand::lower()
{
	bwassert(wnd);
	XLowerWindow(pd,wnd);
}


void WinHand::setBackground(Pixel pxl)
{
	bwassert(wnd);
	XSetWindowBackground(pd,wnd,pxl);
}


void WinHand::setBackground(PixmapHand pmh)
{
	bwassert(wnd);
	bwassert(pmh.isValid());
	XSetWindowBackgroundPixmap(pd,wnd,pmh.m_pixmap);
}


void WinHand::unsetBackground()
{
	bwassert(wnd);
	XSetWindowBackgroundPixmap(pd,wnd,None);
}


void WinHand::setBackgroundToParent()
{
	bwassert(wnd);
	XSetWindowBackgroundPixmap(pd,wnd,ParentRelative);
}


void WinHand::setEventMask(EventMask em)
{
	bwassert(wnd);
	XSetWindowAttributes xswa;
	xswa.event_mask = em;
	XChangeWindowAttributes(pd,wnd,CWEventMask,&xswa);
}


void WinHand::setProcArg(Figure* pfig)
{
	bwassert(wnd);
	XSaveContext(pd,wnd,ctxPFIG,(XPointer)pfig);
}


Figure* WinHand::getProcArg()
{
	bwassert(wnd);
	Figure* pfig;
	XFindContext(pd,wnd,ctxPFIG,(XPointer*)&pfig);
	return pfig;
}


GCHand WinHand::newGC(const GCSpec& gcs)
{
	return GCHand(XGC::fetchGC(pd,wnd,gcs));
}


void WinHand::getImage(int x, int y, int w, int h, PImage& img)
{
	// TODO
	bwassert(0);
	// img.setImage(XGetImage(pd,wnd,x,y,w,h,AllPlanes,ZPixmap));
}

void WinHand::getSubImage(
    int x, int y, int w, int h,
    PImage& img,
    int destX, int destY)
{
	bwassert(wnd);
	// TODO: asserts about image size
	(void)XGetSubImage(pd,wnd,x,y,w,h,AllPlanes,ZPixmap,img.ximg,destX,destY);
}

void WinHand::setCursor(CursorHand chnd)
{
	bwassert(wnd);
	XSetWindowAttributes xswa;
	xswa.cursor = chnd.cursor;
	XChangeWindowAttributes(pd,wnd,CWCursor,&xswa);
}

void WinHand::grabPointer(EventMask em, CursorHand chnd)
{
	bwassert(wnd);
	XGrabPointer(pd,wnd,False,em,
	             GrabModeAsync,GrabModeAsync,None,
	             chnd.cursor,CurrentTime);
}

void WinHand::releasePointer()
{
	bwassert(wnd);
	XUngrabPointer(pd,CurrentTime);
}


void WinHand::grabKeyboard()
{
	bwassert(wnd);
	XGrabKeyboard(pd,wnd,False,
	              GrabModeAsync,GrabModeAsync,CurrentTime);
}


void WinHand::releaseKeyboard()
{
	bwassert(wnd);
	XUngrabKeyboard(pd,CurrentTime);
}

void WinHand::takeFocus()
{
	bwassert(wnd);
	ddMap[pd]->focus = wnd;
	if (Window modwnd = ddMap[pd]->modal_root) {
		// There is a modal window, focus can only be set to one
		// of it's descendants
		if (!isAncestor(modwnd,wnd))
			return;
	}
	// Will happen when window manager talks to us.
	// XSetInputFocus(pd,wnd,RevertToParent,CurrentTime);
}


WinHand WinHand::newChild(EventMask evm, Figure* pfig, int x, int y, int w, int h)
{
	XSetWindowAttributes xswa;
	unsigned long xswamask;
	xswa.event_mask = evm;
	xswamask = CWEventMask;
	Window wndChild = XCreateWindow(
	                      pd,
	                      wnd,
	                      x,y,
	                      w,h,
	                      0,
	                      CopyFromParent,
	                      InputOutput,
	                      CopyFromParent,
	                      xswamask,
	                      &xswa
	                  );
	//
	// Reverse pointer so Figure can be found
	//
	if (XSaveContext(pd, wndChild, ctxPFIG, (char*)pfig))
		DisplayHand::callFatalErrorProc(pd);

	return WinHand(pd,wndChild);
}


WinHand WinHand::newChild(EventMask evm, Figure* pfig, int x, int y, int w, int h, RegionHand rh)
{
	WinHand whnd = newChild(evm,pfig,x,y,w,h);
	XShapeCombineRegion(pd,whnd.wnd,ShapeClip,0,0,rh.reg,ShapeSet);
	return whnd;
}

WinHand WinHand::newPopup(EventMask evm, Figure* pfig, int x, int y, int w, int h)
{
	XSetWindowAttributes xswa;
	unsigned long xswamask;
	xswa.event_mask = evm;
	xswa.save_under = True;
	xswa.override_redirect = True;
	xswamask = CWEventMask | CWSaveUnder | CWOverrideRedirect;
	return WinHand(pd,XCreateWindow(
	                   pd,
	                   wnd,
	                   x,y,
	                   w,h,
	                   0,
	                   CopyFromParent,
	                   InputOutput,
	                   CopyFromParent,
	                   xswamask,
	                   &xswa
	               ));
	//
	// Reverse pointer so Figure can be found
	//
	if (XSaveContext(pd, wnd, ctxPFIG, (char*)pfig))
		DisplayHand::callFatalErrorProc(pd);
}


WinHand WinHand::newPopup(EventMask evm, Figure* pfig, int x, int y, int w, int h, RegionHand rh)
{
	WinHand whnd = newPopup(evm,pfig,x,y,w,h);
	XShapeCombineRegion(pd,whnd.wnd,ShapeClip,0,0,rh.reg,ShapeSet);
	return whnd;
}


WinHand WinHand::newDialog(EventMask evm, Figure* pfig, Modality fModal,
                           int x, int y, int w, int h)
{
	// This makes a frame window as a child of the root with Modality
	// TODO: Modality
	WinHand whnd = newPopup(evm,pfig,x,y,w,h);
	return whnd;
}


void WinHand::setTitle(const char* pszTitle)
{
	XStoreName( pd, wnd, pszTitle );
}

void WinHand::setMinSize(int w, int h)
{
	// TODO: Verify if we really need to read first
	XSizeHints* psh = XAllocSizeHints();
	long user_set;
	if (XGetWMNormalHints( pd, wnd, psh, &user_set )) {
		psh->min_width = w;
		psh->min_height = h;
		psh->flags |= PMinSize;
		XSetWMNormalHints( pd, wnd, psh );
	}
	XFree(psh);
}


void WinHand::setMaxSize(int w, int h)
{
	// TODO: Verify if we really need to read first
	XSizeHints* psh = XAllocSizeHints();
	long user_set;
	if (XGetWMNormalHints( pd, wnd, psh, &user_set )) {
		psh->max_width = w;
		psh->max_height = h;
		psh->flags |= PMaxSize;
		XSetWMNormalHints( pd, wnd, psh );
	}
	XFree(psh);
}

// Frame Windows only
WinHand WinHand::newSecondary(EventMask em, Figure* pfig, const char* pszTitle,
                              int x, int y, int w, int h)
{
	// TODO: Link new window back to original
	DisplayHand dh(pd);
	return dh.newFrameWindow( em, pfig, pszTitle, x, y, w, h );
}

//WinHand WinHand::newIconWindow(EventMask, Figure* pfig)
//void WinHand::setIconTitle(const char*)

bool WinHand::isAncestor(unsigned long anc, unsigned long dec)
{
	// TODO: Verify that this isn't too slow

	Window root = 0;
	Window parent;
	Window **children = 0;
	unsigned int nchildren;
	Window cur = dec;
	while (cur!=root) {
		XQueryTree( pd, cur, &root, &parent, children, &nchildren);
		if (nchildren)
			XFree(children);
		if (parent==anc)
			return true;
		cur = parent;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////
// PhysicalPalette

PhysicalPalette* PhysicalPalette::pPalFirst = 0;

PhysicalPalette::PhysicalPalette(Display* pDis, VisualInfo* pvis, unsigned long cmap, VisualType)
	:	pd(pDis), pvi(0), cm(cmap)
{
	// Insert into chain
	pPalNext = pPalFirst;
	pPalFirst = this;

	// Allocate VisualInfo structure
	pvi = new VisualInfo(*pvis);
}

PhysicalPalette::~PhysicalPalette()
{
	// Delete copy of VisualInfo
	delete pvi;

	// Remove from chain
	if (pPalFirst==this) {
		pPalFirst = pPalNext;
	} else {
		for (PhysicalPalette* ppp=pPalFirst; ppp->pPalNext; ppp=ppp->pPalNext) {
			if (this==ppp->pPalNext) {
				ppp->pPalNext = pPalNext;
				break;
			}
		}
	}
}

PhysicalPalette* PhysicalPalette::find(Display* pDis)
{
	PhysicalPalette* ppp;
	for (ppp=pPalFirst; ppp; ppp=ppp->pPalNext)
		if (ppp->pd == pDis)
			break;
	return ppp;
}

VisualType PhysicalPalette::visualType()
{
	VisualType ret = Monochrome;

	switch (pvi->c_class) {
	case GrayScale:
	case StaticGray:
		if (pvi->depth==1)
			ret = Monochrome;
		else
			ret = Grayscale;
		break;

	case PseudoColor:
		ret = DynamicPalette;

	case StaticColor:
		ret = FixedPalette;
		break;

	case DirectColor:
	case TrueColor:
		ret = TrueColour;
	}
	return ret;
}


int PhysicalPalette::pixelRange() const
{
	return 1 << pvi->depth;
}


int PhysicalPalette::numColours() const
{
	return pvi->colormap_size;
}

Colour PhysicalPalette::operator[](Pixel pix) const
{
	// TODO: we should really have a better idea and query the server less
	XColor xc;
	xc.pixel = pix;
	XQueryColor(pd,cm,&xc);
	return Colour(xc.red,xc.green,xc.blue);
}


Pixel PhysicalPalette::lookup(Colour c) const
{
	// TODO: This is a stub
	// TODO: All these case statements indicates that we should have subclasses

	Pixel ret=0;

	switch (pvi->c_class) {
	case GrayScale:
	case StaticGray:
		ret = (19661UL*c.red + 38667UL*c.green + 7209UL*c.blue) >> (32-pvi->depth);
		break;

	case PseudoColor:
	case StaticColor:
		// TODO: Seriously not supported (we pretend its 3/3/2)
		ret = ((c.red>>8)&0xe0) | ((c.green>>11)&0x1c) | ((c.blue>>14)&0x03);
		break;

	case DirectColor:
	case TrueColor:
		// TODO: Assumes fixed masks and 24bit (need to compute shifts from masks)
		ret = ((c.red<<8)&0xff0000) | ((c.green)&0xff00) | ((c.blue>>8)&0xff);
		break;
	}

	return ret;
}


Pixel PhysicalPalette::allocate(Colour c)
{
	XColor xc;
	xc.red = c.red;
	xc.green = c.green;
	xc.blue = c.blue;
	xc.flags = DoRed|DoGreen|DoBlue;

	if (!XAllocColor(pd,cm,&xc))
		throw "Colour allocation failed.";	// TODO: Handle error better

	return xc.pixel;
}

Pixel PhysicalPalette::allocateDynamic(Colour c)
{
	XColor xc;
	xc.red = c.red;
	xc.green = c.green;
	xc.blue = c.blue;
	xc.flags = DoRed|DoGreen|DoBlue;

	XAllocColorCells(pd,cm,False,0,0,&xc.pixel,1);

	XStoreColor(pd,cm,&xc);

	return xc.pixel;
}


void PhysicalPalette::set(Pixel pix, Colour c)
{
	XColor xc;
	xc.pixel = pix;
	xc.red = c.red;
	xc.green = c.green;
	xc.blue = c.blue;
	xc.flags = DoRed|DoGreen|DoBlue;

	XStoreColor(pd,cm,&xc);
}


//////////////////////////////////////////////////////////////////
//
// PixmapHand
//

PixmapHand::PixmapHand(Display* pdis,unsigned long pixmap, int w, int h)
	:	m_pDisplay(pdis), m_pixmap(pixmap), m_w(w), m_h(h)
{}


PixmapHand::PixmapHand()
	:	m_pDisplay(0), m_pixmap(0), m_w(0), m_h(0)
{}


void PixmapHand::destroy()
{
	XFreePixmap(m_pDisplay,m_pixmap);
	m_pixmap = 0;
}


bool PixmapHand::isValid()
{
	return m_pixmap!=0;
}


int PixmapHand::w()
{
	return m_w;
}


int PixmapHand::h()
{
	return m_h;
}


GCHand PixmapHand::newGC(const GCSpec& gcs)
{
	return GCHand(XGC::fetchGC(m_pDisplay,m_pixmap,gcs));
}


void PixmapHand::getImage(int x, int y, int w, int h, PImage&)
{
	// TODO: The whole thing
	bwassert(0);
	(void)(x+y+w+h);
}


void PixmapHand::getSubImage(int x, int y, int w, int h, PImage&, int destX, int destY)
{
	// TODO: The whole thing
	bwassert(0);
	(void)(x+y+w+h+destX+destY);
}

bool PixmapHand::operator==(const PixmapHand& ph) const
{
	return m_pDisplay==ph.m_pDisplay
	       && m_pixmap==ph.m_pixmap
	       && m_w==ph.m_w
	       && m_h==ph.m_h;
}

//////////////////////////////////////////////////////////////////
//
// BitmapHand
//

BitmapHand::BitmapHand(Display* pdis,unsigned long bitmap, int w, int h)
	:	m_pDisplay(pdis), m_bitmap(bitmap), m_w(w), m_h(h)
{}


BitmapHand::BitmapHand()
	:	m_pDisplay(0), m_bitmap(0), m_w(0), m_h(0)
{}


void BitmapHand::destroy()
{
	XFreePixmap(m_pDisplay,m_bitmap);
	m_bitmap = 0;
}


bool BitmapHand::isValid()
{
	return m_bitmap!=0;
}


int BitmapHand::w()
{
	return m_w;
}


int BitmapHand::h()
{
	return m_h;
}


GCHand BitmapHand::newGC(const GCSpec& gcs)
{
	return GCHand(XGC::fetchGC(m_pDisplay,m_bitmap,gcs));
}


void BitmapHand::getImage(int x, int y, int w, int h, PImage&)
{
	// TODO: The whole thing
	bwassert(0);
	(void)(x+y+w+h);
}


void BitmapHand::getSubImage(int x, int y, int w, int h, PImage&, int destX, int destY)
{
	// TODO: The whole thing
	bwassert(0);
	(void)(x+y+w+h+destX+destY);
}

bool BitmapHand::operator==(const BitmapHand& bh) const
{
	return m_pDisplay==bh.m_pDisplay
	       && m_bitmap==bh.m_bitmap
	       && m_w==bh.m_w
	       && m_h==bh.m_h;
}


//////////////////////////////////////////////////////////////////
//
// FontHand
//

FontHand::FontHand(Display* pd,void * pfs)
	:	pDisplay(pd), pFontSet(pfs)
{}


FontHand::FontHand()
	:	pFontSet(0)
{}

void FontHand::release()
{
	if (pFontSet)
		XFreeFontSet(pDisplay,static_cast<XFontSet>(pFontSet));
	pFontSet = 0;
}

int FontHand::maxH() const
{
	XFontSetExtents *xfse = XExtentsOfFontSet(static_cast<XFontSet>(pFontSet));
	return xfse->max_logical_extent.height;
}


int FontHand::maxW() const
{
	XFontSetExtents *xfse = XExtentsOfFontSet(static_cast<XFontSet>(pFontSet));
	return xfse->max_logical_extent.width;
}

int FontHand::maxAscent() const
{
	XFontSetExtents *xfse = XExtentsOfFontSet(static_cast<XFontSet>(pFontSet));
	return -xfse->max_logical_extent.y;
}

int FontHand::maxDescent() const
{
	XFontSetExtents *xfse = XExtentsOfFontSet(static_cast<XFontSet>(pFontSet));
	return xfse->max_logical_extent.height + xfse->max_logical_extent.y;
}


void FontHand::getTextExtent(const char* psz, TextExtent& te) const
{
	XRectangle extent;
	te.width = XmbTextExtents(static_cast<XFontSet>(pFontSet),psz,strlen(psz),0,&extent);
	te.height = extent.height;
	te.descent = -extent.y;
	te.ascent = te.height-te.descent;
}


int FontHand::getTextLength(const char* psz) const
{
	return XmbTextEscapement(static_cast<XFontSet>(pFontSet),psz,strlen(psz));
}


bool FontHand::operator==(const FontHand& fh) const
{
	return pDisplay==fh.pDisplay
	       && pFontSet==fh.pFontSet;
}

//////////////////////////////////////////////////////////////////
//
// FontSpec
//


FontSpec::FontSpec()
	:
	m_tenths(0),
	m_weight(AnyWeight),
	m_slant(AnySlant)
{
	// Note: Does find all
	*m_pszFamily = '\0';
	*m_pszFoundry = '\0';
	*m_pszCharset = '\0';
}


FontSpec::FontSpec(const char* pszName)
	:
	m_tenths(0),
	m_weight(AnyWeight),
	m_slant(AnySlant)
{
	// Note: friendly or X
	//
	// X font name specification is:
	// -foundry-family-weight-slant-setwidth-adstyle-pixelsize-pointsize-resx-resy-spacing-width-charset1-charset2
	//
	// Friendly names are:
	//
	// Family(Foundry) pointsize weight slant
	//
	//	Foundry is optional
	//	weight is one of "", bold
	//   slant is one of "", italic, oblique
	//
	// TODO: This really needs more thought, but starting from the caller side.

	*m_pszFamily = '\0';
	*m_pszFoundry = '\0';
	*m_pszCharset = '\0';

	int i;

	if (pszName) {

		if (*pszName=='-') {
			// X name

			// Foundry
			++pszName;
			for (i=0; i<49 && *pszName && *pszName!='-'; ++i) {
				m_pszFoundry[i] = *pszName++;
			}
			m_pszFoundry[i] = '\0';
			if (strcmp(m_pszFoundry,"*")==0)
				*m_pszFoundry = '\0';

			if (*pszName)
				++pszName;

			for (i=0; i<49 && *pszName && *pszName!='-'; ++i) {
				m_pszFamily[i] = *pszName++;
			}
			m_pszFamily[i] = '\0';
			if (strcmp(m_pszFamily,"*")==0)
				*m_pszFamily = '\0';

			if (*pszName)
				++pszName;

			if (strncmp(pszName,"*-",2)) {
				if (strncmp(pszName,"medium-",7)==0 || strncmp(pszName,"regular-",7)==0 )
					m_weight = Medium;
				else
					m_weight = Bold;
			}

			while (*pszName && *pszName++!='-')
				;

			if (strncmp(pszName,"*-",2)) {
				switch(*pszName) {
				case 'r':
					m_slant = Roman;
					break;
				case 'i':
					m_slant = Italic;
					break;
				case 'o':
					m_slant = Oblique;
				}
			}

			for (i=0; i<4; ++i)
				while (*pszName && *pszName++!='-')
					;

			while( isdigit(*pszName) ) {
				m_tenths = m_tenths*10 + *pszName - '0';
				++pszName;
			}

			for (i=0; i<6; ++i)
				while (*pszName && *pszName++!='-')
					;

			strncpy(m_pszCharset,pszName,49);
			m_pszCharset[49] = '\0';
		} else {
			// Friendly name

			for (i=0; i<49 && *pszName; ++i) {
				if (*pszName=='(')
					break;
				if (*pszName==' ' && isdigit(pszName[1]))
					break;
				m_pszFamily[i] = *pszName++;
			}
			m_pszFamily[i] = '\0';

			if (*pszName=='(') {
				++pszName;
				for (i=0; i<49 && *pszName; ++i) {
					if (*pszName==')')
						break;
					m_pszFoundry[i] = *pszName++;
				}
				m_pszFoundry[i] = '\0';
			}

			if (*pszName==')')
				++pszName;

			while (*pszName==' ')
				++pszName;

			while (isdigit(*pszName)) {
				m_tenths = m_tenths*10 + *pszName++ - '0';
			}
			if (*pszName=='.') {
				if (isdigit(*++pszName))
					m_tenths = m_tenths*10 + *pszName++ - '0';
				else
					m_tenths = m_tenths*10;
				++pszName;
			} else
				m_tenths = m_tenths*10;

			if (strstr(pszName,"bold"))
				m_weight = Bold;
			else
				m_weight = Medium;

			if (strstr(pszName,"italic"))
				m_slant = Italic;
			else if (strstr(pszName,"oblique"))
				m_slant = Oblique;
			else
				m_slant = Roman;

		}
	}
}


FontSpec::FontSpec(const char* pszFamily,
                   int tenPoints,
                   const char* pszFoundry,
                   Weight w,
                   Slant s,
                   const char* pszCharset)
	:
	m_tenths(tenPoints),
	m_weight(w),
	m_slant(s)
{
	if (pszFamily) {
		strncpy(m_pszFamily,pszFamily,49);
		m_pszFamily[49] = '\0';
	} else
		*m_pszFamily = '\0';

	if (*pszFoundry) {
		strncpy(m_pszFoundry,pszFoundry,49);
		m_pszFoundry[49] = '\0';
	} else
		*m_pszFoundry = '\0';

	if (*pszCharset) {
		strncpy(m_pszCharset,pszCharset,49);
		m_pszCharset[49] = '\0';
	} else
		*m_pszCharset = '\0';
}


void FontSpec::getName(char* buf, int maxLen) const
{
	char psz[40] = "";
	if (m_weight==Bold)
		strcpy(psz," bold");
	if (m_slant==Italic)
		strcat(psz," italic");
	if (m_slant==Oblique)
		strcat(psz," oblique");

	if (m_tenths%10)
		snprintf(buf,maxLen,"%s(%s) %d.%d%s",m_pszFamily,m_pszFoundry,m_tenths/10,m_tenths%10,psz);
	else
		snprintf(buf,maxLen,"%s(%s) %d%s",m_pszFamily,m_pszFoundry,m_tenths/10,psz);

}


const char* FontSpec::family() const
{
	return m_pszFamily;
}


int FontSpec::tenPoints() const
{
	return m_tenths;
}


const char* FontSpec::foundry() const
{
	return m_pszFoundry;
}


const char* FontSpec::charset() const
{
	return m_pszCharset;
}


FontSpec::Weight FontSpec::weight() const
{
	return m_weight;
}


FontSpec::Slant FontSpec::slant() const
{
	return m_slant;
}


bool FontSpec::operator==(const FontSpec& fs) const
{
	return
	    strcmp(m_pszFamily,fs.m_pszFamily)==0 &&
	    strcmp(m_pszFoundry,fs.m_pszFoundry)==0 &&
	    m_tenths==fs.m_tenths &&
	    m_weight==fs.m_weight &&
	    m_slant==fs.m_slant &&
	    strcmp(m_pszCharset,fs.m_pszCharset)==0;
}


bool FontSpec::operator<(const FontSpec& fs) const
{
	int cmp;

	cmp = strcmp(m_pszFamily,fs.m_pszFamily);
	if (cmp<0)
		return true;
	else if (cmp>0)
		return false;


	cmp = strcmp(m_pszFoundry,fs.m_pszFoundry);
	if (cmp<0)
		return true;
	else if (cmp>0)
		return false;

	if (m_tenths<fs.m_tenths)
		return true;
	else if (m_tenths>fs.m_tenths)
		return false;

	if (m_weight<fs.m_weight)
		return true;
	else if (m_weight>fs.m_weight)
		return false;

	if (m_slant<fs.m_slant)
		return true;
	else if (m_slant>fs.m_slant)
		return false;

	cmp = strcmp(m_pszCharset,fs.m_pszCharset);
	if (cmp<0)
		return true;

	return false;
}


bool FontSpec::match(const FontSpec& pat) const
{
	bool ret = true;

	if (*pat.m_pszFamily)
		if (strcmp(m_pszFamily,pat.m_pszFamily)!=0)
			ret = false;

	if (*pat.m_pszFoundry)
		if (strcmp(m_pszFoundry,pat.m_pszFoundry)!=0)
			ret = false;

	if (pat.m_tenths)
		if (m_tenths!=pat.m_tenths)
			ret = false;

	if (pat.m_weight!=AnyWeight)
		if (m_weight!=pat.m_weight)
			ret = false;

	if (pat.m_slant!=AnySlant)
		if (m_slant!=pat.m_slant)
			ret = false;

	if (*pat.m_pszCharset)
		if (strcmp(m_pszCharset,pat.m_pszCharset)!=0)
			ret = false;

	return ret;
}


//////////////////////////////////////////////////////////////////
//
// CursorHand
//

CursorHand::CursorHand()
	: pd(0), cursor(0)
{}


CursorHand::CursorHand(Display* pDisplay,unsigned long c)
	: pd(pDisplay), cursor(c)
{}


void CursorHand::release()
{
	XFreeCursor(pd,cursor);
}

bool CursorHand::isValid()
{
	return pd!=0;
}


//////////////////////////////////////////////////////////////////
//
// RegionHand
//

RegionHand::RegionHand()
	: reg(0)
{
}


RegionHand::RegionHand(Point* pts, int nPoints)
{
	reg = XPolygonRegion(reinterpret_cast<XPoint*>(pts),nPoints,EvenOddRule);
}

RegionHand::RegionHand(const RegionHand& rh)
{
	if (rh.reg) {
		reg = XCreateRegion();
		add(rh);
	} else
		reg = 0;
}


RegionHand& RegionHand::operator=(const RegionHand& rh)
{
	if (reg)
		XDestroyRegion(reg);

	if (rh.reg) {
		reg = XCreateRegion();
		Region tmpreg = XCreateRegion();
		XUnionRegion(tmpreg,rh.reg,reg);
		XDestroyRegion(tmpreg);
	} else
		reg = 0;

	return *this;
}


RegionHand::~RegionHand()
{
	if (reg)
		XDestroyRegion(reg);
}


bool RegionHand::isEmpty() const
{
	return reg==0 || XEmptyRegion(reg);
}


bool RegionHand::isInside(int x, int y) const
{
	return XPointInRegion(areg(),x,y);
}


bool RegionHand::isInside(int x, int y, int w, int h) const
{
	return XRectInRegion(areg(),x,y,w,h);
}


bool RegionHand::operator==(const RegionHand& rh) const
{
	return XEqualRegion(areg(),rh.areg());
}


void RegionHand::addRect(int x, int y, int w, int h)
{
	Region reg2 = XCreateRegion();
	XRectangle xrect;

	xrect.x = x;
	xrect.y = y;
	xrect.width = w;
	xrect.height = h;

	XUnionRectWithRegion(&xrect,areg(),reg2);
	XDestroyRegion(reg);
	reg = reg2;
}


void RegionHand::subRect(int x, int y, int w, int h)
{
	Region tmp = XCreateRegion();
	Region rect = XCreateRegion();
	Region res = XCreateRegion();
	XRectangle xrect;

	xrect.x = x;
	xrect.y = y;
	xrect.width = w;
	xrect.height = h;

	XUnionRectWithRegion(&xrect,tmp,rect);
	XSubtractRegion(areg(),rect,res);
	XDestroyRegion(reg);
	reg = res;
	XDestroyRegion(tmp);
	XDestroyRegion(rect);
}


void RegionHand::intersectRect(int x, int y, int w, int h)
{
	Region tmp = XCreateRegion();
	Region rect = XCreateRegion();
	Region res = XCreateRegion();
	XRectangle xrect;

	xrect.x = x;
	xrect.y = y;
	xrect.width = w;
	xrect.height = h;

	XUnionRectWithRegion(&xrect,tmp,rect);
	XIntersectRegion(areg(),rect,res);
	XDestroyRegion(reg);
	reg = res;
	XDestroyRegion(tmp);
	XDestroyRegion(rect);
}


void RegionHand::add(const RegionHand& rh)
{
	Region res = XCreateRegion();

	XUnionRegion(areg(),rh.reg,res);
	XDestroyRegion(reg);
	reg = res;
}


void RegionHand::sub(const RegionHand& rh)
{
	Region res = XCreateRegion();

	XSubtractRegion(areg(),rh.reg,res);
	XDestroyRegion(reg);
	reg = res;
}


void RegionHand::intersect(const RegionHand& rh)
{
	Region res = XCreateRegion();

	XIntersectRegion(areg(),rh.reg,res);
	XDestroyRegion(reg);
	reg = res;
}

Region RegionHand::areg() const
{
	if (!reg)
		reg = XCreateRegion();
	return reg;
}

//////////////////////////////////////////////////////////////////
//
// GCSpec
//

GCSpec::GCSpec()
	: m_mask(mnone)
{}


GCSpec::~GCSpec()
{}

void GCSpec::reset()
{
	m_mask = mnone;
}

bool GCSpec::compatible(const GCSpec& gcs) const
{
	Mask mc = (Mask)(m_mask & gcs.m_mask);
	bool ret = true;

	if (mc&mfg)
		ret &= (m_fg==gcs.m_fg);
	if (mc&mbg)
		ret &= (m_bg==gcs.m_bg);
	if (mc&mtile)
		ret &= (m_tile==gcs.m_tile);
	if (mc&mstip)
		ret &= (m_stip==gcs.m_stip);
	if (mc&mwidth)
		ret &= (m_width==gcs.m_width);
	if (mc&mfunc)
		ret &= (m_func==gcs.m_func);
	if (mc&mcap)
		ret &= (m_cap==gcs.m_cap);
	if (mc&mdash)
		ret &= (m_dash==gcs.m_dash);
	if (mc&morig)
		ret &= (m_orig==gcs.m_orig);
	if (mc&mclip)
		ret &= (m_clip==gcs.m_clip);

	return ret;
}

void GCSpec::add(const GCSpec& gcs)
{
	Mask mc = gcs.m_mask;

	if (mc&mfg)
		m_fg = gcs.m_fg;
	if (mc&mbg)
		m_bg = gcs.m_bg;
	if (mc&mtile)
		m_tile = gcs.m_tile;
	if (mc&mstip)
		m_stip = gcs.m_stip;
	if (mc&mwidth)
		m_width = gcs.m_width;
	if (mc&mfunc)
		m_func = gcs.m_func;
	if (mc&mcap)
		m_cap = gcs.m_cap;
	if (mc&mdash)
		m_dash = gcs.m_dash;
	if (mc&morig)
		m_orig = gcs.m_orig;
	if (mc&mclip)
		m_clip = gcs.m_clip;

}

GCSpec GCSpec::subtract(const GCSpec& orig,const GCSpec& gcs)
{
	// Returns a GCSpec that sets all the attributes in gcs that are not
	// already set
	GCSpec ret;

	Mask mc = (Mask)(orig.m_mask & ~ gcs.m_mask);
	ret.m_mask = mc;

	if (mc&mfg)
		ret.m_fg = gcs.m_fg;
	if (mc&mbg)
		ret.m_bg = gcs.m_bg;
	if (mc&mtile)
		ret.m_tile = gcs.m_tile;
	if (mc&mstip)
		ret.m_stip = gcs.m_stip;
	if (mc&mwidth)
		ret.m_width = gcs.m_width;
	if (mc&mfunc)
		ret.m_func = gcs.m_func;
	if (mc&mcap)
		ret.m_cap = gcs.m_cap;
	if (mc&mdash)
		ret.m_dash = gcs.m_dash;
	if (mc&morig)
		ret.m_orig = gcs.m_orig;
	if (mc&mclip)
		ret.m_clip = gcs.m_clip;

	return ret;
}

GC GCSpec::createGC(Display* pd,unsigned long wnd) const
{
	XGCValues xgcv;
	Region reg;
	unsigned long mask = xvalues(&xgcv,reg);
	GC ret = XCreateGC(pd,wnd,mask,&xgcv);
	if (reg)
		XSetRegion(pd,ret,reg);
	return ret;
}


void GCSpec::changeGC(Display* pd,GC gc) const
{
	XGCValues xgcv;
	Region reg;
	unsigned long mask = xvalues(&xgcv,reg);
	if (mask)
		XChangeGC(pd,gc,mask,&xgcv);
	if (reg)
		XSetRegion(pd,gc,reg);
}



unsigned long GCSpec::xvalues(void* pxgcv, Region& reg) const
{
	unsigned long xmask = 0;
	XGCValues& xgcv=*static_cast<XGCValues*>(pxgcv);

	if (m_mask&mfg) {
		xmask |= GCForeground;
		xgcv.foreground = m_fg;
	}

	if (m_mask&mbg) {
		xmask |= GCBackground;
		xgcv.background = m_bg;
	}

	if (m_mask&mtile) {
		xmask |= GCTile;
		xgcv.tile = m_tile.m_pixmap;
	}

	if (m_mask&mstip) {
		xmask |= GCStipple;
		xgcv.stipple = m_stip.m_bitmap;
	}

	if (m_mask&mwidth) {
		xmask |= GCLineWidth;
		xgcv.line_width = m_width;
	}

	if (m_mask&mfunc) {
		xmask |= GCFunction;
		xgcv.function = m_func;
	}

	if (m_mask&mcap) {
		xmask |= GCCapStyle;
		xgcv.cap_style = m_cap;
	}

	if (m_mask&mdash) {
		xmask |= GCLineStyle;
		xgcv.line_style = m_dash;
	}

	if (m_mask&morig) {
		xmask |= GCTileStipXOrigin | GCTileStipYOrigin;
		xgcv.ts_x_origin = m_orig.x;
		xgcv.ts_y_origin = m_orig.y;
	}

	if (m_mask&mclip)
		reg = m_clip.reg;
	else
		reg = 0;
	return xmask;
}

//////////////////////////////////////////////////////////////////
//
// GCPool
//

GCPoolRef GCPool::poolroot = 0;

GCPool::GCPool(Display* pdis,Drawable refdrawable,const GCSpec& gcs)
	:	pd(pdis), combined(gcs)
{
	trace << "Creating GCPool at " << this << std::endl;

	gc = gcs.createGC(pd,refdrawable);

	// TODO: Does this require a round trip to the server? -- YES!!!
	Window dum1;
	int dum2,dum3;
	unsigned int dum4,dum5,dum6;
	XGetGeometry(pd,refdrawable,&dum1,&dum2,&dum3,&dum4,&dum5,&dum6,&depth);

	if (poolroot) {
		GCPool* pGCP=poolroot;
		while (pGCP->next)
			pGCP = pGCP->next;
		pGCP->next = this;
	} else {
		poolroot = this;
	}
	AddRef();		// For pool
}


GCPoolRef GCPool::firstCompatible(Display* pdis,Drawable draw,const GCSpec& gcs)
{
	// Look for an existing one

	// TODO: Does this require a round trip to the server?
	unsigned int ndepth;
	Window dum1;
	int dum2,dum3;
	unsigned int dum4,dum5,dum6;
	XGetGeometry(pdis,draw,&dum1,&dum2,&dum3,&dum4,&dum5,&dum6,&ndepth);

	GCPool* pGCP=poolroot;
	while (pGCP) {
		if (pGCP->pd==pdis && pGCP->depth==ndepth && gcs.compatible(pGCP->combined)) {
			GCSpec diff = GCSpec::subtract(pGCP->combined,gcs);
			diff.changeGC(pGCP->pd,pGCP->gc);
			pGCP->combined.add(gcs);
			pGCP->AddRef();
			return pGCP;
		}
		pGCP = pGCP->next;
	}

	// Look for an unused one
	pGCP=poolroot;
	while (pGCP) {
		if (pGCP->pd==pdis && pGCP->depth==ndepth && pGCP->isUnused()) {
			gcs.changeGC(pGCP->pd,pGCP->gc);
			pGCP->combined.reset();
			pGCP->combined.add(gcs);
			pGCP->AddRef();
			return pGCP;
		}
		pGCP = pGCP->next;
	}

	return 0;
}

bool GCPool::isUnused() const
{
	return m_cRef==1;
}

bool GCPool::isShared() const
{
	return m_cRef>2;
}



//////////////////////////////////////////////////////////////////
//
// XGC
//

XGC* XGC::fetchGC(Display* pdis,Drawable drawable,const GCSpec& gcs)
{
	// GC's in X are not window specific, but the isolation layer uses
	// drawable specific GCs.
	//
	// This routine searches a pool of GCs for a compatible one. If found,
	// this is used.  If not found, an unused or new GC is used.
	//
	GCPoolRef gcpr = GCPool::firstCompatible(pdis,drawable,gcs);
	if (!gcpr)
		gcpr = new GCPool(pdis,drawable,gcs);

	return new XGC(pdis,drawable,gcs,gcpr);
}


XGC::XGC(Display* pdis,Drawable drawable,const GCSpec& gcspec,const GCPoolRef gcprIn)
	: pd(pdis), draw(drawable), gcs(gcspec), gcpr(gcprIn)
{}

XGC::~XGC()
{}


bool XGC::isValid()
{
	return pd!=0;
}


bool XGC::isEmpty()
{
	// should probably verify against window bounds
	//if (gcs.m_mask&GCSpec::mclip)
	//   return gcs.m_clip.isEmpty();

	return false;	// TODO: The real thing
}


void XGC::change(const GCSpec& gcc)
{
	gcs.add(gcc);

	if (gcpr->isShared())
		setSpec(gcs);      	// Just switch to another GC
	else
		gcc.changeGC(pd,*gcpr);	// Note: not thread safe
}


void XGC::setSpec(const GCSpec& gcc)
{
	gcs = gcc;
	gcpr = GCPool::firstCompatible(pd,draw,gcs);
	if (!gcpr)
		gcpr = new GCPool(pd,draw,gcs);
}

GCSpec XGC::getSpec()
{
	return gcs;
}


void XGC::clearAll()
{
	XClearWindow(pd,draw);
}


void XGC::clearArea(int x, int y, int w, int h)
{
	XClearArea(pd,draw,x,y,w,h,False);
}


void XGC::copy(int x, int y, WinHand src, int srcX, int srcY, int w, int h)
{
	XCopyArea(pd, src.wnd, draw, *gcpr, srcX, srcY, w, h, x, y);
}


void XGC::copy(int x, int y, PixmapHand src, int srcX, int srcY, int w, int h)
{
	XCopyArea(pd, src.m_pixmap, draw, *gcpr, srcX, srcY, w, h, x, y);
}


void XGC::copy(int x, int y, BitmapHand src, int srcX, int srcY, int w, int h)
{
	XCopyArea(pd, src.m_bitmap, draw, *gcpr, srcX, srcY, w, h, x, y);
}


void XGC::copy(int x, int y, PImage& src, int srcX, int srcY, int w, int h)
{
	bwassert(0);
}


void XGC::drawPoint(int x, int y)
{
	XDrawPoint(pd,draw,*gcpr,x,y);
}


void XGC::drawPoints(Point* pts, int nPoints, AbsRel absrel)
{
	XDrawPoints(pd,draw,*gcpr,reinterpret_cast<XPoint*>(pts),nPoints,absrel);
}


void XGC::drawLine(int x1, int y1, int x2, int y2)
{
	XDrawLine(pd,draw,*gcpr,x1,y1,x2,y2);
}


void XGC::drawRectangle(int x, int y, int w, int h)
{
	XDrawRectangle(pd,draw,*gcpr,x,y,w,h);
}


void XGC::fillRectangle(int x, int y, int w, int h)
{
	XFillRectangle(pd,draw,*gcpr,x,y,w,h);
}


void XGC::drawArc(int x, int y, int w, int h, int angle1, int angle2)
{
	XDrawArc(pd,draw,*gcpr,x,y,w,h,angle1,angle2);
}


void XGC::fillPie(int x, int y, int w, int h, int angle1, int angle2)
{
	XGCValues xgcv;
	xgcv.arc_mode = ArcPieSlice;
	XChangeGC(pd,*gcpr,GCArcMode,&xgcv);
	XFillArc(pd,draw,*gcpr,x,y,w,h,angle1,angle2);
}


void XGC::fillChord(int x, int y, int w, int h, int angle1, int angle2)
{
	XGCValues xgcv;
	xgcv.arc_mode = ArcChord;
	XChangeGC(pd,*gcpr,GCArcMode,&xgcv);
	XFillArc(pd,draw,*gcpr,x,y,w,h,angle1,angle2);
}


void XGC::drawPolygon(Point* pts, int nPoints, AbsRel ar, JoinStyle js)
{
	XGCValues xgcv;
	xgcv.join_style = js;
	XChangeGC(pd,*gcpr,GCJoinStyle,&xgcv);
	XDrawLines(pd,draw,*gcpr,reinterpret_cast<XPoint*>(pts),nPoints,ar);
}


void XGC::fillPolygon(Point* pts, int nPoints, AbsRel ar, ShapeHint sh)
{
	XFillPolygon(pd,draw,*gcpr,reinterpret_cast<XPoint*>(pts),nPoints,sh,ar);
}


void XGC::drawString(FontHand fh,int x, int y, const char* psz)
{
	XmbDrawString(pd,draw,
	              static_cast<XFontSet>(fh.pFontSet),
	              *gcpr,x,y,psz,strlen(psz));
}


//virtual void XGC::drawString(int x, int y, const wchar_t*);


void XGC::fillString(FontHand fh,int x, int y, const char* psz)
{
	XmbDrawImageString(pd,draw,
	                   static_cast<XFontSet>(fh.pFontSet),
	                   *gcpr,x,y,psz,strlen(psz));
}


//virtual void XGC::fillString(int x, int y, const wchar_t*);

//////////////////////////////////////////////////////////////////
//
// KeyNum
//

KeyNum::KeyNum(const char* name)
{
	m_keysym = static_cast<KeySym>(XStringToKeysym(name));
}

KeyNum::KeyNum(KeySym ks)
	: m_keysym(ks)
{}


KeyNum::KeySym KeyNum::keysym()
{
	return m_keysym;
}

const char* KeyNum::name()
{
	// map Prior -> Page_Up, Next -> Page_Down
	const char* ret = XKeysymToString(m_keysym);
	if (strcmp(ret,"Prior")==0)
		ret = "Page_Up";
	if (strcmp(ret,"Next")==0)
		ret = "Page_Down";

	return ret;
}

void KeyNum::fullName(Modifier m, char* retbuf, int nChars)
{
	// TODO
	bwassert(0);
}


bool KeyNum::isCursorKey()
{
	return m_keysym>=KS_Up && m_keysym<=KS_Right;
}


bool KeyNum::isFunctionKey()
{
	// //TODO: (or PFKey)
	return m_keysym>=KS_F1 && m_keysym<=KS_F12;
}


bool KeyNum::isKeypadKey()
{
	return m_keysym==KS_KP_Enter || (m_keysym>=KS_KP_Multiply && m_keysym<=KS_KP_9);
}


bool KeyNum::isMiscFunctionKey()
{
	return m_keysym>255 && !isCursorKey() && !isFunctionKey() && !isKeypadKey() && !isModifierKey();
}


bool KeyNum::isModifierKey()
{
	return m_keysym>=KS_Shift_L && m_keysym<=KS_Alt_R;
}


}	// Namespace


