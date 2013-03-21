/* figure.h -- Interface for Figure class

Copyright (C) 1997-2013, Brian Bray

*/

/* Needs:
#include "bw/string.h"
*/

namespace bw {

class Canvas;
class Figure;
class Parent;
union Event;
class EventRet;
class FigureImp;
class FigureNode;
class Scene;


class FigureBase {
protected:
	//FigureBase();
	//virtual ~FigureBase();
	virtual Figure* getpFigure() = 0;
	virtual void doFigureEvent( const Event& ev, EventRet& evr ) = 0;
	virtual void doParentEvent( const Event& ev, EventRet& evr );
	virtual void doSizeEvent( const Event& ev, EventRet& evr );
	virtual void doFrameEvent( const Event& ev, EventRet& evr );
	virtual void doMouseEvent( const Event& ev, EventRet& evr );
	virtual void doMouseMoveEvent( const Event& ev, EventRet& evr );
	virtual void doFocusEvent( const Event& ev, EventRet& evr );
	virtual void doRawKeyboardEvent( const Event& ev, EventRet& evr );
protected:
	friend class FigureImp;
	FigureImp*	m_pfi;
	Parent*	m_pParent;
};

typedef int LogPos;	// needs minimum 32bit integral type
const LogPos Percentile=-100000;
const LogPos FullWidth=100000;
const LogPos FullHeight=100000;
const LogPos MidWidth=200000;
const LogPos MidHeight=200000;
const LogPos AskChild=300000;

class Figure : virtual private FigureBase {	// Note: Protected not impl in egcs
public:  // Services
	Figure();
	virtual ~Figure();
	bool doAction( const String& strCommand );
	bool isActive();
	void getSize( int& width, int& height );
	void getLocation( unsigned int& x, unsigned int& y );
	void invalidate();
	Scene& getScene() const;

public: // Interface
	virtual void draw( Canvas& cvs ) = 0;
	virtual void onFlash() {}
	virtual void onEnterScene() {}
	virtual void onExitScene() {}
	virtual void onQuerySize( LogPos& lposWidth, LogPos& lposHeight ) {
		lposWidth = FullWidth;
		lposHeight = FullHeight;
	}
	virtual void onQueryLocation( LogPos& lposX, LogPos& lposY ) {
		lposX = 0;
		lposY = 0;
	}

private:  // Internal use
	friend class FigureImp;
	void doEvent( const Event& ev, EventRet& evr );
	Figure* getpFigure();
	void doFigureEvent( const Event& ev, EventRet& evr );
	void activate();
	void deactivate();
	void computeGeometry(int&,int&,int&,int&,int&);
};

class Parent : private virtual FigureBase {
public: // Services
	Parent();
	virtual ~Parent();
	void addChild( Figure* pfig,
	               LogPos lposX=0, LogPos lposY=0,
	               LogPos lposWidth=FullWidth, LogPos lposHeight=FullHeight );
	void removeChild( Figure* pfig ) throw();
	void removeAllChildren() throw();
	int numChildren();
	//FigureIter getChildren();
	//addModal(...);
	void moveChild( Figure* fig, LogPos lposX, LogPos lposY );
	void resizeChild( Figure* fig, LogPos lposWidth, LogPos lposHeight );
	void relocateChild( Figure* fig,
	                    LogPos lposX, LogPos lposY,
	                    LogPos lposWidth, LogPos lposHeight );
	//replaceChild( Figure* figOld, Figure* figNew );
	//raiseChild( Figure* figChild );
	//lowerChild( Figure* figChild );

public: // Interface
	virtual bool onAction(const String& strCommand); //{return doAction(strCommand);}
	virtual void onResizeRequest( Figure* fig, LogPos lposWidth, LogPos lposHeight ) {
		resizeChild( fig, lposWidth, lposHeight );
	}
	virtual void onMoveRequest( Figure* fig, LogPos lposX, LogPos lposY ) {
		moveChild( fig, lposX, lposY );
	}
	virtual void onRelocateRequest( Figure* fig, LogPos lposX, LogPos lposY,
	                                LogPos lposWidth, LogPos lposHeight ) {
		relocateChild( fig, lposX, lposY, lposWidth, lposHeight );
	}
//    virtual void onRaiseRequest( Figure* fig );
//    virtual void onLowerRequest( Figure* fig );
//    virtual void onReplaceRequest( Figure* fig, Figure* new );

private:
	friend class FigureImp;
	virtual void doParentEvent( const Event& ev, EventRet& evr );
	FigureNode*	m_pfignFirst;
};



class Size : private virtual FigureBase {
public: // Services
	Size() {};
	virtual ~Size() {};

	virtual void requestResize( LogPos lposWidth, LogPos lposHeight );
	virtual void requestMove( LogPos lposX, LogPos lposY );
	virtual void requestRelocate( LogPos lposX, LogPos lposY,
	                              LogPos lposWidth, LogPos lposHeight );
	//virtual void requestRaise(...)
	//virtual void requestLower(...)
	//virtual void requestReplace(...)

public: // Interface
	virtual void onResize() {}
	virtual void onMove() {}
	virtual void onRelocate() {}

private:
	virtual void doSizeEvent( const Event& ev, EventRet& evr );
};



class Frame : private virtual FigureBase {
public: // Services
	Frame();
	virtual ~Frame();
	void setTitle( String strTitle );

public: // Interface
	virtual void onClose();

private:
	virtual void doFrameEvent( const Event& ev, EventRet& evr );
	String	m_strTitle;
};

class Mouse : private virtual FigureBase {
public:
	Mouse() {};
	virtual ~Mouse() {};

public: // Overridables
	virtual void onMouseEnter() {}
	virtual void onMouseExit() {}
	virtual void onMouseButton(int /*button*/, bool /*isPress*/) {}

private:
	virtual void doMouseEvent( const Event& ev, EventRet& evr );
};

class Focus : private virtual FigureBase {
public:
	Focus();
	virtual ~Focus();
	void takeFocus();
	void releaseFocus();
	void tabFocus();

public: //Virtuals
	virtual void onGotFocus() {}
	virtual void onLostFocus() {}
	virtual void onCharacter( char ) {}

private:
	virtual void doFocusEvent( const Event& ev, EventRet& evr );
};

class Main : private virtual FigureBase {
public:
	Main();
	virtual ~Main();
	static Figure* getMain();

private:
	static Main* m_pTheMain;
};


}   // namespace


