/* scene.cc - Implementation of Scene figure class

Copyright (C) 1997-2013, Brian Bray

*/


// Compile time option
//#define NOTRACE
#include "bw/trace.h"

#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/figure.h"
#include "bw/scene.h"
#include "bw/exception.h"
#include "bw/process.h"
#include "bw/context.h"
#include "bw/tools.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include "fscene.h"
#include "fprocess.h"
#include "ffigure.h"
#include "event.h"
#include "gdevice.h"

namespace bw {

/*: class Scene

  Represents the display.  Scene is a special Figure class that is the root
  of all active Figures.  Adding a child Figure to the Scene causes a new
  window to be displayed.  Normally, only one Scene object is defined in an
  application's Process::main() routine.  Main then adds child figures for each
  root window of the application (again, normally just one) and then calls
  Scene::nextAction() to get and process action commands from the user
  interface.  This command processing loop terminates with a particular
  command (eg: "quit") and Process::main then exits.

  In addition, the Scene class has a number of query methods to determine
  characteristics of the display and to control overall display / keyboard /
  mouse activity.
*/

//
// Static variable definitions
//
XContext	FScene::m_ctxPSCENE = 0;

//
// Error callbacks from Xlib
//
extern "C" int XIOErrorTrap( Display* pDisplay )
{
	//
	// Get display name for error message
	//
	String strDisplayName;
	Scene* pScene = FScene::findScene( pDisplay );
	if (pScene) {
		strDisplayName = pScene->getScreen();
	} else {
		strDisplayName = "Unknown";
	}
	BGUIException e( BGUIException::ConnectionLost, strDisplayName );

	FProcess::fatalError( e );
	bwassert( false );
	return 0;		// Keep compiler happy
}

extern "C" int XErrorEventTrap( Display* pDisplay, XErrorEvent* pxev )
{
	bwassert( !FProcess::m_pxevLast );
	FProcess::m_pxevLast = pxev;		// TODO: rethink errors for threads

	if (!FProcess::m_isExpectingErrors) {
		const int 	nMessage = 100;
		char		pszMessage[nMessage];
		// This is an unexpected error.  This really indicates a defect in BW.
		XGetErrorText( pDisplay, pxev->error_code, pszMessage, nMessage );
		pszMessage[nMessage-1] = '\0';
		FProcess::fatalError( BGUIException( BGUIException::SystemError, pszMessage ) );
		bwassert( false );
	}
	return 0;
}

/* FScene::findScene()

   Returns a pointer to the Scene attached to an X Display*.

   Returns 0 if no Scene attached.
*/
Scene* FScene::findScene( Display* pDisplay )
{
	Scene* pScene;
	if (XFindContext(pDisplay,
	                 RootWindow(pDisplay,DefaultScreen(pDisplay)),
	                 m_ctxPSCENE,
	                 (char**)&pScene )) {
		pScene = 0;
	}
	return pScene;
}

/* Atoms::Atoms()

   Initialize the list of common atoms for this display.
*/
Atoms::Atoms( Display* pDisplay )
{
	const char* apszNames[] = {
		"WM_PROTOCOLS",
		"WM_DELETE_WINDOW",
		"WM_TAKE_FOCUS",
		"WM_SAVE_YOURSELF"
	};
	if (!XInternAtoms( pDisplay, const_cast<char**>(apszNames), 4, False, &m_atomProtocol ))
		throw BGUIException( BGUIException::SystemError, "Cannot intern common atoms." );
}

/*: Scene::Scene()

  Constructor.  The first optional parameter is a platform dependent string
  indicating which display head to connect to.

  On UNIX, it's syntax is the same as the X DISPLAY environment variable
  or a -display parameter.

  If the string is null, then the created Scene represents the default
  display in the users environment.

  The second optional parameter is the flash rate in milliseconds.  Zero
  indicates no flash, One indicates that the rate should be synchronized
  with the vertical retrace.

  Prototype: Scene(String strScreen="", int msecFlashRate=0)

*/
Scene::Scene( String strScreen, int msecFlashRate )
	:   m_pDisplay( 0 ),
	    m_iScreen( 0 ),
	    m_strDisplayName( "" ),
	    m_strCommand( "" ),
	    m_haveInvalidRegions( false )
{
	bwassert( msecFlashRate==0 );		// TODO
	//
	// Things I do only when called for the first time
	//
	static bool isFirstCall = true;
	if (isFirstCall) {
		//
		// Set the X IO handlers.
		//
		XSetIOErrorHandler( XIOErrorTrap );
		(void)XSetErrorHandler( XErrorEventTrap );

		//
		// Get a context handle for reverse lookup
		//
		FScene::m_ctxPSCENE = XUniqueContext();

		isFirstCall = false;
	}

	//
	// Set the display name for error messages
	//
	m_strDisplayName = XDisplayName(strScreen);

	//
	// Open the display connection
	//
	m_pDisplay = XOpenDisplay( strScreen );

	if (!m_pDisplay) {
		throw BGUIException(BGUIException::NoDisplay);
	}
	m_iScreen = DefaultScreen( m_pDisplay );

	//
	// Provide a pointer back from the root window of the display to myself
	//
	if (XSaveContext( (Display*)m_pDisplay,
	                  RootWindow((Display*)m_pDisplay, m_iScreen),
	                  FScene::m_ctxPSCENE,
	                  (char*)this )) {
		// Fatal internal error
		throw BGUIException(BGUIException::SystemError, "Cannot save context.");
	}

	//
	// Intern common atoms
	//
	m_patoms = new Atoms( (Display*)m_pDisplay );

	//
	// Make myself active as the root window
	//
	m_pfi = new FigureImp;
	m_pfi->m_pDisplay = (Display*)m_pDisplay;
	m_pfi->m_pgdev = new GDevice( (Display*)m_pDisplay );
	m_pfi->m_pscene = this;
	m_pfi->m_wnd = RootWindow( (Display*)m_pDisplay, m_iScreen );
	m_pfi->m_wndParent = 0;
	m_pfi->m_pfigThis = this;
	m_pfi->m_pfigParent = 0;
	m_pfi->m_pfignParent = 0;
	m_pfi->m_xswa.background_pixmap = None;
	m_pfi->m_xswa.background_pixel = 0;
	m_pfi->m_xswa.border_pixmap = None;
	m_pfi->m_xswa.border_pixel = 0;
	m_pfi->m_xswa.bit_gravity = ForgetGravity;
	m_pfi->m_xswa.win_gravity = StaticGravity;
	m_pfi->m_xswa.backing_store = NotUseful;
	m_pfi->m_xswa.backing_planes = 0;
	m_pfi->m_xswa.backing_pixel = 0;
	m_pfi->m_xswa.save_under = False;
	m_pfi->m_xswa.event_mask = 0;
	m_pfi->m_xswa.do_not_propagate_mask = -1;
	m_pfi->m_xswa.override_redirect = True;
	m_pfi->m_xswa.colormap = CopyFromParent;
	m_pfi->m_xswa.cursor = None;
	m_pfi->m_x = 0;
	m_pfi->m_y = 0;
	m_pfi->m_width = DisplayWidth( (Display*)m_pDisplay, m_iScreen );
	m_pfi->m_height = DisplayHeight( (Display*)m_pDisplay, m_iScreen );
	//
	// TODO: Startup the flash alarm
	//

	//
	// Allocate system colours and fonts
	//

	//
	// Process is now in a new state
	//
	FProcess::setState(Process::running);	// Note: Assumes single Scene

}

/*: Scene::~Scene

  Destructor.  Removes all child Figures from the Display and terminates
  the graphical user interface of the application.
*/
Scene::~Scene() throw()
{
	//
	// Change of state
	// Note: Single Scene assumption
	FProcess::setState( Process::finishing );

	//
	// TODO: Turn off the flash alarm
	//

	//
	// remove my children (this will deactivate them)
	//
	removeAllChildren();

	//
	// Make myself inactive
	//
	delete m_pfi;
	m_pfi = 0;

	//
	// Remove reverse pointer
	//
	XDeleteContext( (Display*)m_pDisplay,
	                RootWindow((Display*)m_pDisplay, m_iScreen),
	                FScene::m_ctxPSCENE );

	//
	// Close the display connection
	//
	XCloseDisplay( (Display*)m_pDisplay );
	m_pDisplay = 0;
}

/*: Scene::getScreen()

  Return the name of the screen where this scene is displayed.  This may
  be different from the Screen string passed to the constructor.
*/
const String& Scene::getScreen()
{
	return m_strDisplayName;
}


/*: Scene::nextAction()

  Start / Continue user interface processing.  This method starts an event
  processing loop that results in method calls on active figures.

  The loop continues until a "doAction" call is made that propogates to
  the screen.  The action command is queued and then returned by this
  routine after event processing finishes.

  It is an error to call this routine if no Figures are active.
*/
const String& Scene::nextAction()
{
	//
	// Reset previous command
	// TODO: Command queue.  Right now, my queue length is 1.
	//
	m_strCommand = "";

	messagePump();
	bwassert ( m_strCommand!="" );
	return m_strCommand;
}

/*: Scene::addFrame()

  Add a frame figure to the scene.  Positions and sizes can be specified, but
  these may or may not be honored by the windowing system.

  By default, the child is asked for it's desired size and location.  If the
  size returned is FullWindow, then a platform dependent size is chosen (1/4
  of the screen or more).

  Prototype: void Scene::addFrame( Figure* pfig )
  Prototype: void Scene::addFrame( Figure* pfig, LogPos lposWidth, LogPos lposHeight )
  Prototype: void Scene::addFrame( Figure* pfig,
			      LogPos lposX, LogPos lposY,
                              LogPos lposWidth, LogPos lposHeight )
*/
void Scene::addFrame( Figure* pfig, LogPos lposWidth, LogPos lposHeight )
{
	addFrame( pfig, 0, 0, lposWidth, lposHeight );
}


void Scene::addFrame( Figure* pfig,
                      LogPos lposX, LogPos lposY,
                      LogPos lposWidth, LogPos lposHeight )
{
	//
	// This is basically an addchild with special handing so the size and
	// location are absolute.
	//

	int x,y,w,h;
	absLoc(pfig,lposX,lposY,lposWidth,lposHeight,x,y,w,h);

	addChild( pfig, x, y, w, h );
}


/*: Scene::removeFrame()

  Removes a Figure from the Scene.
*/
void Scene::removeFrame( Figure* pfig )
{
	removeChild( pfig );
}


/*: Scene::setFlashRate()

  Sets the flash rate in milliseconds.  Zero
  indicates no flash, One indicates that the rate should be synchronized
  with the vertical retrace.
*/
void Scene::setFlashRate( int msec )
{
	bwassert( msec==0 );	// TODO
}

void Scene::onResizeRequest( Figure* pfig, LogPos lposWidth, LogPos lposHeight )
{
	unsigned int x, y;
	LogPos absX, absY, absW, absH;
	getLocation(x,y);
	absLoc(pfig,x,y,lposWidth,lposHeight,absX,absY,absW,absH);
	resizeChild(pfig,absW,absH);
}


void Scene::onMoveRequest( Figure* pfig, LogPos lposX, LogPos lposY )
{
	int w, h;
	LogPos absX, absY, absW, absH;
	getSize(w,h);
	absLoc(pfig,lposX,lposY,w,h,absX,absY,absW,absH);
	moveChild(pfig,absW,absH);
}


void Scene::onRelocateRequest( Figure* pfig,
                               LogPos lposX, LogPos lposY,
                               LogPos lposWidth, LogPos lposHeight )
{
	LogPos absX, absY, absW, absH;
	absLoc(pfig,lposX,lposY,lposWidth,lposHeight,absX,absY,absW,absH);
	relocateChild(pfig,absX,absY,absW,absH);
}


/* Scene::onAction()

   This is called when an action reaches the scene.  Queue it for processing
   by main.
*/
bool Scene::onAction( const String& strCommand )
{
	// TODO: Command Queue
	bwassert( m_strCommand="" );
	m_strCommand = strCommand;
	return false;
}

/* Scene::draw()

   Draw routine required for Figures.

   This is called when a Draw event starts out.
*/
void Scene::draw( Canvas& )
{
	// Intentionally empty
}

/* Scene::messagePump()

   This is the message loop.  This is the only routine to manipulate the
   X event queue.

   The event loop is basically in one of two states:  Input or Update.

   The Scene starts in the Input state.  It switches to the Update state
   when the Flash timer goes off.  Other than that, commands are executed
   before new input events and draws are effectively deferred until there
   is no other work to be done (or the next flash).  This deferral is
   accomplished by separating expose events from draw events.

   Expose events are dispatched as they arrive and each Figure maintains
   an update region that is the union of all exposed and invalidated areas.
   Redraw is initiated when there is no work in the event queue or as part
   of a Flash.

   Flash alarms are processed in the order they are received within the input
   Queue with the proviso that old flash alarms are ignored.  I.E. If there
   are multiple alarm events in the queue, only the most recent will be
   processed.

   In addition to the X Event queue, two other queues are kept:
   <OL>
   <LI>Commands queued to be sent to main
   <LI>Invalidations deferred to the next Flash
   </OL>

   TODO: Multi-threads (lock scene?)
*/
void Scene::messagePump()
{
	//
	// First, make sure there is something to display (ie: I have at least one
	// child.
	//
	if (numChildren()==0)
		throw BGUIException( BGUIException::NoMainWindow );

	// now, enter event loop
	Event 	ev;
	EventRet 	evr;
	XEvent* 	pevt = (XEvent*)&ev;
	int		nEvents;

	//
	// This section is the Input state
	//
	while (true) {
		//
		// Check command queue
		//
		if (m_strCommand!="")
			return;

		//
		// If there is no other work, bring the screen up to date
		//
		nEvents = XPending( (Display*)m_pDisplay );
		if (nEvents==0 && m_haveInvalidRegions) {
			doDraw();
		}

		//
		// Otherwise, wait for something to do and then do it.
		//
		// TODO: Commands queued by other Threads will not be processed
		//
		XNextEvent( (Display*)m_pDisplay, pevt );

		//
		// X Defined event responses
		//
		if (XFilterEvent( pevt, (Window)None ))
			break;

		if (pevt->type==MappingNotify) {
			XRefreshKeyboardMapping( (XMappingEvent*)pevt );
			break;
		}

		//
		// TODO: Special handling of Flash alarms
		//
		// if pevt is a flash alarm, then go into Update state:
		// 1) Make the flash call to all active figures
		// 2) Dispatch all deferred invalidations
		// 3) Retrieve and dispatch all expose events
		// 4) doDraw()
		// 5) break;

		//
		// TODO: Event translation.
		//
		// Because the X event types for extensions are not fixed, there
		// is a possibility that their values overlap the types I'm using
		// internally.  This step maps External X event types to internal
		// Event.types.
		//

		//
		// Now, send it out
		//
		dispatch( ev, evr );

	} // end of event loop

}

// TODO: Move this into BEvent.cpp
const char* apszEventNames[] = {
	"Unknown",		// 0
	"Unknown",		// 1
	"KeyPress",		// 2		XKeyEvent
	"KeyRelease",	// 3		XKeyEvent
	"ButtonPress",	// 4		XButtonEvent
	"ButtonRelease",	// 5		XButtonEvent
	"MotionNotify",	// 6		XMotionEvent
	"EnterNotify",	// 7		XCrossingEvent
	"LeaveNotify",	// 8		XCrossingEvent
	"FocusIn",		// 9		XFocusChangeEvent
	"FocusOut",		// 10		XFocusChangeEvent
	"KeymapNotify",	// 11		XKeymapEvent
	"Expose",		// 12		XExposeEvent
	"GraphicsExposure",	// 13		XGraphicsExposeEvent
	"NoExposure",	// 14		XNoExposeEvent
	"VisibilityNotify",	// 15		XVisibilityEvent
	"CreateNotify",	// 16		XCreateWindowEvent
	"DestroyNotify",	// 17		XDestroyWindowEvent
	"UnmapNotify",	// 18		XUnmapEvent
	"MapNotify",	// 19		XMapEvent
	"MapRequest",	// 20		XMapRequestEvent
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
	"Unknown",		// 35
	"Unknown",		// 36
	"Unknown",		// 37
	"Unknown",		// 38
	"Unknown",		// 39
	"Activate",		// 40		ActivateEvent
	"PreActivate",	// 41		PreActivateEvent
	"PostActivate",	// 42		PostActivateEvent
	"InvalidateRect",	// 43		XExposeEvent
	"InvalidateParentRect", // 44	XExposeEvent
	"Draw",		// 45		type only
	"Flash",		// 46		?
	"Deactivate",	// 47		type only
	"PreDeactivate",	// 48		type only
	"Reserved",		// 49
	"Reserved"		// 50
};

/*  Scene::dispatch()

    Dispatch one event.  This is used by the messagePump and can be called
    by any Figure.

    Updates 'haveInvalidRegions' if it's an expose or invalidate event.

    TODO: Make this static
*/
void Scene::dispatch( const Event& ev, EventRet& evr )
{
	using std::endl;

	bwassert( ev.type >= 0 );
	bwassert( ev.type < MaxEvent );
	bwassert( ev.xany.display==m_pDisplay );
	//
	// Event tracing
	//
	//trace << "Got event " << ev.type
	//     << " (" << apszEventNames[ev.type]
	//      << ") on window " << ev.xany.window
	//      << endl;

	//
	// Update haveInvalidRegions
	//
	// TODO: move this into Figure
	//
	if (ev.type==Expose || ev.type==InvalidateRect)
		m_haveInvalidRegions = true;

	//
	// Dispatch the event to the window specific code here
	//
	try {
		//
		// Dispatch event to window
		// Note: events for unknown windows are silently ignored.
		//
		Figure* pfig = FigureImp::findFigure( ev.xany.display, ev.xany.window );
		bwassert( pfig );
		if (pfig)
			FigureImp::doEvent( pfig, ev, evr );
	}
	// Uncaught exceptions print message in debug -- ignored in release
	catch( const BException& e ) {
		trace << "Uncaught BW exception in event handling routine:" << endl;
		trace << e.message() << endl;
		trace << "Exception ignored." << endl;
	} catch( const std::exception& e ) {
		trace << "Uncaught C++ runtime exception in event handling routine:" << endl;
		trace << e.what() << endl;
		trace << "Exception ignored." << endl;
	} catch( ... ) {
		trace << "Uncaught unknown exception in event handling routine:" << endl;
		trace << "Exception ignored." << endl;
	}
}

/* Scene::doDraw()

   Starts a redraw of every item in the scene with invalid regions.
*/
void Scene::doDraw()
{
	Event ev;
	EventRet evr;

	ev.type = Draw;
	FigureImp::doEvent( (Figure*)this, ev, evr );
}

/* Scene::absLoc

   Convert Logical co-ordinates to absolute for scene children
*/
void Scene::absLoc(Figure* pfig,
                   LogPos lposX, LogPos lposY, LogPos lposW, LogPos lposH,
                   int& x, int& y, int& w, int& h)
{
	const long maxwidth = 50000;		// Absolute max width
	LogPos lposQX;
	LogPos lposQY;
	LogPos lposQW;
	LogPos lposQH;
	int widthParent = m_pfi->m_width;
	int heightParent = m_pfi->m_height;

	//
	// Handle AskChild values
	//
	if (lposW==AskChild || lposH==AskChild)
		pfig->onQuerySize( lposQW, lposQH );

	if (lposW==AskChild)
		w = lposQW;
	else
		w = lposW;

	if (lposH==AskChild)
		h = lposQH;
	else
		h = lposH;

	if (lposX==AskChild || lposY==AskChild)
		pfig->onQueryLocation( lposQX, lposQY );

	if (lposX==AskChild)
		x = lposQX;
	else
		x = lposX;

	if (lposY==AskChild)
		y = lposQY;
	else
		y = lposY;

	bwassert( x!=AskChild );	// Informative asserts
	bwassert( y!=AskChild );
	bwassert( w!=AskChild );
	bwassert( h!=AskChild );
	if (x==AskChild) x = 0;	// Default actions
	if (y==AskChild) y = 0;
	if (w==AskChild) w = FullWidth;
	if (h==AskChild) h = FullHeight;

	//
	// Now, adjust size
	//
	if (w<0)
		w = (w/(Percentile/100)) * widthParent / 10000;
	else if (w>FullWidth-maxwidth && w<=FullWidth)
		w = widthParent - (FullWidth-w);
	else if (w==MidWidth)
		w = widthParent / 2;

	if (h<0)
		h = (h/(Percentile/100)) * heightParent / 10000;
	else if (h>FullHeight-maxwidth && h<=FullHeight)
		h = heightParent - (FullHeight-h);
	else if (h==MidHeight)
		h = heightParent / 2;

	bwassert( w>=0 );
	bwassert( w<maxwidth );
	bwassert( h>=0 );
	bwassert( h<maxwidth );

	//
	// Now adjust Location
	//
	if (x<0)
		x = (x/(Percentile/100)) * widthParent / 10000;
	else if (x>FullWidth-maxwidth && x<=FullWidth)
		x = widthParent - (FullWidth-x);
	else if (x==MidWidth)
		x = widthParent/2 - w/2;	// Centered

	if (y<0)
		y = (y/(Percentile/100)) * heightParent / 10000;
	else if (y>FullHeight-maxwidth && y<=FullHeight)
		y = heightParent - (FullHeight-y);
	else if (y==MidHeight)
		y = heightParent/2 - h/2;

	bwassert( x>-maxwidth );
	bwassert( x<maxwidth );
	bwassert( y>-maxwidth );
	bwassert( y<maxwidth );
}

}	// namespace bw
