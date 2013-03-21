/* string.cc -- basic string functions

Copyright (C) 1997-2013, Brian Bray

*/

//
// There is probably a clever way to reduce the redundancy of the code below,
// but this was hard to do without either adding extra call overhead or
// requiring that <string.h> be included in the header file and thus into
// the users program.

#include "bw/bwassert.h"
#include "bw/string.h"

#include <cctype>
#include <cstring>

namespace bw {

// Compilation time options

// When Strings are shortened, they are re-allocated to a smaller size if
// more than maxSpill characters are allocated, but unused.
const int maxSpill=128;

// Allocations for string space are always in multiples of quantaAlloc.
// quantaAlloc must be a power of 2.
const int quantaAlloc=16;




/////////////////////////////////////////////////////////////////////////
/*: class String

  Represents a null terminated string.

  This implementation is uses pointers to null terminated character arrays.
  Reference counts are not used.  Characters are "char"s, not "wchar_t"s in
  the "C" locale.
*/


/////////////////////////////////////////////////////////////////////////
/*: routine String::String

  Constructors.   Parameters specify an initial string in one of a variety
  of formats or give an integer for the initial capacity.

  Prototype: String( const int length=0 )
  Prototype: String( const String& str );
  Prototype: String( const char* psz );
  Prototype: String( const char* ps, const int len );
*/
String::String( const int len )
{
	bwassert( len>=0 );
	bwassert( len<65536 );	// Reasonableness, not strictly required

	m_lenMax = (len+quantaAlloc) & -quantaAlloc;  // Adds one and rounds up
	bwassert( m_lenMax>len );
	bwassert( (m_lenMax&15) == 0 );
	m_pszString = new char[m_lenMax];
	*m_pszString = '\0';
}


String::String( const String& str )
{
	int len = str.length();
	bwassert( len>=0 );
	bwassert( len<65536 );	// Reasonableness, not strictly required

	m_lenMax = (len+quantaAlloc) & -quantaAlloc;  // Adds one and rounds up
	m_pszString = new char[m_lenMax];
	::strcpy( m_pszString, str.m_pszString );
}


String::String( const char* psz)
{
	bwassert( psz );
	int len = ::strlen( psz );
	bwassert( len>=0 );
	bwassert( len<65536 ); // Reasonableness, not strictly required

	m_lenMax = (len+quantaAlloc) & -quantaAlloc;  // Adds one and rounds up

	m_pszString = new char[m_lenMax];
	::strcpy( m_pszString, psz );
}


String::String( const char* ps, const int len )
{
	bwassert( len>=0 );
	bwassert( len<65536 );       // Reasonableness

	m_lenMax = (len+quantaAlloc) & -quantaAlloc;  // Adds one and round up
	m_pszString = new char[m_lenMax];
	::memcpy( m_pszString, ps, len );
	m_pszString[len] = '\0';
}

/////////////////////////////////////////////////////////////////////////
/*: routine String::~String

    Destructor
*/
String::~String()
{
	bwassert( m_pszString );
	delete [] m_pszString;
#ifdef _DEBUG
	m_pszString = 0;		// Will cause rogue pointers to assert
#endif
}

/////////////////////////////////////////////////////////////////////////
/*: routine String::operator+

    String concatenation operators.

    Prototype: char* + String
    Prototype: String + String
    Prototype: String + char*
*/
String operator+( const char* psz, const String& str )
{
	String strRet( ::strlen(psz)+str.length() );
	::strcpy( strRet.m_pszString, psz );
	::strcat( strRet.m_pszString, str.m_pszString );
	return strRet;
}


String String::operator+( const String& str ) const
{
	bwassert( m_pszString );
	bwassert( str.m_pszString );
	String strRet( length()+str.length() );
	::strcpy( strRet.m_pszString, m_pszString );
	::strcat( strRet.m_pszString, str.m_pszString );
	return strRet;
}


String String::operator+( const char* psz ) const
{
	bwassert( m_pszString );
	String strRet( length()+::strlen(psz) );
	::strcpy( strRet.m_pszString, m_pszString );
	::strcat( strRet.m_pszString, psz );
	return strRet;
}


/////////////////////////////////////////////////////////////////////////
/*: routine String::operator=

    Assignment operators.

    Prototype: (String) = (String);
    Prototype: (String) = (char *);
    Prototype: (String) = (char);
*/
String& String::operator=( const char* psz)
{
	bwassert( m_pszString );
	bwassert( psz );
	if (m_pszString != psz) {	// Handle a=a
		int len = ::strlen(psz);
		bwassert( len>=0 );
		bwassert( len<65536 );	// Reasonableness, not strictly required

		if (len>=m_lenMax || m_lenMax>(len+maxSpill)) {
			delete [] m_pszString;
			m_lenMax = (len+quantaAlloc) & -quantaAlloc; // Add 1 and round up
			m_pszString = new char[m_lenMax];
		}
		::strcpy( m_pszString, psz );
	}
	return *this;
}


String& String::operator=( char ch )
{
	bwassert( m_pszString );

	if (m_lenMax>(maxSpill+1)) {
		// Need to shorten
		delete [] m_pszString;
		m_lenMax = quantaAlloc;
		m_pszString = new char[quantaAlloc];
	}
	m_pszString[0] = ch;
	m_pszString[1] = '\0';

	return *this;
}

/////////////////////////////////////////////////////////////////////////
/*: routine String::operator(char*)

    char* conversion

    Prototype: operator const char*() const inline;
*/


/////////////////////////////////////////////////////////////////////////
/*: routine String::length()

    Return length to terminator
*/
int String::length() const
{
	bwassert( m_pszString );
	return ::strlen( m_pszString );
}


/////////////////////////////////////////////////////////////////////////
/*: routine String::append

    Append to string.

    Prototype: void append( const String& str) inline
    Prototype: void append( const char* psz )
    Prototype: void append( char ch )
*/

void String::append( const char* psz)
{
	bwassert( m_pszString );
	ensureCapacity( length()+::strlen( psz ) );
	::strcat( m_pszString, psz );
}

void String::append( char ch )
{
	bwassert( m_pszString );

	int len = length();
	ensureCapacity( len+1 );
	m_pszString[len] = ch;
	m_pszString[len+1] = '\0';
}


/////////////////////////////////////////////////////////////////////////
/*: routine String::ensureCapacity()

    Make sure buffer is big enough.
*/
void String::ensureCapacity( int len )
{
	bwassert( m_pszString );
	bwassert( len>=0 );
	bwassert( len<65536 );	// Reasonableness, not strictly required
	if (len>=m_lenMax) {
		m_lenMax = (len+quantaAlloc) & -quantaAlloc;	// Add 1 and round up
		char* pszNew = new char[m_lenMax];
		::strcpy( pszNew, m_pszString );
		delete [] m_pszString;
		m_pszString = pszNew;
	}
}


/////////////////////////////////////////////////////////////////////////
/*: routine String::capacity()

    Returns capacity of string (not including terminator)

    Prototype: 	int capacity() const inline;
*/


/////////////////////////////////////////////////////////////////////////
/*: routine String::operator==

  Comparison operators for strings.  These are case sensitive and not
  localized.

  There are a complete set of comparison operators for String's and
  char*'s.  Operators are: ==, !=, &lt;=, >=, &lt;, and >.

  Prototype: (String) <op> (String)
  Prototype: (String) <op> (char*)
  Prototype: (char*) <op> (String)
*/


/////////////////////////////////////////////////////////////////////////
/*: routine String::compareTo

  Case sensitive comparison

  Prototype: int compareTo( const String& str ) const
  Prototype: int compareTo( const char* psz ) const

  Returns: 0 if the strings are equal, >0 if the this string is
  greater than the given string, &lt;0 if this string is less than
  the given string.
*/
int String::compareTo( const char* psz ) const
{
	bwassert( m_pszString );
	bwassert( psz );
	return ::strcmp(m_pszString, psz);
}


/////////////////////////////////////////////////////////////////////////
/*: routine String::equals()

  Case sensitive comparison for equality

  Prototype: bool equals( const char* psz ) const
  Prototype: bool equals( const String& str ) const
*/


/////////////////////////////////////////////////////////////////////////
/*: routine String::equalsIgnoreCase

  Case insensitive comparison for equality.

  Prototype: bool equalsIgnoreCase( const char* psz ) const
  Prototype: bool equalsIgnoreCase( const String& str ) const
*/
bool String::equalsIgnoreCase( const char* psz ) const
{
	bwassert( m_pszString );
	bwassert( psz );
	return strcasecmp(m_pszString, psz)==0;
}


/////////////////////////////////////////////////////////////////////////
/*: routine String::indexOf()

  Finds first occurrance of character or string.

  Prototype: int indexOf( char ch ) const
  Prototype: int indexOf( const char* psz ) const
  Prototype: int indexOf( const String& str ) const

  Returns:	Index of first occurrance or -1 if not found
*/
int String::indexOf( char ch ) const
{
	bwassert( m_pszString );
	char *psz = ::strchr(m_pszString, ch);
	if (psz)
		return psz - m_pszString;
	else
		return -1;
}

int String::indexOf( const char* psz ) const
{
	bwassert( m_pszString );
	bwassert( psz );
	char* pszT = ::strstr(m_pszString, psz);
	if (pszT)
		return pszT - m_pszString;
	else
		return -1;
}


/////////////////////////////////////////////////////////////////////////
/*: routine String::lastIndexOf

  Finds last occurrance of character or string.

  Prototype: int lastIndexOf( char ch ) const
  Prototype: int lastIndexOf( const char* psz ) const
  Prototype: int lastIndexOf( const String& str ) const

  Returns:	Index of last occurrance or -1 if not found
*/
int String::lastIndexOf( char ch ) const
{
	bwassert( m_pszString );
	char *psz = ::strrchr(m_pszString, ch);
	if (psz)
		return psz - m_pszString;
	else
		return -1;
}

int String::lastIndexOf( const char* psz ) const
{
	bwassert( m_pszString );
	bwassert( psz );

	// There is no library function for this operation, so I'll
	// have to code it myself
	int nCmp = ::strlen(psz);
	int	indx;
	int indx2;

	for (indx=::strlen(m_pszString)-1; indx>=nCmp-1; --indx) {
		for (indx2=0; indx2<nCmp; ++indx2) {
			if (m_pszString[indx-indx2]!=psz[nCmp-1-indx2])
				break;
		}
		if (indx2==nCmp)
			return indx - (nCmp-1);
	}
	return -1;
}


/////////////////////////////////////////////////////////////////////////
/*: routine String::startsWith

  Tells if this string starts with the given string. Case dependent.

  Prototype: bool startsWith( const char* psz ) const
  Prototype: bool startsWith( const String& str ) const
*/
bool String::startsWith( const char* psz ) const
{
	bwassert( m_pszString );
	bwassert( psz );
	return ::strstr(m_pszString, psz) == m_pszString;
}


/////////////////////////////////////////////////////////////////////////
/*: routine String::substring

  Extracts a substring.

  Extracts from the start index to just before the end index.  If
  no end index is given, extracts to the end of the string.  The
  indices must be contained within the string and end>=start.

  Prototype: String substring( int start ) const
  Prototype: String substring( int start, int end ) const
*/
String String::substring( int start ) const
{
	bwassert( m_pszString );
	bwassert( start>=0 );
	bwassert( start<=length() );
	return String( &m_pszString[start] );
}

String String::substring( int start, int end ) const
{
	bwassert( m_pszString );
	bwassert( start>=0 );
	bwassert( start<=length() );
	bwassert( end>=0 );
	bwassert( end<=length() );
	bwassert( end>=start );

	String str;
	int len = end-start;
	str.ensureCapacity( len );
	::strncpy(str.m_pszString, &m_pszString[start], len);
	str.m_pszString[len] = '\0';

	return str;
}


/////////////////////////////////////////////////////////////////////////
/*: routine String::toUpperCase()

  Convert String to upper case.  Current c locale is used.
*/
void String::toUpperCase()
{
	bwassert( m_pszString );

	for (char *pch = m_pszString; *pch; pch++)
		*pch = toupper(*pch);
}

/////////////////////////////////////////////////////////////////////////
/*: routine String::toLowerCase()

  Convert String to lower case.  Current c locale is used.
*/
void String::toLowerCase()
{
	bwassert( m_pszString );

	for (char *pch = m_pszString; *pch; pch++)
		*pch = tolower(*pch);
}

}	// namespace bw
