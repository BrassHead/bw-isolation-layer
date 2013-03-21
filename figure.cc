/* figure.cc -- Implementation of Figure class

Copyright (C) 1997-2013, Brian Bray

*/

// Compile time option
#define NOTRACE
#include "bw/trace.h"

#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/context.h"
#include "bw/tools.h"
#include "bw/figure.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include "ffigure.h"
#include "event.h"


namespace bw {

/*: class Figure

  The Figure abstract base class defines an object that can draw itself
  on demand and that may accept user input (mouse and/or keyboard).

  Figure provides a set of services to it's subclasses and provides
  default definitions of an interface called by the BW windowing system.
  All member functions in this interface have default definitions except
  for the 'draw' function.  The expectations and default actions of these
  interface functions and services is detailed below.

  There are a number of "mix-in" base classes that add services and
  interface functions to the basics of the Figure class.  The BW
  windowing system uses the presence of these mix-ins to control:
  <UL>
  <LI>the style of window displayed,
  <LI>the types of adornments displayed,
  <LI>which input events are processed, and
  <LI>some behaviours of the Figure.
  <UL>

  In the long term, The mix-ins available will be:
  <DL>
  <DT><A HREF="Parent.html">Parent</A>
  <DD>Indicates that this Figure can have child figures.
  <DT><A HREF="Size.html">Size</A>
  <DD>Indicates that this figure wants notification of size and location
      changes within it's parent Figure.
  <DT><A HREF="Frame.html">Frame</A>
  <DD>Indicates that OS adornments should be drawn around this figure
      when it is a child of the screen.
  <DT><A HREF="Mouse.html">Mouse</A>
  <DD>Indicates that this figure wants notification when the mouse pointer
      enters or leaves the screen area drawn by the figure or when the
      mouse is clicked over the figure.
  <DT><A HREF="MouseMove.html">MouseMove</A>
  <DD>Indicates that this figure want notification of mouse movements
      when the mouse pointer is over the figure.
  <DT><A HREF="Focus.html">Focus</A>
  <DD>Indicates that this figure accepts keyboard input and can be the Keyboard
      focus.
  <DT><A HREF="RawKeyboard.html">RawKeyboard</A>
  <DD>Indicates that this Figure needs detailed information about keypresses
      and releases.
  <DT><A HREF="VScroll.html">VScroll</A>
  <DD>Indicates that this figure wants notification of vertical scrolling
      events when it is a child of a scrolling figure.
  <DT><A HREF="HScroll.html">HScroll</A>
  <DD>Indicates that this figure wants notification of horizontal scrolling
      events when it is a child of a scrolling figure.
  <DT><A HREF="Main.html">Main</A>
  <DD>Indicates that this Figure is a primary interface of the application
      and will be displayed on startup.
  </DL>

*/

/*: Figure::Figure()

  Constructor
*/
Figure::Figure()
{
	// Initialize FigureBase members (since Figurebase cannot have a
	// constructor due to egc++ defect
	m_pfi = 0;
	m_pParent = 0;
}

/*: Figure::~Figure()

  Destructor
*/
Figure::~Figure()
{
	if (m_pParent) {
		m_pParent->removeChild( this );
		m_pParent = 0;
	}
}


/*: Figure::doAction()

  Initiate processing of an Action routine.

  Processing starts at the parent of the calling control.  The Command argument
  contains a string indicating which action should be taken.  The syntax of the
  commands is application dependent.

  Action processing generally goes up the Figure hierarchy until an ancestor
  recognizes the command and takes the action.  If processing reaches the
  screen, then the command is posted for processing by the 'main' routine in
  applications Process object.

  A 'true' return indicates that an ancestor processed the command.  A 'false'
  return indicates that the command was posted to the 'main' routine if the
  originating Figure is active or was discarded for inactive Figures.

  The default 'main' routine handles the single command "quit" and ignores all
  others.
*/
bool Figure::doAction( const String& strCommand )
{
	// Note: I decided to call the parent routine directly instead of
	// making an event...seemed safe since actions are primarily a client
	// concept.
	if (m_pParent)
		return m_pParent->onAction( strCommand );
	else
		return false;
}

/*: Figure::isActive()

  Returns true if this Figure is "active".  I.E. part of a scene.
*/
bool Figure::isActive()
{
	return m_pfi != 0;
}

/*: Figure::getSize()

  Returns the size of the Figure in pixels.  The Figure must be active. or
  an error is thrown.
*/
void Figure::getSize( int& width, int& height )
{
	if (isActive()) {
		bwassert( m_pfi );
		width = m_pfi->m_width;
		height = m_pfi->m_height;
	} else {
		bwassert(false);		// TODO: throw something
	}
}


/*: Figure::getLocation()

  Returns the current location of the Figure in relation to it's parent
  (in pixels).

  The Figure must be active.
*/
void Figure::getLocation( unsigned int& x, unsigned int& y )
{
	if (isActive()) {
		bwassert( m_pfi );
		x = m_pfi->m_x;
		y = m_pfi->m_y;
	} else {
		bwassert(false);		// TODO: throw something
	}
}

/*: Figure::invalidate()

  Invalidates the entire drawing surface of a Figure.

  This will eventually cause a draw call to refresh the display.
*/
void Figure::invalidate()
{
	if (isActive()) {
		bwassert( m_pfi );
		m_pfi->m_ctx.invalidate();
	}
}


/*: Figure::getScene()

  Returns the Scene for which this active figure is a part.

  Throws BGUIError if the scene is not active or in the process of becoming
  active.
*/
Scene& Figure::getScene() const
{
	bwassert( m_pfi );		// TODO: Throw on error

	return *(m_pfi->m_pscene);
}




/* Figure::getpFigure()

   This is a convenience routine for my mix-in classes so that they can get
   allways get a pointer to the Figure Interface from any of the others.
*/
Figure* Figure::getpFigure()
{
	return this;
}

void Figure::doEvent( const Event& ev, EventRet& evr )
{
	// This routine constitutes the "event" handler chain for Figures.
	// The advantages of this design are it's relative simplicity and
	// lack of downcasts.  The disadvantage is that the types of things
	// on the event chain is fixed and that all items need to be in subclasses.

	doFigureEvent( ev, evr );
	doParentEvent( ev, evr );
	doSizeEvent( ev, evr );
	doFrameEvent( ev, evr );
	doMouseEvent( ev, evr );
	doMouseMoveEvent( ev, evr );
	doFocusEvent( ev, evr );
	doRawKeyboardEvent( ev, evr );
}


/* Figure::doFigureEvent()

   This is the event handler for the Figure class
*/
void Figure::doFigureEvent( const Event& ev, EventRet& evr )
{
	// 	This needs to process: flash, draw, window creation, window deletion,
	//	Scene entry and exit, window mapping, and determining
	//	window attributes.
	switch( ev.type ) {
	case Expose:
	case InvalidateRect:
		bwassert( m_pfi );
		m_pfi->m_ctx.addClip( ev.xexpose.x, ev.xexpose.y,
		                      ev.xexpose.width, ev.xexpose.height );
		break;

	case Draw:
		bwassert( m_pfi );
		if (m_pfi->m_ctx.needRedraw()) {
			Canvas cvs( m_pfi );
			draw( cvs );
			m_pfi->m_ctx.reset();
		}
		break;

	case Activate:
		bwassert( !m_pfi );
		bwassert( ev.evAct.display );
		bwassert( ev.evAct.parent );
		bwassert( ev.evAct.m_pfigParent );
		bwassert( ev.evAct.m_pfignParent );
		bwassert( ev.evAct.m_pfigParent->m_pfi );
		{
			m_pfi = new FigureImp;
			m_pfi->m_pDisplay = ev.evAct.display;
			m_pfi->m_pgdev = ev.evAct.m_pfigParent->m_pfi->m_pgdev;
			m_pfi->m_pscene = ev.evAct.m_pfigParent->m_pfi->m_pscene;
			m_pfi->m_wnd = 0;
			m_pfi->m_wndParent = ev.evAct.parent;
			m_pfi->m_pfigThis = this;
			m_pfi->m_pfigParent = ev.evAct.m_pfigParent;
			m_pfi->m_pfignParent = ev.evAct.m_pfignParent;
			activate();
		}
		onEnterScene();		// Let the Figure know
		break;

	case PreActivate:
		bwassert( m_pfi );
		m_pfi->m_xswa.event_mask |= ExposureMask | StructureNotifyMask;
		evr.retcode |= CWBackPixel;		// TODO: Selective background
		break;

	case Flash:
		bwassert( m_pfi );
		onFlash();
		break;

	case Deactivate:
		onExitScene();		// Let the Figure know
		deactivate();
		delete m_pfi;
		m_pfi = 0;
		break;

	case ConfigureNotify:
		bwassert( m_pfi );
		if (ev.xconfigure.window==m_pfi->m_wnd) {
			// It's for me
			bool isMoved = (m_pfi->m_x!=ev.xconfigure.x ||
			                m_pfi->m_y!=ev.xconfigure.y );
			bool isResize = (m_pfi->m_width!=ev.xconfigure.width ||
			                 m_pfi->m_height!=ev.xconfigure.height );
			m_pfi->m_x = ev.xconfigure.x;
			m_pfi->m_y = ev.xconfigure.y;
			m_pfi->m_width = ev.xconfigure.width;
			m_pfi->m_height = ev.xconfigure.height;
			Event evNew(ev);
			if (isMoved && isResize)
				evNew.type = RelocateNotify;
			else if (isMoved)
				evNew.type = MoveNotify;
			else if (isResize)
				evNew.type = SizeNotify;
			if (evNew.type!=ConfigureNotify)
				doEvent( evNew, evr );
		}
		break;

	case GravityNotify: {
		bwassert( m_pfi );
		m_pfi->m_x = ev.xgravity.x;
		m_pfi->m_y = ev.xgravity.y;
		Event evNew;
		evNew.type = MoveNotify;
		evNew.xconfigure.x = ev.xgravity.x;
		evNew.xconfigure.y = ev.xgravity.y;
		doEvent( evNew, evr );
	}
	break;

	case Reposition: {
		bwassert( m_pfi );
		int newx, newy, neww, newh, newwg;
		computeGeometry(newx,newy,neww,newh,newwg);
		if (newwg!=m_pfi->m_xswa.win_gravity) {
			m_pfi->m_xswa.win_gravity = newwg;
			XChangeWindowAttributes(m_pfi->m_pDisplay,
			                        m_pfi->m_wnd,
			                        CWWinGravity,
			                        &(m_pfi->m_xswa) );
		}
		bool isMove = (newx!=m_pfi->m_x || newy!=m_pfi->m_y);
		bool isResize = (neww!=m_pfi->m_width || newh!=m_pfi->m_height);
		if (isMove && isResize)
			XMoveResizeWindow(m_pfi->m_pDisplay,
			                  m_pfi->m_wnd,
			                  newx, newy,
			                  neww, newh );
		else if (isMove)
			XMoveWindow(m_pfi->m_pDisplay,
			            m_pfi->m_wnd,
			            newx, newy );
		else if (isResize)
			XResizeWindow(m_pfi->m_pDisplay,
			              m_pfi->m_wnd,
			              neww, newh );
	}
	break;

	case DestroyNotify:
		bwassert( false );
		break;

	case UnmapNotify:
		if (ev.xunmap.from_configure && m_pfi->m_xswa.win_gravity==UnmapGravity) {
			Event evNew;
			evNew.type = Reposition;
			// Note: trial to see if I can get unmap notify messages on
			// subwindows
			XMapWindow( m_pfi->m_pDisplay, m_pfi->m_wnd );
			doEvent( evNew, evr );
		}
		break;

	default:
		break;
	}

}

// TODO: Document the virtual functions

/* Figure::activate()

   Activate the figure.  This includes creating a window with the
   correct attributes, position, and size.

   On entry, m_pfi points to a structure with the structure elements (parent,
   etc. filled in.

   On exit, the remaining FigureImp fields (m_wnd, m_xswa, size,
   and location) will be set.
*/
void Figure::activate()
{
	using std::endl;

	bwassert( m_pfi );
	bwassert( m_pfi->m_pDisplay );
	bwassert( m_pfi->m_wnd==0 );
	bwassert( m_pfi->m_wndParent );
	//
	// set everything to X defaults
	//
	m_pfi->m_xswa.background_pixmap = None;
	m_pfi->m_xswa.background_pixel =
	    WhitePixel( m_pfi->m_pDisplay, DefaultScreen(m_pfi->m_pDisplay) );
	m_pfi->m_xswa.border_pixmap = CopyFromParent;
	m_pfi->m_xswa.border_pixel =
	    BlackPixel( m_pfi->m_pDisplay, DefaultScreen(m_pfi->m_pDisplay) );
	m_pfi->m_xswa.bit_gravity = ForgetGravity;
	m_pfi->m_xswa.win_gravity = NorthWestGravity;
	m_pfi->m_xswa.backing_store = NotUseful;
	m_pfi->m_xswa.backing_planes = ~0L;
	m_pfi->m_xswa.backing_pixel = 0;
	m_pfi->m_xswa.save_under = False;
	m_pfi->m_xswa.event_mask = 0;
	m_pfi->m_xswa.do_not_propagate_mask = 0;
	m_pfi->m_xswa.override_redirect = False;
	m_pfi->m_xswa.colormap = CopyFromParent;
	m_pfi->m_xswa.cursor = None;
	//
	// Set the size and location fields
	//
	computeGeometry(m_pfi->m_x,m_pfi->m_y,m_pfi->m_width,m_pfi->m_height,m_pfi->m_xswa.win_gravity);
	//
	// Give each mix-in a crack at setting xswa attributes.
	//
	Event ev;
	EventRet evr;
	ev.type = PreActivate;
	evr.retcode = CWEventMask | CWWinGravity;		// We always set these
	doEvent( ev, evr );
	//
	// Create the window (note: I always create a window in this version)
	//
	trace << "Creating window:" << endl;
	trace << "\tm_pDisplay = " << m_pfi->m_pDisplay << endl;
	trace << "\tm_wndParent = " << m_pfi->m_wndParent << endl;
	trace << "\tm_x = " << m_pfi->m_x << ",  m_y = " << m_pfi->m_y << endl;
	trace << "\tm_width = " << m_pfi->m_width << ",  m_height = " << m_pfi->m_height << endl;
	trace << "\tWinGravity = " << m_pfi->m_xswa.win_gravity << endl;
	trace << "\tDepth = " << CopyFromParent << endl;
	trace << "\tClass = " << InputOutput << endl;
	trace << "\tVisual = " << (Visual*)CopyFromParent << endl;
	trace << "\tValue Mask = " << evr.retcode << endl;
	trace << "\t&xswa = " << &(m_pfi->m_xswa) << endl << endl;

	m_pfi->m_wnd = XCreateWindow( m_pfi->m_pDisplay,
	                              m_pfi->m_wndParent,
	                              m_pfi->m_x, m_pfi->m_y,
	                              m_pfi->m_width, m_pfi->m_height,
	                              0,			// Border width
	                              CopyFromParent,	// Depth
	                              InputOutput,		// Class
	                              (Visual*)CopyFromParent,	// Visual
	                              evr.retcode,		// value mask
	                              &(m_pfi->m_xswa) );
	//
	// Reverse pointer so Figure can be found
	//
	m_pfi->setFigure();

	//
	// Give mix-ins a chance to do their own window initialization
	//
	ev.type = PostActivate;
	evr.retcode = 0;
	doEvent( ev, evr );

	//
	// Finally, show the window
	//
	XMapWindow( m_pfi->m_pDisplay, m_pfi->m_wnd );

}



/* Figure::deactivate()

   Deactivate this Figure.
*/
void Figure::deactivate()
{
	bwassert( m_pfi );
	bwassert( m_pfi->m_pDisplay );
	bwassert( m_pfi->m_wnd );

	//
	// Give fair warning
	//
	Event ev;
	EventRet evr;
	ev.type = PreDeactivate;
	doEvent( ev, evr );

	//
	// Remove reverse pointer
	//
	m_pfi->unsetFigure();

	//
	// Delete the window
	//
	XDestroyWindow( m_pfi->m_pDisplay, m_pfi->m_wnd );
	m_pfi->m_wnd = 0;
	m_pfi->m_pDisplay = 0;
}



/*  Figure::computeGeometry()

    Compute actual size and position from Relative size and position
*/
void Figure::computeGeometry(int& x, int& y, int& w, int& h, int& winGravity)
{
	const long maxwidth = 50000;		// Absolute max width
	bwassert( m_pfi );
	bwassert( m_pfi->m_pfigParent );
	bwassert( m_pfi->m_pfignParent );
	bwassert( m_pfi->m_pfigParent->isActive() );

	bool forceUnmap = false;
	int relX = -1;
	int relY = -1;

	//
	// Retrieve size of Parent
	//
	int widthParent;
	int heightParent;
	m_pfi->m_pfigParent->getSize( widthParent, heightParent );
	//
	// Retrieve Logical geometry
	//
	x = m_pfi->m_pfignParent->m_lposX;
	y = m_pfi->m_pfignParent->m_lposY;
	w = m_pfi->m_pfignParent->m_lposWidth;
	h = m_pfi->m_pfignParent->m_lposHeight;
	//
	//	Handle the queries first
	//
	if (w==AskChild || h==AskChild) {
		forceUnmap = true;
		LogPos qw, qh;
		onQuerySize( qw, qh );
		if (w==AskChild)
			w = qw;
		if (h==AskChild)
			h = qh;
	}
	if (x==AskChild || y==AskChild) {
		forceUnmap = true;
		LogPos qx, qy;
		onQueryLocation( qx, qy );
		if (x==AskChild)
			x = qx;
		if (y==AskChild)
			y = qy;
	}
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
	if (w<0 || w>maxwidth || h<0 || h>maxwidth)
		forceUnmap = true;
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
	if (x<0) {
		forceUnmap = true;
		x = (x/(Percentile/100)) * widthParent / 10000;
	} else if (x>FullWidth-maxwidth && x<=FullWidth) {
		relX = 1;
		x = widthParent - (FullWidth-x);
	} else if (x==MidWidth) {
		relX = 0;
		x = widthParent/2 - w/2;	// Centered
	}

	if (y<0) {
		forceUnmap = true;
		y = (y/(Percentile/100)) * heightParent / 10000;
	} else if (y>FullHeight-maxwidth && y<=FullHeight) {
		relY = 1;
		y = heightParent - (FullHeight-y);
	} else if (y==MidHeight) {
		relY = 0;
		y = heightParent/2 - h/2;
	}

	bwassert( x>-maxwidth );
	bwassert( x<maxwidth );
	bwassert( y>-maxwidth );
	bwassert( y<maxwidth );

	// Now set winGravity
	if (forceUnmap)
		winGravity = UnmapGravity;
	else if (relX<0) {
		if (relY<0)
			winGravity = NorthWestGravity;
		else if (relY==0)
			winGravity = WestGravity;
		else
			winGravity = SouthWestGravity;
	} else if (relX==0) {
		if (relY<0)
			winGravity = NorthGravity;
		else if (relY==0)
			winGravity = CenterGravity;
		else
			winGravity = SouthGravity;
	} else {
		if (relY<0)
			winGravity = NorthEastGravity;
		else if (relY==0)
			winGravity = EastGravity;
		else
			winGravity = SouthEastGravity;
	}
}


/*: Class Main

  Indicates that Figure should be shown on startup as the applications
  main Figure.
*/

Main* Main::m_pTheMain = 0;

/*: Main::Main()

  Constructor
*/
Main::Main()
{
	// This routine just squirrels away the 'this' pointer so that
	// Process::main() can find it.
	// It cannot be added as a child of the screen
	// because the Screen may not exist yet.
	bwassert( !m_pTheMain );
	m_pTheMain = this;
}

/*: Main::~Main()

  Destructor
*/
Main::~Main()
{
	// This may do nothing, although it's probably a good idea to notice
	// if no UI is left.  Removing the Figure from it's parent (ie: from the
	// screen) is the job of Figure::~Figure.
	m_pTheMain = 0;
}

/*: Main::getMain()

  Returns a pointer to the Main figure for the application.

  Null is returned if no figure has been defined with the Main mix-in base
  class.
*/
Figure* Main::getMain()
{
	return m_pTheMain->getpFigure();
}

}   // namespace













