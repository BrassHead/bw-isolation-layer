/* directory.cc -- implementation of directory search routines

Copyright (C) 1997-2013, Brian Bray

*/

#include "bw/exception.h"
#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/filename.h"
#include "bw/directory.h"

#include <sys/types.h>
#include <dirent.h>
#include <fnmatch.h>


namespace bw {


/*: class Directory

  A directory object acts like a forward iterator over a list of FileNames.
  The -> and * operators can be used to access the filenames contained in the
  directory.  In addition, some additional information is available about the
  contained files, such as their type.
*/

/*: Directory::Directory()

  Constructor.  Parameters specify the directory to be searched with an
  optional file pattern within the directory.  If no files are found (eg:
  nothing matches pattern, base directory doesn't exist, syntax error in
  filename), then an empty list is returned.

  Prototype: Directory(const FileName& fnDir, const FileName& fnPattern="*")
*/
Directory::Directory(const FileName& fnDir, const FileName& fnPattern)
	:   m_fnDir( fnDir ),
	    m_fnPattern( fnPattern ),
	    m_atEof( false )
{
	FileName fnFull = fnDir;
	fnFull.append(fnPattern);

	m_pvdir = opendir( (const char*)fnDir );

	if (m_pvdir)
		step();
	else
		m_atEof = true;
}

/* Directory::~Directory()

   Destructor.
*/
Directory::~Directory()
{
	if (m_pvdir)
		closedir((DIR*)m_pvdir); // Note: errors ignored (nothing can be done)
}

/* Directory::step()

   Internal routing advances to next matching filename.
*/
void Directory::step()
{
	if (!m_atEof) {
		m_pde = readdir((DIR*)m_pvdir);

		while ( m_pde && fnmatch( m_fnPattern, m_pde->d_name, 0 ) )
			m_pde = readdir((DIR*)m_pvdir);

		if (m_pde) {
			m_fnResult = m_fnDir;
			m_fnResult.append( m_pde->d_name );
		} else {
			m_fnResult.empty();
			m_atEof = true;
		}
	}
}

/*: Directory::operator++()

  Step to next file in search.

  Prototype: dir++;
  Prototype: ++dir;
*/
Directory& Directory::operator++()
{
	step();
	return *this;
}

void Directory::operator++(int)
{
	step();
}

/*: Directory::operator->()

  Access to filename of current search result.

  Prototype: FileName* dir->foo()
  Prototype: FileName& (*dir).foo()
*/
FileName* Directory::operator->()
{
	if (!m_atEof)
		return &m_fnResult;
	else
		return 0;
}

FileName& Directory::operator*()
{
	return m_fnResult;
}

/*: Directory::atEof()

  True if directory iterator points past last found file.  Note that
  the FileName is only valid if !atEof().

*/
bool Directory::atEof() const
{
	return m_atEof;
}

/*: Directory::rewind()

  Restarts search from the begining.
*/
void Directory::rewind()
{
	rewinddir( (DIR*)m_pvdir );
	m_atEof = false;
	step();
}

/*: Directory::getType()

  Returns the file type of the currect search result.
  The result is described under FileName::getFileType().
  FileName::NoAccess is returned on errors or atEof().  This routine is
  provided because it is faster than getFileType() on some platforms (since
  the directory holds this information).
*/
FileName::Type Directory::getType() const
{
	return m_fnResult.getFileType();  // Not faster on POSIX
}

}   // namespace

