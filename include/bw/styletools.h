/* styletools.h -- Interface to BW Style drawing tools

Copyright (C) 1997-2013, Brian Bray

*/

/* Needs:
#include "bw/tools.h"
*/

namespace bw {

class Canvas;
class Brush;
class Pen;

class Bevel {
public:
	Bevel(Canvas& cvs);
	~Bevel();

	static int borderWidth() {
		return 3;
	}

	void drawRaised( int x, int y, int w, int h );
	void drawLowered( int x, int y, int w, int h );

private:
	Brush	m_brFace;
	Pen		m_penWhite;
	Pen		m_penBlack;
	Pen		m_penLight;
	Pen		m_penDark;
	int		m_width;
	int		m_height;
};


}   // namespace
