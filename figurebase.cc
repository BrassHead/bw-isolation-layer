/* figurebase.cc -- Implementation of the FigureBase class

Copyright (C) 1997-2013, Brian Bray

*/


#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/figure.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include "event.h"

using namespace bw;

/* class FigureBase

   The FigureBase class is an implementation class that is a common base class
   for figure and all the mix-in classes that go with figure.

   It's main job is to handle communication between it's direct descendants.

   The primary means for this communication is an event processing chain.
   Initially, I thought that I would construct this chain during the
   constructors of these child classes, but it appears that using some
   virtual functions is a cleaner design.
*/

//FigureBase::FigureBase()
//: m_pfi( 0 ),
//  m_pParent( 0 )
//{
//    // Constructor intentionally empty
//}

//FigureBase::~FigureBase()
//{
//    // These are managed by the Figure subclass
//    bwassert( m_pParent==0 );
//    bwassert( m_pfi==0 );
//}

/*  Following are the default event handlers for each mix-in type.
    Right now, they do nothing, but these spots are available in case
    a default action (such as forwarding to a parent) is needed for some
    events.
*/

void FigureBase::doParentEvent( const Event&, EventRet& )
{}

void FigureBase::doSizeEvent( const Event&, EventRet& )
{}

void FigureBase::doFrameEvent( const Event&, EventRet& )
{}

void FigureBase::doMouseEvent( const Event&, EventRet& )
{}

void FigureBase::doMouseMoveEvent( const Event&, EventRet& )
{}

void FigureBase::doFocusEvent( const Event&, EventRet& )
{}

void FigureBase::doRawKeyboardEvent( const Event&, EventRet& )
{}



