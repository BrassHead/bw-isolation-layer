/* ustring.cc -- basic unicode (UTF16) string functions

Copyright (C) 1997-2013, Brian Bray

*/

//
// There is probably a clever way to reduce the redundancy of the code below,
// but this was hard to do without either adding extra call overhead or
// requiring that <string.h> be included in the header file and thus into
// the users program.

#include "bw/bwassert.h"
#include "bw/ustring.h"

#include <wctype.h>
#include <wchar.h>
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
/*: class UString

  Represents a null terminated unicode string.

  This implementation is uses pointers to null terminated wide character
  arrays.
  Reference counts are not used.  Characters are unicode.
*/


/////////////////////////////////////////////////////////////////////////
/*: routine UString::UString

  Constructors.   Parameters specify an initial string in one of a variety
  of formats or give an integer for the initial capacity.

  Prototype: UString( const int length=0 )
  Prototype: UString( const String& str );
  Prototype: UString( const wchar_t* psz );
  Prototype: UString( const wchar_t* ps, const int len );
*/
UString::UString( const int len )
{
	bwassert( len>=0 );
	bwassert( len<65536 );	// Reasonableness, not strictly required

	m_lenMax = (len+quantaAlloc) & -quantaAlloc;  // Adds one and rounds up
	bwassert( m_lenMax>len );
	bwassert( (m_lenMax&15) == 0 );
	m_pszString = new wchar_t[m_lenMax];
	*m_pszString = L'\0';
}


UString::UString( const UString& str )
{
	int len = str.length();
	bwassert( len>=0 );
	bwassert( len<65536 );	// Reasonableness, not strictly required

	m_lenMax = (len+quantaAlloc) & -quantaAlloc;  // Adds one and rounds up
	m_pszString = new wchar_t[m_lenMax];
	::wmemcpy( m_pszString, str.m_pszString, len+1 );
}


UString::UString( const wchar_t* psz)
{
	bwassert( psz );
	int len = ::wcslen( psz );
	bwassert( len>=0 );
	bwassert( len<65536 ); // Reasonableness, not strictly required

	m_lenMax = (len+quantaAlloc) & -quantaAlloc;  // Adds one and rounds up

	m_pszString = new wchar_t[m_lenMax];
	::wmemcpy( m_pszString, psz, len+1 );
}


UString::UString( const wchar_t* ps, const int len )
{
	bwassert( len>=0 );
	bwassert( len<65536 );       // Reasonableness

	m_lenMax = (len+quantaAlloc) & -quantaAlloc;  // Adds one and round up
	m_pszString = new wchar_t[m_lenMax];
	::wmemcpy( m_pszString, ps, len );
	m_pszString[len] = L'\0';
}

/////////////////////////////////////////////////////////////////////////
/*: routine UString::~UString

    Destructor
*/
UString::~UString()
{
	bwassert( m_pszString );
	delete [] m_pszString;
#ifdef _DEBUG
	m_pszString = 0;		// Will cause rogue pointers to assert
#endif
}

/////////////////////////////////////////////////////////////////////////
/*: routine UString::operator+

    UString concatenation operators.

    Prototype: wchar_t* + UString
    Prototype: UString + UString
    Prototype: UString + wchar_t*
*/
UString operator+( const wchar_t* psz, const UString& str )
{
	UString strRet( ::wcslen(psz)+str.length() );
	::wcscpy( strRet.m_pszString, psz );
	::wcscat( strRet.m_pszString, str.m_pszString );
	return strRet;
}


UString UString::operator+( const UString& str ) const
{
	bwassert( m_pszString );
	bwassert( str.m_pszString );
	UString strRet( length()+str.length() );
	::wcscpy( strRet.m_pszString, m_pszString );
	::wcscat( strRet.m_pszString, str.m_pszString );
	return strRet;
}


UString UString::operator+( const wchar_t* psz ) const
{
	bwassert( m_pszString );
	UString strRet( length()+::wcslen(psz) );
	::wcscpy( strRet.m_pszString, m_pszString );
	::wcscat( strRet.m_pszString, psz );
	return strRet;
}


