/* process.cc -- Internal routines for the Process class (X version)

Copyright (C) 1997-2013, Brian Bray

*/

#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/exception.h"
#include "bw/process.h"
#include "bw/figure.h"
#include "bw/scene.h"
#include "bw/context.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <clocale>
#include <cstring>
#include <stdlib.h>
#include <stdexcept>
#include <iostream>
#include "fprocess.h"
#include "event.h"
#include "ffigure.h"


// Globals
const char* BWLocale = "";

namespace bw {

// Static variable definitions
Process*        Process::m_TheProcess = 0;
int		Process::m_argc = 0;
char**	        Process::m_argv = 0;
char*		Process::m_locale = 0;
Process::state	Process::m_fState = Process::staticConstruction;

bool		FProcess::m_isExpectingErrors = false;
XErrorEvent*	FProcess::m_pxevLast = 0;
char		FProcess::m_pszHomeDir[HomeDirLen] = "";

void FProcess::setArgs(int argc, char *argv[])
{
	// Copy the arguments

	Process::m_argc = argc;
	Process::m_argv = new char*[argc];
	std::memcpy( Process::m_argv, argv, sizeof(char*) * argc );
}


// Routine to call user's main
int FProcess::main()
{
	int		rc;
	//
	// First, make sure we have a process object
	//
	Process*	pp;	// Created process or NULL (deleted later)
	if (Process::m_TheProcess) {
		// User's own process
		pp = 0;
	} else {
		// Create process
		pp = new Process;
	}

	//
	// Set the locale
	//
	Process::m_locale = setlocale(LC_ALL, "" );
	BWLocale = Process::m_locale;

	//
	// Now call the users main routine
	//
	rc = Process::m_TheProcess->main(Process::m_argc,Process::m_argv);

	//
	// Now delete the stuff we created
	//
	delete [] Process::m_argv;
	delete pp;

	return rc;
}


//
// Error handling routines
//
void FProcess::fatalError( const BException& e ) throw()
{
	// Helper routine handles default processing for fatal errors

	using std::cerr;
	using std::endl;

	// First, give the user a crack at it.
	bwassert( Process::m_TheProcess );
	Process::m_TheProcess->fatalError( e );

	// Since they return, do the default thing
	cerr << "Fatal error:" << endl;
	cerr << e.message() << endl;;
	cerr << "Terminating" << endl;

	exit(-1);
}

const char* FProcess::getHomeDir()
{
	// This routine (and the variable) are provided for backward compatibility

	if (!*m_pszHomeDir) {
		String strHomeDir =bw::getHomeDir();

		if (strHomeDir.length()>=HomeDirLen)
			throw BSystemError("Home directory too long.");
		std::strcpy( m_pszHomeDir, (const char*)strHomeDir );
	}
	return m_pszHomeDir;
}


/*: class Process

	Represents the current process.

	If a static or extern Process object is defined, it will be used.
	Otherwise, a default Process object will be created.

	Process has a number of static functions that manage the run state.

	It has one overridable function -- main -- with syntax and semantics
	similar to "C" main.  Main should create a Scene and then add
	one or more Frame windows to it.

	It should then call nextAction() repeatedly until an exit command
	is received.
*/

Process::Process()
{
	bwassert( m_TheProcess==0 );
	m_TheProcess = this;
}

Process::~Process()
{
	m_TheProcess = 0;
}


/*: routine Process::getArgX()

	Returns the command line arguments.


	Prototype: static int getArgc();
	Prototype: static char** getArgv();

*/
int Process::getArgc()
{
	return m_argc;
}

char** Process::getArgv()
{
	return m_argv;
}

/*: routine Process::getLocale()

  Returns the current operating locale of the program.  The most common
  format is <Language>_<Territory>.<Charset> or a prefix of this form,
  such as "fr_CA.iso-8859-1" or, simply, "fr".

  There are some special values such as "C" and "POSIX" that refer to the
  portable character sets for those standards.

  This is *not* the same as the LANG environment variable -- a blank LANG=
  defaults to "C", for example.
*/


/*: routine Process::getState()

	Tells run state of the current process.

	Prototype: static state getState();
	Prototype: enum state {staticConstruction, initializing, running, finishing, staticDestruction};

	Returns: One of
	<UL>
	<LI>staticConstruction
	<LI>initializing
	<LI>running
	<LI>finishing
	<LI>staticDestruction
	</UL>
	The staticConstruction and StaticDestruction states occur before and
	after "main" is executed.  "Main" goes through three phases,
	initializing, running the GUI, and finishing after the GUI elements are
	complete.
*/
Process::state Process::getState()
{
	return m_fState;
}


/*: Process::Main()

  This is the 'Main' entry point for a BW application.  It is necessary for
  BW to provide the OS 'main' both for initialization and because there
  are platform differences in the name 'main' for GUI applications.

  If no user Process is statically defined, then a default Process is created.

  The default main routine is:
  <PRE>
	{
	    String strAction;
	    Scene TheScene;

	    bwassert( Main::getMain() );
	    TheScene.addFrame( Main::getMain() );

	    while ( !strAction.startsWith("exit(") )
	        strAction = TheScene.nextAction();

	    return atoi(strAction.substring(5));
	}
  </PRE>

  Prototype: int Process::main(int argc, char* argv[])

*/
int Process::main(int, char**)
{
	String strAction;
	Scene TheScene;

	bwassert( Main::getMain() );
	TheScene.addFrame( Main::getMain() );

	while ( !strAction.startsWith("exit(") )
		strAction = TheScene.nextAction();

	return atoi(strAction.substring(5));
}


/*:	Process::fatalError()

    Called upon fatal errors in BW and the underlying GUI subsystem.
	The routine is overridable.  This routine can be called before
	Process::main (in which case Process::main is never called).

	If overriden, the routine should save whatever data it can to a save
	file and then exit.  The BException passed as a parameter indicates
	the nature of the fatal error.  This routine cannot throw an exception.

	If the routine returns, a message is output to stderr and exit()
	is called.

	The default routine in the base class just returns.

	Prototype:  void fatalError(const BException& e) throw()
*/

//
// Helper routine
//
// TODO: Documentation
//
const char* getHomeDir()
{
	static const int HomeDirLen = 200;
	static char pszHomeDir[HomeDirLen] = "";
	if (!*pszHomeDir) {
		//
		// Find the users Home Directory
		//
		String strHomeDir = "/";	// absolute end default

		char *psz = getenv( "HOME" );
		if (psz) {
			strHomeDir = psz;
		} else {
			psz = getenv( "USER" );
			if (psz) {
				strHomeDir = "/home/";
				strHomeDir.append( psz );
			}
		}
		int len = strHomeDir.length()+1;
		if (len>HomeDirLen)
			throw BSystemError("Home directory too long.");
		std::strcpy( pszHomeDir, strHomeDir.c_str() );
	}
	return pszHomeDir;
}


}	//namespace bw
