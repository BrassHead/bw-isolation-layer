/* event.h -- Interface for the Event and EventRet classes

Copyright (C) 1997-2013 Brian Bray

*/

//#include <X11/Xlib.h>
//#include <X11/Xutil.h>
//#include <X11/Xresource.h>

namespace bw {

// Forward definitions
class Figure;
class FigureImp;
class FigureNode;

// BW Specific events

enum {Activate=40, PreActivate, PostActivate, InvalidateRect,
      InvalidateParentRect, Draw, Flash, Deactivate, PreDeactivate,
      Reposition, MoveNotify, SizeNotify, RelocateNotify,
      MoveRequest, SizeRequest, RelocateRequest, MaxEvent
     };

struct ActivateEvent {
	int			type;			// Activate
	unsigned long	serial;			// Unused
	Bool		send_event;		// Unused
	Display*		display;		// pDisplay
	Window		parent;			// parent window
	Figure*		m_pfigParent;		// parent figure
	FigureNode*		m_pfignParent;		// parent figurenode
};
// EventRet is not used

struct PreActivateEvent {
	int			type;			// PreActivate
};
// The retcode return indicates the event XSetWindowAttributes mask for set
// fields.

struct PostActivateEvent {
	int			type;			// PostActivate
};
// EventRet is not used.

// Move, Size, and Relocate Notify events use XConfigure

typedef int LogPos;		// Must match figure.h
struct SizeRequestEvent {
	int		type;	  	// Size, Move, or RelocateRequest
	unsigned long	serial;		// Unused
	Bool		send_event;	// Unused
	Display*	display;	// Unused
	Window		window;		// Unused
	Figure*		pfig;		// Requesting figure
	LogPos		lposX;
	LogPos		lposY;
	LogPos		lposWidth;
	LogPos		lposHeight;
};


// An Event is the same as an XEvent, but with some additional alternatives
// ***DO NOT CHANGE*** anything before "pad" below.
union Event {
	int type;
	XAnyEvent xany;
	XKeyEvent xkey;
	XButtonEvent xbutton;
	XMotionEvent xmotion;
	XCrossingEvent xcrossing;
	XFocusChangeEvent xfocus;
	XExposeEvent xexpose;
	XGraphicsExposeEvent xgraphicsexpose;
	XNoExposeEvent xnoexpose;
	XVisibilityEvent xvisibility;
	XCreateWindowEvent xcreatewindow;
	XDestroyWindowEvent xdestroywindow;
	XUnmapEvent xunmap;
	XMapEvent xmap;
	XMapRequestEvent xmaprequest;
	XReparentEvent xreparent;
	XConfigureEvent xconfigure;
	XGravityEvent xgravity;
	XResizeRequestEvent xresizerequest;
	XConfigureRequestEvent xconfigurerequest;
	XCirculateEvent xcirculate;
	XCirculateRequestEvent xcirculaterequest;
	XPropertyEvent xproperty;
	XSelectionClearEvent xselectionclear;
	XSelectionRequestEvent xselectionrequest;
	XSelectionEvent xselection;
	XColormapEvent xcolormap;
	XClientMessageEvent xclient;
	XMappingEvent xmapping;
	XErrorEvent xerror;
	XKeymapEvent xkeymap;
	long pad[24];

	// BW Specific events
	ActivateEvent evAct;
	PreActivateEvent evPreAct;
	PostActivateEvent evPostAct;
	SizeRequestEvent evSizeReq;
};


class EventRet {
public:
	unsigned long	retcode;
};

inline void makeActivateEvent( Event& ev, Display* pDisplay,
                               Window wndParent, Figure* pfigParent,
                               FigureNode* pfignParent )
{
	ev.type = Activate;
	ev.evAct.display = pDisplay;
	ev.evAct.parent = wndParent;
	ev.evAct.m_pfigParent = pfigParent;
	ev.evAct.m_pfignParent = pfignParent;
}



}   // namespace

