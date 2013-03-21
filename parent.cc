/* parent.cc -- Implementation of Parent mix-in for Figure class

Copyright (C) 1997-2013, Brian Bray

*/

// Mainly comments right now

#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/figure.h"
#include "bw/context.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include "ffigure.h"
#include "event.h"

namespace bw {

/*: class Parent

   Mix-in class for Parent Figures.

   The Parent mix-in class provides services for managing child and owned
   Figures and supports a set of events related to child activity.

   It maintains a pointer in each Parent to a linked list node for each child.
   The node records a pointer to each child along with its logical position
   and size.

   Each child also has a pointer to it's parent.
*/

/*: Parent::Parent()

  Constructor
*/
Parent::Parent()
	:   m_pfignFirst( 0 )
{
	// Intentionally empty
}


/*: Parent::~Parent()

  Destructor
*/
Parent::~Parent()
{
	removeAllChildren();
	bwassert( !m_pfignFirst );
}

/*: Parent::addChild()

  Adds a child figure.

  If a size is not given, the child is queried for it's desired size.

  Prototype: addChild( Figure* fig,
         LogPos lposX=0, LogPos lposY=0,
	 LogPos lposWidth=FullWidth, LogPos lposHeight=FullHeight );

  Description:
  The X and Y values give the location of the child within the parent.
  (0,0) is the upper left corner with increasing x values to the right and
  increasing y values going down.

  All values are Logical Positions (LogPos).  LogPos is a typedef to an
  platform dependent integral type.

  Logical Positions
  can either be absolute positions and lengths
  measured in pixels or specifications relative to the size of the parent
  Figure as follows:
  <UL>
  <LI>FullWidth-x -- x pixels from the right hand side of the parent
  <LI>FullHeight-y -- y pixels from the bottom
  <LI>MidWidth+/-x -- x pixels offset from the center of the parent
  <LI>MidHeight+/-y -- y pixels offset from the center of the parent
  <LI>Percentile*x -- x% of the parent width or height
  <LI>AskChild -- asks child control for desired location or size
  </UL>

  There is no requirement or guarantee that the location and size of the child
  control be within the drawing area of the parent.
  Negative x and y co-ordinates are permitted, but sizes must be positive.
*/
void Parent::addChild( Figure* pfig,
                       LogPos lposX, LogPos lposY,
                       LogPos lposWidth, LogPos lposHeight )
{
	//
	// First, create a new FigureNode
	//
	FigureNode* pfign = new FigureNode;
	pfign->m_pfigParent = getpFigure();
	pfign->m_pfigChild = pfig;
	pfign->m_lposX = lposX;
	pfign->m_lposY = lposY;
	pfign->m_lposWidth = lposWidth;
	pfign->m_lposHeight = lposHeight;

	//
	// Now add to start of list
	//
	pfign->m_pfignNext = m_pfignFirst;
	pfign->m_pfignPrev = 0;
	if (m_pfignFirst)
		m_pfignFirst->m_pfignPrev = pfign;
	m_pfignFirst = pfign;

	//
	// Reverse link from child
	//
	FigureImp::setParent( pfig, this );

	//
	// If the parent control is active, then a lot more work is required
	// most of which is done by the child itself (via an Event)
	//
	if (getpFigure()->isActive()) {
		bwassert( m_pfi );
		EventRet evr;
		Event ev;
		makeActivateEvent( ev,
		                   m_pfi->m_pDisplay,
		                   m_pfi->m_wnd,
		                   getpFigure(),
		                   pfign );
		FigureImp::doEvent( pfig, ev, evr );
	}

}

/*: Parent::removeChild()

  Removes a child Figure from under this Figure.

  The child Figure is not deleted.

  If the specified Figure is not a child, this request is silently ignored.
*/
void Parent::removeChild( Figure* pfig ) throw()
{
	//
	// Find child FigureNode
	//
	FigureNode* pfign;
	for (pfign=m_pfignFirst; pfign; pfign=pfign->m_pfignNext)
		if (pfig == pfign->m_pfigChild)
			break;
	bwassert( pfign );		// Removal of non-existant child
	if (pfign) {
		//
		// If I'm active, deactivate the child before culling
		//
		if (getpFigure()->isActive()) {
			Event ev;
			EventRet evr;
			ev.type = Deactivate;
			FigureImp::doEvent( pfig, ev, evr );
		}
		//
		// Remove reverse link
		//
		FigureImp::setParent( pfig, 0 );

		//
		// Cull FigureNode
		//
		if (pfign->m_pfignPrev)
			pfign->m_pfignPrev->m_pfignNext = pfign->m_pfignNext;
		else
			m_pfignFirst = pfign->m_pfignNext;

		if (pfign->m_pfignNext)
			pfign->m_pfignNext->m_pfignPrev = pfign->m_pfignPrev;

		delete pfign;
	}
}



/*: Parent::removeAllChildren()

  Removes all child Figures from under this one.
*/
void Parent::removeAllChildren() throw()
{
	while (m_pfignFirst)
		removeChild( m_pfignFirst->m_pfigChild );
}



/*: Parent::numChildren()

  Returns the number of children currently parented.
*/
int Parent::numChildren()
{
	int count = 0;
	if (m_pfignFirst) {
		for (FigureNode* pfign=m_pfignFirst; pfign; pfign=pfign->m_pfignNext)
			++count;
	}

	return count;
}


/*: Parent::moveChild()

  Moves the logical position of the specified child figure.
*/
void Parent::moveChild( Figure* pfig, LogPos lposX, LogPos lposY )
{
	// Find child FigureNode

	FigureNode* pfign;
	for (pfign=m_pfignFirst; pfign; pfign=pfign->m_pfignNext)
		if (pfig == pfign->m_pfigChild)
			break;
	bwassert( pfign );	// Not my child!
	if (pfign) {
		// Change logical position
		pfign->m_lposX = lposX;
		pfign->m_lposY = lposY;

		// If I'm active, then child needs a reposition
		if (getpFigure()->isActive()) {
			Event ev;
			EventRet evr;
			ev.type = Reposition;
			FigureImp::doEvent( pfign->m_pfigChild, ev, evr );
		}
	}
}

/*: Parent::resizeChild()

  Changes the logical size of the specified child figure.
*/
void Parent::resizeChild( Figure* pfig, LogPos lposWidth, LogPos lposHeight )
{
	// Find child FigureNode

	FigureNode* pfign;
	for (pfign=m_pfignFirst; pfign; pfign=pfign->m_pfignNext)
		if (pfig == pfign->m_pfigChild)
			break;
	bwassert( pfign );	// Not my child!
	if (pfign) {
		// Change logical size
		pfign->m_lposWidth = lposWidth;
		pfign->m_lposHeight = lposHeight;

		// If I'm active, then child needs a reposition
		if (getpFigure()->isActive()) {
			Event ev;
			EventRet evr;
			ev.type = Reposition;
			FigureImp::doEvent( pfign->m_pfigChild, ev, evr );
		}
	}
}

/*: Parent::relocateChild()

  Changes the logical size and position of the specified child figure.
*/
void Parent::relocateChild(
    Figure* pfig,
    LogPos lposX, LogPos lposY,
    LogPos lposWidth, LogPos lposHeight )
{
	// Find child FigureNode

	FigureNode* pfign;
	for (pfign=m_pfignFirst; pfign; pfign=pfign->m_pfignNext)
		if (pfig == pfign->m_pfigChild)
			break;
	bwassert( pfign );	// Not my child!
	if (pfign) {
		// Change logical size and position
		pfign->m_lposX = lposX;
		pfign->m_lposY = lposY;
		pfign->m_lposWidth = lposWidth;
		pfign->m_lposHeight = lposHeight;

		// If I'm active, then child needs a reposition
		if (getpFigure()->isActive()) {
			Event ev;
			EventRet evr;
			ev.type = Reposition;
			FigureImp::doEvent( pfign->m_pfigChild, ev, evr );
		}
	}
}

/*: Parent::OnMoveRequest

  Called when a child figure has requested a change in position.

  The default action is to grant the request by calling moveChild.

  Prototype: void onMoveRequest( Figure* fig, LogPos lposX, LogPos lposY )
*/

/*: Parent::OnResizeRequest

  Called when a child figure has requested a change in size.

  The default action is to grant the request by calling resizeChild.

  Prototype: void onResizeRequest( Figure* fig, LogPos lposWidth, LogPos lposHeight )
*/

/*: Parent::OnRelocateRequest

  Called when a child figure has requested a change in location and size.

  The default action is to grant the request by calling relocateChild.

  Prototype: void onRelocateRequest( Figure* fig, LogPos lposX, LogPos lposY, LogPos lposWidth, LogPos lposHeight )
*/


/*: Parent::onAction

  Handle action commands from descendent Figures.

  This routine is called whenever a descendent Figure has called 'doAction'
  and no intervening Figure has handled the action.

  The routine should check the Command argument for actions appropriate to
  this Figure and return true if the action is fully processed.

  Unprocessed actions should be forwarded to the parent Figure by calling
  doAction with the same parameter.

  By default all actions are forwarded.

  Prototype: bool onAction( const String& strCommand )

*/
bool Parent::onAction( const String& strCommand )
{
	return getpFigure()->doAction(strCommand);
}

/* Parent::doParentEvent

   Internal event handler for parent controls.
*/
void Parent::doParentEvent( const Event& ev, EventRet& evr )
{
	// This needs to handle events for Child Destruction, resizing, ...
	switch( ev.type ) {
	case PreActivate:
		bwassert( m_pfi );
		m_pfi->m_xswa.event_mask |= SubstructureNotifyMask;
		break;

	case PostActivate:
		bwassert( m_pfi );
		//
		// Activate all my children
		//
		for (FigureNode* pfign=m_pfignFirst; pfign; pfign=pfign->m_pfignNext) {
			if (!pfign->m_pfigChild->isActive()) {
				Event evAct;
				EventRet evrAct;

				makeActivateEvent( evAct,
				                   m_pfi->m_pDisplay,
				                   m_pfi->m_wnd,
				                   getpFigure(),
				                   pfign );
				FigureImp::doEvent( pfign->m_pfigChild, evAct, evrAct );
			}
		}
		break;

	case PreDeactivate:
		bwassert( m_pfi );
		//
		// Deactivate all my children
		//
		for (FigureNode* pfign=m_pfignFirst; pfign; pfign=pfign->m_pfignNext) {
			Event evDeact;
			EventRet evrDeact;
			evDeact.type = Deactivate;
			FigureImp::doEvent( pfign->m_pfigChild, evDeact, evrDeact );
		}
		break;

	case Flash:
	case Draw:
		//
		// Pass it on (unchanged)
		//
		for (FigureNode* pfign=m_pfignFirst; pfign; pfign=pfign->m_pfignNext) {
			FigureImp::doEvent( pfign->m_pfigChild, ev, evr );
		}
		break;

	case InvalidateRect:
		//
		// Pass it on (as an InvalidateParentRect)
		//
	{
		Event evRect = ev;
		evRect.type = InvalidateParentRect;
		for (FigureNode* pfign=m_pfignFirst; pfign; pfign=pfign->m_pfignNext) {
			FigureImp::doEvent( pfign->m_pfigChild, evRect, evr );
		}
	}
	break;

	case CirculateNotify:
		bwassert( false );	// Should not occur
		break;

	case ConfigureNotify:
		bwassert( m_pfi );
		if (ev.xconfigure.window!=m_pfi->m_wnd) {
			// It's for one of my kids
			// TODO: Adjust stacking order
		}
		break;

	case DestroyNotify:
		// TODO: make sure it's gone?
		break;

	case SizeRequest:
		onResizeRequest(ev.evSizeReq.pfig,
		                ev.evSizeReq.lposWidth,
		                ev.evSizeReq.lposHeight);
		break;

	case MoveRequest:
		bwassert( m_pfi );
		onMoveRequest(ev.evSizeReq.pfig,
		              ev.evSizeReq.lposX,
		              ev.evSizeReq.lposY);
		break;

	case RelocateRequest:
		bwassert( m_pfi );
		onRelocateRequest(ev.evSizeReq.pfig,
		                  ev.evSizeReq.lposX,
		                  ev.evSizeReq.lposY,
		                  ev.evSizeReq.lposWidth,
		                  ev.evSizeReq.lposHeight);
		break;

	default:
		break;
	}

}

}	// namespace bw

