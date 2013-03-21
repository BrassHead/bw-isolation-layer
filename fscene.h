/* fscene.h -- Internal interface to the Scene class

Copyright (C) 1997-2013, Brian Bray

*/

// This is not a client accessible class.  It's a friend of Scene
// and it manages the private data of the scene.  It's purpose is to hold
// the things for which the Xlib include files are necessary.
//

/*
#include "bw/scene.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
*/

namespace bw {


class FScene {
public:
	static Scene* findScene( Display* pDisplay );
	static const Atoms& atomsOf( const Scene* pscene ) {
		return *(pscene->m_patoms);
	}

	static XContext	m_ctxPSCENE;
};

class Atoms {
public:
	Atoms( Display* pDisplay );

	Atom	m_atomProtocol;
	Atom	m_atomDelete;
	Atom	m_atomFocus;
	Atom	m_atomSave;
};

}   // namespace
