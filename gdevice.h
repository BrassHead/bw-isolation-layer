/* gdevice.h -- Represents a graphic device

Copyright (C) 1998-2013, Brian Bray

*/


//#include "bw/font.h"

namespace bw {

// The GDevice class keeps track of device specific resources such as
// fonts, colours, and Palettes.

// TODO: class XGDevice : public GDevice
class GDevice {
public:
	GDevice( Display* pDisplay );
	~GDevice();

	XFontStruct* getFontStruct( FontIndex findx );

//private:
	Display*	m_pDisplay;
	//Vector<XFontStruct*>	m_vpfs;  TODO: remove Hack
	XFontStruct*	m_vpfs[2];
};


}   // namespace
