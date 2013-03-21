/* ustring.h -- basic string operations

Copyright (C) 1997-2013, Brian Bray

*/

/* Needs:
#include <bw/bwassert.h>
*/

namespace bw {


// Represents a null terminated unicode string of characters
class UString {
public: //	Friends
	friend UString operator+( const wchar_t*, const UString& );
	friend bool operator==(const UString& s1, const UString& s2);
	friend bool operator!=(const UString& s1, const UString& s2);
	friend bool operator<=(const UString& s1, const UString& s2);
	friend bool operator>=(const UString& s1, const UString& s2);
	friend bool operator<(const UString& s1, const UString& s2);
	friend bool operator>(const UString& s1, const UString& s2);

public:	//	Ctor, Dtor
	explicit UString( const int length=0 );
	UString( const UString& str );
	UString( const wchar_t* psz );
	UString( const wchar_t* ps, const int length );
	~UString();

public:	//	Operators
	UString operator+( const UString& ) const;
	UString operator+( const wchar_t* ) const;
	UString& operator+=( const UString& str2 ) {
		append(str2);
		return *this;
	}
	UString& operator+=( const wchar_t* psz ) {
		append(psz);
		return *this;
	}
	UString& operator+=( wchar_t ch ) {
		append(ch);
		return *this;
	}
	operator const wchar_t*() const {
		return m_pszString;
	}
	UString& operator=( const UString& str) {
		return operator=(str.m_pszString);
	}
	UString& operator=( const wchar_t* );
	UString& operator=( wchar_t );
	wchar_t& operator[](int indx) {
		bwassert(indx<=length());
		return m_pszString[indx];
	}
	const wchar_t& operator[](int indx) const {
		bwassert(indx<=length());
		return m_pszString[indx];
	}


public:	//	Functions
	const wchar_t* c_str() const {
		return m_pszString;
	}
	const wchar_t* data() const {
		return m_pszString;
	}
	int length() const;
	void append( const UString& str ) {
		append(str.m_pszString);
	}
	void append( const wchar_t* );
	void append( wchar_t );
	void ensureCapacity( int );
	int capacity() const {
		return m_lenMax-1;
	}
	int compareTo( const UString& str) const {
		return compareTo(str.m_pszString);
	}
	int compareTo( const wchar_t* ) const;
	bool equals( const UString& str) const {
		return compareTo(str.m_pszString)==0;
	}
	bool equals( const wchar_t* psz) const {
		return compareTo(psz)==0;
	}
	bool equalsIgnoreCase( const UString& str ) const {
		return equalsIgnoreCase(str.m_pszString);
	}
	bool equalsIgnoreCase( const wchar_t* ) const;
	bool startsWith( const UString& str ) const {
		return startsWith(str.m_pszString);
	}
	bool startsWith( const wchar_t* ) const;
	int indexOf( wchar_t ) const;
	int indexOf( const wchar_t* ) const;
	int indexOf( const UString& str) const {
		return indexOf(str.m_pszString);
	}
	int lastIndexOf( wchar_t ) const;
	int lastIndexOf( const wchar_t* ) const;
	int lastIndexOf( const UString& str) const {
		return lastIndexOf(str.m_pszString);
	}
	UString substring(int start) const;
	UString substring(int start, int end) const;
	void toLowerCase();
	void toUpperCase();


private:  // Internal routines


private:  // Storage
	wchar_t*	m_pszString;
	int		m_lenMax;	// Allocated buffer length (includes null)
};

// Comparison operators
inline bool operator==(const UString& s1, const UString& s2)
{
	return s1.compareTo(s2.m_pszString)==0;
}

inline bool operator==(const UString& s1, const wchar_t* s2)
{
	return s1.compareTo(s2)==0;
}

inline bool operator==(const wchar_t* s1, const UString& s2)
{
	return s2.compareTo(s1)==0;
}

inline bool operator!=(const UString& s1, const UString& s2)
{
	return s1.compareTo(s2.m_pszString)!=0;
}

inline bool operator!=(const UString& s1, const wchar_t* s2)
{
	return s1.compareTo(s2)!=0;
}

inline bool operator!=(const wchar_t* s1, const UString& s2)
{
	return s2.compareTo(s1)!=0;
}

inline bool operator<=(const UString& s1, const UString& s2)
{
	return s1.compareTo(s2.m_pszString)<=0;
}

inline bool operator<=(const UString& s1, const wchar_t* s2)
{
	return s1.compareTo(s2)<=0;
}

inline bool operator<=(const wchar_t* s1, const UString& s2)
{
	return s2.compareTo(s1)>=0;
}

inline bool operator>=(const UString& s1, const UString& s2)
{
	return s1.compareTo(s2.m_pszString)>=0;
}

inline bool operator>=(const UString& s1, const wchar_t* s2)
{
	return s1.compareTo(s2)>=0;
}

inline bool operator>=(const wchar_t* s1, const UString& s2)
{
	return s2.compareTo(s1)<=0;
}

inline bool operator<(const UString& s1, const UString& s2)
{
	return s1.compareTo(s2.m_pszString)<0;
}

inline bool operator<(const UString& s1, const wchar_t* s2)
{
	return s1.compareTo(s2)<0;
}

inline bool operator<(const wchar_t* s1, const UString& s2)
{
	return s2.compareTo(s1)>0;
}

inline bool operator>(const UString& s1, const UString& s2)
{
	return s1.compareTo(s2.m_pszString)>0;
}

inline bool operator>(const UString& s1, const wchar_t* s2)
{
	return s1.compareTo(s2)>0;
}

inline bool operator>(const wchar_t* s1, const UString& s2)
{
	return s2.compareTo(s1)<0;
}

// Syntax sugar

// Localizable string constant TODO: StringConst V( S, __FILE__, V )
#define suconst( V,S ) static const UString V(S);

// Non-Localizable string constant TODO: StringConst V( S )
#define suconsti( V,S ) static const UString V(S);


}	// namespace bw




