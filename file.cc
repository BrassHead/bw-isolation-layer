/* file.cc -- Binary file I/O class

Copyright (C) 1997-2013, Brian Bray

*/


#include "bw/file.h"
#include "bw/exception.h"
#include "bw/bwassert.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>


namespace bw {

/*: class BFile
Binary file object.
Represents a disk or network file.
<P>
<B>Concrete class</B>
<P>

Superclass:
	none
Subclasses:
	none

*/


const int modes[] = {
	O_RDONLY,							// ReadOnly
	O_RDWR,							// ReadWrite
	O_WRONLY | O_CREAT | O_TRUNC,	// Create
	O_WRONLY | O_CREAT | O_EXCL
};	// CreateNew

const int pmode = S_IREAD | S_IWRITE;

/*: BFile::BFile()#ctor1

	Creates unopened file object.  Call open or create to open after
	creation.
*/
BFile::BFile()
	: m_fd(-1),
	  m_atEof(false)
{}


/*: BFile::~BFile()

	Closes file (if open) and deletes object
*/
BFile::~BFile()
{
	close();
}

/*: BFile::BFile()#ctor2

	Creates open file object for reading or writing

	Throws: BFileException if unable to open for any reason
*/
BFile::BFile
(
    const char* fileName,
    FileMode fm
)
	: m_fd(-1),
	  m_atEof(false)
{
	if ( !BFile::open(fileName,fm) )
		throw BFileException( BFileException::FileNotFound );
}

/*: BFile::open()

	Opens file.

	Requires: Not currently open

	Returns:	true on success, false if file doesn't exist in ReadOnly or ReadWrite modes

	Throws: BFileException on all other errors
*/
bool
BFile::open
(
    const char* fileName,
    FileMode fm
)
{
	bwassert( m_fd<0 );

	m_fd = ::open(fileName, modes[fm], pmode);

	if( m_fd>=0 ) {
		m_atEof = false;
		return true;
	}

	// Error conditions
	if( errno != ENOENT )
		throw BFileException( BFileException::SystemError );

	return false;			// File not found
}


/*: BFile::create()

	Opens file for writing.  Creates if necessary, truncates existing files
	Requires: Not currently open
	Throws: BFileException on other errors
*/
void
BFile::create
(
    const char* fileName
)
{
	bwverify( open( fileName, Create ) );
	m_atEof = true;
}


/*: BFile::close()

	Closes open file (if any).

	Throws: BFileException on serious errors
*/
void
BFile::close()
{
	if( m_fd>=0 ) {
		if( ::close( m_fd ) ) {
			throw BFileException( BFileException::SystemError );
		}
		m_fd = -1;
	}
}


/*: BFile::read()

	Reads length bytes into buffer.

	Requires: Currently opened file

	Promises: Sets atEof() if no more data after this read

	Throws: BFileException if unable to fully fill buffer
*/
void
BFile::read
(
    void* buffer,
    long length
)
{
	bwassert( m_fd>=0 );

	int ret = ::read( m_fd, buffer, length );

	if( ret<0 )
		throw BFileException( BFileException::SystemError );

	if( ret!=length ) {
		throw BFileException( BFileException::UnexpectedEof );
		m_atEof = true;
	}
}


/*: BFile::readUpTo()

	Reads up to length bytes into buffer.

	Requires: Currently opened file
	Promises: Sets atEof() if end of file reached
	Returns: actual count of bytes read (may be 0)
	Throws: BfileException if unable to complete operation
*/
long
BFile::readUpTo
(
    void* buffer,
    long length
)
{
	bwassert( m_fd>=0 );

	int ret = ::read( m_fd, buffer, length );

	if( ret<0 )
		throw BFileException( BFileException::SystemError );
	if( ret!=length )
		m_atEof = true;

	return ret;
}


/*: BFile::write()

	Writes length bytes into buffer

	Requires: Currently opened mode ReadWrite, Create, or CreateNew
	Throws: BFileException if complete buffer is not written
*/
void
BFile::write
(
    const void* buffer,
    long length
)
{
	bwassert( m_fd>=0 );

	if( ::write(m_fd,buffer,length) != length )
		throw BFileException( BFileException::SystemError );
}

/*: BFile::commit()

	Flushes output to device.

	Requires: Currently opened mode ReadWrite, Create, or CreateNew
	Throws: BFileException on error
*/
void
BFile::commit()
{
	bwassert( m_fd>=0 );

	// There is no effective way to commit a single file in UNIX,
	// so this routine does nothing
}

/*: BFile::atEof()

	Indicates that file position is at the end.

	Requires:
		Currently opened file

	Throws: BFileException on serious errors.
*/
bool
BFile::atEof() const
{
	bwassert( m_fd>=0 );

	// Note: never throws in UNIX
	return m_atEof;
}


/*: BFile::isOpen()

	Indicates if file is open.
*/
bool
BFile::isOpen() const
{
	if( m_fd<0 )
		return false;
	else
		return true;
}


/*: BFile::seek()

	Positions read/write pointer in file.

	Requires: Currently opened file
	Returns: new file position
	Throws: BfileException if unable to comply
*/
long
BFile::seek
(
    long position,
    SeekMode sm
)
{
	bwassert( m_fd>=0 );

	long ret = lseek( m_fd, position, sm );	// Implementation note: sm matches OS modes

	if( ret<0 )
		throw BFileException( BFileException::SystemError );

	m_atEof = false;   // Assumed
	return ret;
}


/*: BFile::tell()

	Indicates current read/write position.

	Requires: Currently opened file
	Returns: file position
*/
long
BFile::tell() const
{
	bwassert( m_fd>=0 );

	long ret = ::lseek( m_fd, 0, SEEK_CUR );

	if( ret<0 )
		throw BFileException( BFileException::SystemError );

	return ret;
}

}	// namespace bw