/////////////////////////////////////////////////////////////////////////
/*: routine UString::operator=

    Assignment operators.

    Prototype: (UString) = (UString);
    Prototype: (UString) = (wchar_t *);
    Prototype: (UString) = (wchar_t);
*/
UString& UString::operator=( const wchar_t* psz)
{
	bwassert( m_pszString );
	bwassert( psz );
	if (m_pszString != psz) {	// Handle a=a
		int len = ::wcslen(psz);
		bwassert( len>=0 );
		bwassert( len<65536 );	// Reasonableness, not strictly required

		if (len>=m_lenMax || m_lenMax>(len+maxSpill)) {
			delete [] m_pszString;
			m_lenMax = (len+quantaAlloc) & -quantaAlloc; // Add 1 and round up
			m_pszString = new wchar_t[m_lenMax];
		}
		::wcscpy( m_pszString, psz );
	}
	return *this;
}


UString& UString::operator=( wchar_t ch )
{
	bwassert( m_pszString );

	if (m_lenMax>(maxSpill+1)) {
		// Need to shorten
		delete [] m_pszString;
		m_lenMax = quantaAlloc;
		m_pszString = new wchar_t[quantaAlloc];
	}
	m_pszString[0] = ch;
	m_pszString[1] = L'\0';

	return *this;
}

/////////////////////////////////////////////////////////////////////////
/*: routine UString::operator(wchar_t*)

    wchar_t* conversion

    Prototype: operator const wchar_t*() const inline;
*/


/////////////////////////////////////////////////////////////////////////
/*: routine UString::length()

    Return length to terminator
*/
int UString::length() const
{
	bwassert( m_pszString );
	return ::wcslen( m_pszString );
}


/////////////////////////////////////////////////////////////////////////
/*: routine UString::append

    Append to string.

    Prototype: void append( const UString& str) inline
    Prototype: void append( const wchar_t* psz )
    Prototype: void append( wchar_t ch )
*/

void UString::append( const wchar_t* psz)
{
	bwassert( m_pszString );
	ensureCapacity( length()+::wcslen( psz ) );
	::wcscat( m_pszString, psz );
}

void UString::append( wchar_t ch )
{
	bwassert( m_pszString );

	int len = length();
	ensureCapacity( len+1 );
	m_pszString[len] = ch;
	m_pszString[len+1] = L'\0';
}


/////////////////////////////////////////////////////////////////////////
/*: routine UString::ensureCapacity()

    Make sure buffer is big enough.
*/
void UString::ensureCapacity( int len )
{
	bwassert( m_pszString );
	bwassert( len>=0 );
	bwassert( len<65536 );	// Reasonableness, not strictly required
	if (len>=m_lenMax) {
		m_lenMax = (len+quantaAlloc) & -quantaAlloc;	// Add 1 and round up
		wchar_t* pszNew = new wchar_t[m_lenMax];
		::wcscpy( pszNew, m_pszString );
		delete [] m_pszString;
		m_pszString = pszNew;
	}
}


/////////////////////////////////////////////////////////////////////////
/*: routine UString::capacity()

    Returns capacity of string (not including terminator)

    Prototype: 	int capacity() const inline;
*/


/////////////////////////////////////////////////////////////////////////
/*: routine UString::operator==

  Comparison operators for strings.  These are case sensitive and not
  localized.

  There are a complete set of comparison operators for UString's and
  wchar_t*'s.  Operators are: ==, !=, &lt;=, >=, &lt;, and >.

  Prototype: (UString) <op> (UString)
  Prototype: (UString) <op> (wchar_t*)
  Prototype: (wchar_t*) <op> (UString)
*/


/////////////////////////////////////////////////////////////////////////
/*: routine UString::compareTo

  Case sensitive comparison

  Prototype: int compareTo( const UString& str ) const
  Prototype: int compareTo( const wchar_t* psz ) const

  Returns: 0 if the strings are equal, >0 if the this string is
  greater than the given string, &lt;0 if this string is less than
  the given string.
*/
int UString::compareTo( const wchar_t* psz ) const
{
	bwassert( m_pszString );
	bwassert( psz );
	return ::wcscmp(m_pszString, psz);
}


/////////////////////////////////////////////////////////////////////////
/*: routine UString::equals()

  Case sensitive comparison for equality

  Prototype: bool equals( const wchar_t* psz ) const
  Prototype: bool equals( const UString& str ) const
*/


