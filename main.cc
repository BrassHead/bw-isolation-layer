/* main.cc -- main for Xwindows BW

Copyright © 1997-2013, Brian Bray


*/

// Hack for g++ defect
//#define __PUT_STATIC_DATA_MEMBERS_HERE 1

// Compile option

//#define NOTRACE
#include "bw/trace.h"
#include "bw/bwassert.h"
#include "bw/exception.h"
#include "bw/process.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <iostream>
#include <stdexcept>
#include "fprocess.h"

using bw::BException;
using bw::Process;
using bw::FProcess;

using std::endl;
using std::cerr;


/*: project BW

	Isolation layer for writing long term GUI code.
*/


int main
(
    int argc,
    char *argv[]
)
{


	int rc;

	try {
		FProcess::setState(Process::initializing);

		trace << "Starting Up" << endl;

		// Squirrel away the arguments
		FProcess::setArgs(argc, argv);

		// Call client level main routine
		rc = FProcess::main();

		// Exit the Process
		FProcess::setState(Process::staticDestruction);
	}

	// These are the "last chance" exception handlers.  If one of these
	// is reached it indicates a defect in the BW system or in the user's
	// main.  Exceptions missed by the users code are trapped within
	// Process::enterGUI().

	catch( const BException& e ) {
		cerr << "Unhandled BW exception caught by last chance handler:" << endl;
		cerr << e.message() << endl;
		cerr << "Program will be terminated." << endl;
		rc = -1;
	} catch( const std::exception& e ) {
		cerr << "Unhandled C++ runtime exception caught by last chance handler:" << endl;
		cerr << e.what() << endl;
		cerr << "Program will be terminated." << endl;
		rc = -2;
	} catch( ... ) {
		cerr << "Unhandled exception of unknown type caught by last chance handler." << endl;
		cerr << "Program will be terminated." << endl;
		rc = -3;
	}

	return rc;
}
