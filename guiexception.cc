/* guiexception.cc -- Implementation of the win32 GUI Exception

Copyright (C) 1997-2013, Brian Bray

*/

#include "bw/exception.h"
#include "bw/bwassert.h"

#include <cstring>
using std::strcpy;
using std::strcat;


namespace bw {

/* BGUIException::BGUIException

   Constuctor.  Parameters are the error type and a string whose interpretation
   depends on the type.
*/
BGUIException::BGUIException( BGUIException::GUIErrorCode errnum, const char* psz)
	:  m_errorCode( errnum )
{
	bwassert( errnum>0 || errnum<=4 );

	switch (errnum) {
	case SystemError:
		bwassert( psz );
		bwassert( strlen(psz)<msgBufferSize-sizeof("Internal GUI system error: ") );
		strcpy( m_messageBuffer, "Internal GUI system error: " );
		strcat( m_messageBuffer, psz );
		break;

	case NoMainWindow:
		strcpy( m_messageBuffer, "No main window defined." );
		break;

	case ConnectionLost:
		bwassert( psz );
		bwassert( strlen(psz)<msgBufferSize-sizeof("Connection to display  lost.") );
		strcpy( m_messageBuffer, "Connection to display " );
		strcat( m_messageBuffer, psz );
		strcat( m_messageBuffer, " lost." );
		break;

	case NoDisplay:
		bwassert( psz );
		bwassert( strlen(psz)<msgBufferSize-sizeof("Cannot open display .") );
		strcpy( m_messageBuffer, "Cannot open display " );
		strcat( m_messageBuffer, psz );
		strcat( m_messageBuffer, "." );
		break;

	default:
		bwassert( false );	// Should not occurr
		strcpy( m_messageBuffer, "Null GUI Error" );
	}

}

BGUIException::~BGUIException()
{}

const char*
BGUIException::message() const
{
	return m_messageBuffer;
}


const char*
BGUIException::name() const
{
	return "GUI";
}


BException::Severity
BGUIException::severity() const
{
	return Fatal;
}

}	//namespace bw