/////////////////////////////////////////////////////////////////////////
/*: routine UString::equalsIgnoreCase

  Case insensitive comparison for equality.

  Prototype: bool equalsIgnoreCase( const wchar_t* psz ) const
  Prototype: bool equalsIgnoreCase( const UString& str ) const
*/
bool UString::equalsIgnoreCase( const wchar_t* psz ) const
{
	bwassert( m_pszString );
	bwassert( psz );
	wchar_t* tpsz = m_pszString;
	while (*tpsz && *psz && towupper(*tpsz)==towupper(*psz))
		++psz, ++tpsz;
	return !(*psz || *tpsz);
}


/////////////////////////////////////////////////////////////////////////
/*: routine UString::indexOf()

  Finds first occurrance of character or string.

  Prototype: int indexOf( wchar_t ch ) const
  Prototype: int indexOf( const wchar_t* psz ) const
  Prototype: int indexOf( const UString& str ) const

  Returns:	Index of first occurrance or -1 if not found
*/
int UString::indexOf( wchar_t ch ) const
{
	bwassert( m_pszString );
	wchar_t *psz = ::wcschr(m_pszString, ch);
	if (psz)
		return psz - m_pszString;
	else
		return -1;
}

int UString::indexOf( const wchar_t* psz ) const
{
	bwassert( m_pszString );
	bwassert( psz );
	wchar_t* pszT = ::wcsstr(m_pszString, psz);
	if (pszT)
		return pszT - m_pszString;
	else
		return -1;
}


/////////////////////////////////////////////////////////////////////////
/*: routine UString::lastIndexOf

  Finds last occurrance of character or string.

  Prototype: int lastIndexOf( wchar_t ch ) const
  Prototype: int lastIndexOf( const wchar_t* psz ) const
  Prototype: int lastIndexOf( const UString& str ) const

  Returns:	Index of last occurrance or -1 if not found
*/
int UString::lastIndexOf( wchar_t ch ) const
{
	bwassert( m_pszString );
	wchar_t *psz = ::wcsrchr(m_pszString, ch);
	if (psz)
		return psz - m_pszString;
	else
		return -1;
}

int UString::lastIndexOf( const wchar_t* psz ) const
{
	bwassert( m_pszString );
	bwassert( psz );

	// There is no library function for this operation, so I'll
	// have to code it myself
	int nCmp = ::wcslen(psz);
	int	indx;
	int indx2;

	for (indx=::wcslen(m_pszString)-1; indx>=nCmp-1; --indx) {
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
/*: routine UString::startsWith

  Tells if this string starts with the given string. Case dependent.

  Prototype: bool startsWith( const wchar_t* psz ) const
  Prototype: bool startsWith( const UString& str ) const
*/
bool UString::startsWith( const wchar_t* psz ) const
{
	bwassert( m_pszString );
	bwassert( psz );
	return ::wcsstr(m_pszString, psz) == m_pszString;
}


/////////////////////////////////////////////////////////////////////////
/*: routine UString::substring

  Extracts a substring.

  Extracts from the start index to just before the end index.  If
  no end index is given, extracts to the end of the string.  The
  indices must be contained within the string and end>=start.

  Prototype: UString substring( int start ) const
  Prototype: UString substring( int start, int end ) const
*/
UString UString::substring( int start ) const
{
	bwassert( m_pszString );
	bwassert( start>=0 );
	bwassert( start<=length() );
	return UString( &m_pszString[start] );
}

UString UString::substring( int start, int end ) const
{
	bwassert( m_pszString );
	bwassert( start>=0 );
	bwassert( start<=length() );
	bwassert( end>=0 );
	bwassert( end<=length() );
	bwassert( end>=start );

	UString str;
	int len = end-start;
	str.ensureCapacity( len );
	::wcsncpy(str.m_pszString, &m_pszString[start], len);
	str.m_pszString[len] = L'\0';

	return str;
}


/////////////////////////////////////////////////////////////////////////
/*: routine UString::toUpperCase()

  Convert UString to upper case.  Current c locale is used.
*/
void UString::toUpperCase()
{
	bwassert( m_pszString );

	for (wchar_t *pch = m_pszString; *pch; ++pch)
		*pch = towupper(*pch);
}

/////////////////////////////////////////////////////////////////////////
/*: routine UString::toLowerCase()

  Convert UString to lower case.  Current c locale is used.
*/
void UString::toLowerCase()
{
	bwassert( m_pszString );

	for (wchar_t *pch = m_pszString; *pch; ++pch)
		*pch = towlower(*pch);
}

}	// namespace bw

