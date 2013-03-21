/* fprocess.h -- Internal interface to the Process class

Copyright (C) 1997-2013, Brian Bray

*/

//
// This is not a client accessible class.  It's a friend of Process
// and it manages the private data of Process.
//

/*
#include "bw/bwassert.h"
#include "bw/exception.h"
#include "bw/process.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
*/

namespace bw {

class FProcess {
public:
	static void setArgs(int argc, char* argv[]);
	static void setState(Process::state s) {
		Process::m_fState = s;
	}
	static Process* getTheProcess() {
		return Process::m_TheProcess;
	}
	static int main();
	static void fatalError( const BException& e ) throw();
	static const char* getHomeDir();

	static int		m_rc;
	static bool		m_isExpectingErrors;
	static XErrorEvent*	m_pxevLast;
	enum {HomeDirLen=200};
	static char		m_pszHomeDir[HomeDirLen];
};

class ErrorSection {
public:
	ErrorSection() {
		// TODO: critical section (since global variables are used)
		bwassert( !FProcess::m_isExpectingErrors );
		bwassert( !FProcess::m_pxevLast );
		FProcess::m_isExpectingErrors = true;
	}
	~ErrorSection() {
		bwassert( FProcess::m_isExpectingErrors );
		FProcess::m_isExpectingErrors = false;
		if (FProcess::m_pxevLast)
			FProcess::fatalError( BGUIException(BGUIException::SystemError) );
	}
	void cancelError() {
		FProcess::m_pxevLast = 0;
	}
};

extern "C" int XIOErrorTrap( Display* pDisplay );
extern "C" int XErrorEventTrap( Display* pDisplay, XErrorEvent* pxev );


} //namespace bw
