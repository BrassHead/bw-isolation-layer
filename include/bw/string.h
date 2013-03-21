/* string.h -- basic string operations

Copyright (C) 1997-2013, Brian Bray

TODO: deprecated

*/

/* Needs:
#include <bw/bwassert.h>
*/

namespace bw {


// Represents a null terminated string of characters
class String {
public: //	Friends
	friend String operator+( const char*, const String& );
	friend bool operator==(const String& s1, const String& s2);
	friend bool operator!=(const String& s1, const String& s2);
	friend bool operator<=(const String& s1, const String& s2);
	friend bool operator>=(const String& s1, const String& s2);
	friend bool operator<(const String& s1, const String& s2);
	friend bool operator>(const String& s1, const String& s2);

public:	//	Ctor, Dtor
	explicit String( const int length=0 );
	String( const String& str );
	String( const char* psz );
	String( const char* ps, const int length );
	~String();

public:	//	Operators
	String operator+( const String& ) const;
	String operator+( const char* ) const;
	String& operator+=( const String& str2 ) {
		append(str2);
		return *this;
	}
	String& operator+=( const char* psz ) {
		append(psz);
		return *this;
	}
	String& operator+=( char ch ) {
		append(ch);
		return *this;
	}
	operator const char*() const {
		return m_pszString;
	}
	String& operator=( const String& str) {
		return operator=(str.m_pszString);
	}
	String& operator=( const char* );
	String& operator=( char );
	char& operator[](int indx) {
		bwassert(indx<=length());
		return m_pszString[indx];
	}
	const char& operator[](int indx) const {
		bwassert(indx<=length());
		return m_pszString[indx];
	}


public:	//	Functions
	const char* c_str() const {
		return m_pszString;
	}
	const char* data() const {
		return m_pszString;
	}
	int length() const;
	void append( const String& str ) {
		append(str.m_pszString);
	}
	void append( const char* );
	void append( char );
	void ensureCapacity( int );
	int capacity() const {
		return m_lenMax-1;
	}
	int compareTo( const String& str) const {
		return compareTo(str.m_pszString);
	}
	int compareTo( const char* ) const;
	bool equals( const String& str) const {
		return compareTo(str.m_pszString)==0;
	}
	bool equals( const char* psz) const {
		return compareTo(psz)==0;
	}
	bool equalsIgnoreCase( const String& str ) const {
		return equalsIgnoreCase(str.m_pszString);
	}
	bool equalsIgnoreCase( const char* ) const;
	bool startsWith( const String& str ) const {
		return startsWith(str.m_pszString);
	}
	bool startsWith( const char* ) const;
	int indexOf( char ) const;
	int indexOf( const char* ) const;
	int indexOf( const String& str) const {
		return indexOf(str.m_pszString);
	}
	int lastIndexOf( char ) const;
	int lastIndexOf( const char* ) const;
	int lastIndexOf( const String& str) const {
		return lastIndexOf(str.m_pszString);
	}
	String substring(int start) const;
	String substring(int start, int end) const;
	void toLowerCase();
	void toUpperCase();


private:  // Internal routines


private:  // Storage
	char*	m_pszString;
	int		m_lenMax;	// Allocated buffer length (includes null)
};

// Comparison operators
inline bool operator==(const String& s1, const String& s2)
{
	return s1.compareTo(s2.m_pszString)==0;
}

inline bool operator==(const String& s1, const char* s2)
{
	return s1.compareTo(s2)==0;
}

inline bool operator==(const char* s1, const String& s2)
{
	return s2.compareTo(s1)==0;
}

inline bool operator!=(const String& s1, const String& s2)
{
	return s1.compareTo(s2.m_pszString)!=0;
}

inline bool operator!=(const String& s1, const char* s2)
{
	return s1.compareTo(s2)!=0;
}

inline bool operator!=(const char* s1, const String& s2)
{
	return s2.compareTo(s1)!=0;
}

inline bool operator<=(const String& s1, const String& s2)
{
	return s1.compareTo(s2.m_pszString)<=0;
}

inline bool operator<=(const String& s1, const char* s2)
{
	return s1.compareTo(s2)<=0;
}

inline bool operator<=(const char* s1, const String& s2)
{
	return s2.compareTo(s1)>=0;
}

inline bool operator>=(const String& s1, const String& s2)
{
	return s1.compareTo(s2.m_pszString)>=0;
}

inline bool operator>=(const String& s1, const char* s2)
{
	return s1.compareTo(s2)>=0;
}

inline bool operator>=(const char* s1, const String& s2)
{
	return s2.compareTo(s1)<=0;
}

inline bool operator<(const String& s1, const String& s2)
{
	return s1.compareTo(s2.m_pszString)<0;
}

inline bool operator<(const String& s1, const char* s2)
{
	return s1.compareTo(s2)<0;
}

inline bool operator<(const char* s1, const String& s2)
{
	return s2.compareTo(s1)>0;
}

inline bool operator>(const String& s1, const String& s2)
{
	return s1.compareTo(s2.m_pszString)>0;
}

inline bool operator>(const String& s1, const char* s2)
{
	return s1.compareTo(s2)>0;
}

inline bool operator>(const char* s1, const String& s2)
{
	return s2.compareTo(s1)<0;
}

// Syntax sugar

// Localizable string constant TODO: StringConst V( S, __FILE__, V )
#define sconst( V,S ) static const String V(S);

// Non-Localizable string constant TODO: StringConst V( S )
#define sconsti( V,S ) static const String V(S);


}	// namespace bw




