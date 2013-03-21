/* fcanvas.h -- Internal routines related to Canvas

Copyright (C) 1997-2013 Brian Bray

*/


//#include <bw/canvas.h>
//#include "ffigure.h"

namespace bw {

class FCanvas {
public:
	static void setGCV( const Canvas& cvs, unsigned long& vm, XGCValues& xgcv );
	static GDevice* getDevice( const Canvas& cvs ) {
		return cvs.m_pfi->m_pgdev;
	}
};

}   // namespace
