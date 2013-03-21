/* bgc.h -- Convenience routines for GCs

Copyright 1997-2013, Brian Bray

*/

//#include <X11/Xlib.h>
//#include <X11/Xutil.h>

namespace bw {

class Canvas;

class BGC {
public:
	static BGC* getPBGC( const Canvas& cvs, unsigned long vm, XGCValues& xgcv );
	static void changeValues( BGC*& pbgc, unsigned long vm, XGCValues& xgcv );

	BGC( Display* pDisplay, Drawable drw, unsigned long vm, XGCValues& xgcv );
	~BGC();

	void release();

	int		m_nRefs;
	Display*	m_pDisplay;
	Drawable	m_drw;
	GC		m_gc;
};



}   // namespace bw
