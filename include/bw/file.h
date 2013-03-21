/* file.h -- unbuffered binary file object

Copyright (C) 1996-2013, Brian Bray

*/

/* Needs:
#include "bw/exception.h"
*/

namespace bw {

class BFile
// Purpose: Binary file object
// Note: Use for binary files only, use C++ iostream classes for text
// TODO: Should be bfile.h (or BinaryFile)
{
public:
	enum FileMode {
		ReadOnly=0,	// Existing file
		ReadWrite=1,	// Existing file
		Create=2,	// Creates new file if necessary or truncates existing file
		CreateNew=3	// Throws if file already exists
	};

	enum SeekMode {
		fromStart=0,
		fromCurrent=1,
		fromEnd=2
	};

	BFile();
	// Purpose: Creates unopened file object, call open or create to open

	~BFile();
	// Purpose: Closes file (if open) and deletes object

	BFile( const char* fileName, FileMode fm=BFile::ReadOnly );
	// Purpose: Creates open file object for reading or writing
	// throw( BFileException ) if unable to open for any reason

	bool open( const char* fileName, FileMode fm=BFile::ReadOnly );
	// Purpose: opens file
	// Requires: Not currently open
	// Returns:	true on success, false if file doesn't exist in ReadOnly or ReadWrite modes
	// throw( BFileException ) on all other errors

	void create( const char* fileName );
	// Purpose: Opens file for writing, creates if necessary, truncates existing files
	// Requires: Not currently open
	// throw( BFileException ) on other errors

	void close();
	// Purpose: Closes open file (if any)
	// throw( BFileException ) on serious errors

	void read( void* buffer, long length );
	// Purpose: Reads length bytes into buffer
	// Requires: Currently opened file
	// Promises: Sets atEof() if no more data after this read
	// throw( BFileException ) if unable to fully fill buffer

	long readUpTo( void* buffer, long length );
	// Purpose: Reads up to length bytes into buffer
	// Requires: Currently opened file
	// Promises: Sets atEof() if end of file reached
	// Returns: actual count of bytes read (may be 0)
	// throw( BfileException ) if unable to complete operation

	void write( const void* buffer, long length );
	// Purpose: Writes length bytes into buffer
	// Requires: Currently opened mode ReadWrite, Create, or CreateNew
	// throw( BFileException ) if complete buffer is not written

	void commit();
	// Purpose: Flushes output to device
	// Requires: Currently opened mode ReadWrite, Create, or CreateNew
	// throw( BFileException ) on error

	bool atEof() const;
	// Purpose: indicates that file position is at the end
	// Requires: Currently opened file

	bool isOpen() const;
	// Purpose: indicates if file is open

	long seek( long position, SeekMode sm=BFile::fromStart );
	// Purpose: positions read/write pointer in file
	// Requires: Currently opened file
	// Returns: new file position
	// throw( BfileException ) if unable to comply

	long tell() const;
	// Purpose: indicates current read/write position
	// Requires: Currently opened file
	// Returns: file position

private:
	// BFiles cannot be copied or assigned
	BFile( const BFile& ) {}
	BFile& operator=( const BFile& ) {
		return *this;
	}

private:
	int	m_fd;
	bool    m_atEof;
};

}	// namespace bw


