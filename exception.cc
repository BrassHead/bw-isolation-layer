/* exception.cc -- Exception handing code

Copyright 1997-2013 Brian Bray

*/

#include <new>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "bw/exception.h"
#include "bw/bwassert.h"

namespace bw {


// Setup ::new to throw exception

void BNewHandler()
{
	throw BMemoryException();
}

class BInit {
public:
	BInit();
};

BInit::BInit()
{
	// Initialization code for B* routines
	// This code is executed before main by the static definition below.

	std::set_new_handler( BNewHandler );
}

static BInit TheApplication;

// Definition of predefined exceptions
// See header file of BException for description of routines
BMemoryException::BMemoryException()
{}


BMemoryException::~BMemoryException()
{}

const char*
BMemoryException::message() const
{
	return "Out of memory";
}


const char*
BMemoryException::name() const
{
	return "memory";
}


BException::Severity
BMemoryException::severity() const
{
	return Fatal;
}


BFileException::BFileException
(
    FileErrorCode errnum
)
	: errorCode_( errnum ),
	  errno_( errno )
{
	static const char* msgFileNo[] = {
		"No error",
		"Unexpected end of file",
		"System error",
		"File not found"
	};
	static const char* msgFileSys = "File system error: ";
	static const char* msgFile = "File error: ";

	bwassert( errnum>0 && errnum<4 );

	if( errnum==SystemError ) {
		strcpy( messageBuffer_, msgFileSys );
		strerror_r( errno_, messageBuffer_+sizeof(msgFileSys), msgBufferSize-sizeof(msgFileSys) );
	} else {
		strcpy( messageBuffer_, msgFile );
		strcpy( messageBuffer_+sizeof(msgFile), msgFileNo[errnum] );
	}

}


BFileException::~BFileException()
{}

const char*
BFileException::message() const
{
	return messageBuffer_;
}


const char*
BFileException::name() const
{
	return "File";
}


BException::Severity
BFileException::severity() const
{
	return Fatal;
}

// BFormatException
BFormatException::BFormatException
(
    const char* msg
)
{
	strncpy( messageBuffer_, msg, msgBufferSize );
	messageBuffer_[msgBufferSize] = '\0';	// Terminate
}


BFormatException::~BFormatException()
{}

const char*
BFormatException::message() const
{
	return messageBuffer_;
}


const char*
BFormatException::name() const
{
	return "memory";
}


BException::Severity
BFormatException::severity() const
{
	return Error;
}

BSystemError::BSystemError( const char* psz )
{
	bwassert( strlen(psz)<msgBufferSize-strlen("Fatal internal error: ") );
	strcpy( m_messageBuffer, "Fatal internal error: " );
	strcat( m_messageBuffer, psz );
}

BSystemError::~BSystemError()
{}

const char* BSystemError::message() const
{
	return m_messageBuffer;
}

const char* BSystemError::name() const
{
	return "System";
}

BException::Severity BSystemError::severity() const
{
	return Fatal;
}

}	// namespace bw
