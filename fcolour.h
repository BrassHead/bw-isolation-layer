/* fcolour.h -- Internal routines related to colours

Copyright (C) 1997-2013, Brian Bray

*/



//#include <bw/colour.h>

namespace bw {

class FColour {
public:
	static void setGCV( const Colour& clr, unsigned long& vm, XGCValues& xgcv );
	static void setGCVBack( const Colour& clr, unsigned long& vm, XGCValues& xgcv );

};

}   // namespace
