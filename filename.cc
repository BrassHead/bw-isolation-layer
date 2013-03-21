/* filename.cc -- FileName class (expert in file names)

Copyright (C) 1997-2013, Brian Bray

*/

#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/filename.h"
#include "bw/exception.h"

#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>

namespace bw {

template <class T> inline T max(const T& a, const T& b)
{
	return a>b ? a : b;
}

/*: Class FileName

  Represents a filename in the host OS.  These are created from a
  string and allow access to the elements of the name.

  Win32 FileNames are divided into four parts:  Root, Directory, BaseName, and
  Extension.  Any or all of these parts can be null.  Some examples:
  <PRE>

  readme.txt          "", "", "readme", ".txt"       Relative (win32), Absolute (POSIX)
  &#47;readme.txt         "/", "", "readme", ".txt"      Relative
  d&#58;\readme.txt       "d:\", "", "readme", ".txt"    Absolute
  d&#58;readme.txt        "d:", "", "readme", ".txt"     Relative
  d&#58;\src/readme       "d:\", "src/", "readme", ""    Absolute
  \\host\c\src\rm     "\\", "host\c\src\", "rm", ""  Absolute
  D&#58;\                 "D:\", "", "", ""              Absolute
  D&#58;\src.d            "D:\", "", "src", ".d"         Absolute
  D&#58;\src.d\           "D:\", "src.d\", "", ""        Absolute
  D&#58;\src.d\*.*        "D:\", "src.d\", "*", ".*"     Absolute, Pattern
  D&#58;\src\..           "D:\", "src\", "..", ""        Absolute
  D&#58;\src\.            "D:\", "src\", ".", ""         Absolute
  D&#58;\src\.foo         "D:\", "src\", "", ".foo"      Absolute
  </PRE>

  On UNIX systems, the root is always either "/" or "".
*/

/*: routine FileName::FileName

  Constructor.

  Prototype: FileName::FileName(const char* pszFileName="");

*/
FileName::FileName(const char* pszFileName)
	: m_sFileName(pszFileName)
{
	parse();
}


/*: routine FileName::setName

  Sets the full name.
*/
void FileName::setName(const char* pszFileName)
{
	m_sFileName = pszFileName;
	parse();
}

void FileName::parse()
{
	// Break filename up into components
	// Root:
	m_nRoot = 0;
	while (m_sFileName[(int)m_nRoot]=='/' )
		++m_nRoot;

	// Directory
	m_nDir = m_sFileName.lastIndexOf('/') + 1;
	if (m_nDir>m_nRoot)
		m_nDir -= m_nRoot;
	else
		m_nDir = 0;

	// BaseName and Extension
	String sName = m_sFileName.substring(m_nRoot+m_nDir);
	if (sName==".") {
		m_nBase = 1;
		m_nExt = 0;
	} else if (sName=="..") {
		m_nBase = 2;
		m_nExt = 0;
	} else {
		m_nBase = sName.lastIndexOf('.');
		if (m_nBase<0) {
			m_nBase = sName.length();
			m_nExt = 0;
		} else
			m_nExt = sName.length()-m_nBase;
	}
	bwassert( m_nRoot+m_nDir+m_nBase+m_nExt == m_sFileName.length() );
}

/*: routine FileName::isAbsolute()

  Returns true if the Filename is an absolute path.  I.E. A path that
  is not relative to the current drive or directory
*/
bool FileName::isAbsolute() const
{
	return m_nRoot>0;
}

/*: routine FileName::isPattern()

  Returns true if the filename is a pattern.  I.E. The BaseName or Extension
  contains "*" or "?" characters.
*/
bool FileName::isPattern() const
{
	String sName = m_sFileName.substring(m_nRoot+m_nDir);
	return sName.indexOf('*')>=0 || sName.indexOf('?')>=0;
}

/*: routine FileName::empty()

  Sets the filename to the null string.
*/
void FileName::empty()
{
	m_sFileName = "";
	m_nRoot = m_nDir = m_nBase = m_nExt = 0;
}


/*: FileName::root()

  Returns root component of filename.
*/
String FileName::root() const
{
	return m_sFileName.substring(0,m_nRoot);
}

/*: FileName::directory()

  Returns directory component of filename.
*/
String FileName::directory() const
{
	return m_sFileName.substring(m_nRoot,m_nRoot+m_nDir);
}

/*: FileName::baseName()

  Returns BaseName component of filename.
*/
String FileName::baseName() const
{
	return m_sFileName.substring(m_nRoot+m_nDir,m_nRoot+m_nDir+m_nBase);
}

/*: FileName::extension()

  Returns extension component of filename.
*/
String FileName::extension() const
{
	return m_sFileName.substring(m_nRoot+m_nDir+m_nBase);
}

/*: FileName::relativeName()

  Returns final component of filename. This is baseName()+extension()
*/
String FileName::relativeName() const
{
	return m_sFileName.substring(m_nRoot+m_nDir);
}

/*: FileName::fullName()

  Returns entire filename.
*/
String FileName::fullName() const
{
	return m_sFileName;
}

/*: FileName::makeAbsolute()

  Converts filename to an absolute filename based on the current
  directory.  Throws BFileException on error.
*/
void FileName::makeAbsolute()
{
	if (isAbsolute())
		return;

	char pszCurDir[PATH_MAX+1];

	if (getcwd( pszCurDir, PATH_MAX+1 )) {
		m_sFileName = String(pszCurDir) + "/" + m_sFileName;
		parse();
	} else
		throw BFileException( BFileException::FileNotFound );
}

/*: FileName::append()

  Appends one filename to another.  The appended filename must be
  a relative path without a drive or initial "/".
*/
void FileName::append(const FileName& fn)
{
	bwassert( !fn.isAbsolute() );
	bwassert( fn.m_nRoot==0 );

	char chLast = m_sFileName[m_sFileName.length()-1];
	if (chLast!='/' )
		m_sFileName.append('/');
	m_sFileName.append(fn.m_sFileName);
	parse();
}

/*: FileName::currentDirectory()

  Returns a FileName object initialized to the current directory
*/
FileName FileName::currentDirectory()
{
	char pszPath[PATH_MAX+1];

	if (getcwd( pszPath, PATH_MAX+1 ))
		return FileName(pszPath);
	else
		throw BFileException( BFileException::FileNotFound );
}

/*: FileName::operator(char*)

  Conversion to character string.  Allows FileName objects to be passed
  to any routine expecting a filename string.
*/
FileName::operator const char*() const
{
	return m_sFileName;
}

/*: FileName::getFileType()

  Checks to see if the named file exists and returns its type.
*/
FileName::Type FileName::getFileType() const
{
	struct stat bStat;

	if (stat(m_sFileName,&bStat)==0) {
		// File found, return type
		if (S_ISREG(bStat.st_mode))
			return RegularFile;
		if (S_ISDIR(bStat.st_mode))
			return Directory;
		if (S_ISFIFO(bStat.st_mode))
			return Pipe;
		if (S_ISCHR(bStat.st_mode) || S_ISBLK(bStat.st_mode))
			return Device;
	}
	return NoAccess;		// No access, doesn't exist, or strange type
}

}   // namespace
