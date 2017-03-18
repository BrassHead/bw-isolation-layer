/* bwassert.cc -- assert and friends

Copyright (C) 1996-2013, Brian Bray

*/

// Version for console applications

// Note: NOTRACE must always be set in this file
#define NOTRACE

#include <cstring>
#include <stdlib.h>
#include <iostream>
#include "bw/bwassert.h"
#include "bw/trace.h"
#include "bw/exception.h"

//  Definitions of trace and tracein
BTrace trace;
BTrace tracein;


namespace bw {

using std::cin;
using std::cout;
using std::endl;
using std::flush;

/*: class BAssert

This class holds static routines accessed by assertion macros.
These routines should not be called directly, but accessed from the
macros provided.  These macros expand into appropriate calls
unless BWASSERTDISCARD is defined.

*/


/*: BAssert::bwassert()

Asserts that the expression X is true.  If not, it throws an exception
containing the file name and line number where the error
occurred along with the expression that failed. If BWASSERTDISCARD is
#defined, then the expression is not evaluated and no exception is thrown.

This macro is intended for additional checking for unexpected errors in
inner loop (frequent) operations too inefficient to verify in production. Non
critical applications should never #define BWASSERTDISCARD. Code not
in high frequency code segments should throw regular exceptions, such as
std::invalid_argument or std::domain_error, etc.

	prototype: bwassert( X );
*/


/*: BAssert::bwverify()

Unit test assertion.

Verifies that the expression X is true.  If not, it displays a
message on std::cerr containing the file name and line number of the bwverify
along with the expression. Execution then continues.

If BWASSERTDISCARD is #defined, X will still be tested and this type of
assertion will throw an exception if X is false.

This macro is intended for use in test harnesses and unit tests, and should
not be used in application code.

	prototype: bwverify( X );
*/


BDebugException::BDebugException( const char* msg )
{
	strncpy( m_messageBuffer, msg, msgBufferSize );
	m_messageBuffer[msgBufferSize-1] = '\0';	// Terminate
}

BDebugException::~BDebugException()
{}

const char* BDebugException::message() const
{
	return m_messageBuffer;
}

const char* BDebugException::name() const
{
	return "assert";
}

BException::Severity BDebugException::severity() const
{
	return Fatal;
}


/* internal routine BAssert::doassert()

   Show assert msg or throw BDebugException.
*/
void BAssert::doassert( const char* msg, const char* fileName,
                        int lineNum, bool isFatal )
{
	std::string s = std::string("Internal error at ") + fileName + ": " + std::to_string(lineNum) + " Failed assertion \"" + msg + "\"";

	if ( isFatal ) {
		throw BDebugException( s.c_str() );
	} else {
		std::cerr << s << endl;
	}

	return;
}


}	// namespace bw

