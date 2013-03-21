/* trace.h -- Trace ofstream or dummy

Copyright (C)1996-2013, Brian Bray

*/

/* Needs:
#include <iostream>
*/

#ifdef _DEBUG
#ifndef NOTRACE
#define BTRACE_TRACING
#endif
#endif

#ifdef BTRACE_TRACING

#include <iostream>

#define trace std::cout
#define tracein std::cin

#else

#include <iostream>

class BTrace {};
extern BTrace trace, tracein;
template <class T> BTrace& operator<<(BTrace& tr, T)
{
	return tr;
}
inline BTrace& operator<<(BTrace& tr, std::ostream& (*)(std::ostream&))
{
	return tr;
}
inline BTrace& operator<<(BTrace& tr, std::ios& (*)(std::ios&))
{
	return tr;
}

template <class T> BTrace& operator>>(BTrace& tr, T)
{
	return tr;
}
inline BTrace& operator>>(BTrace& tr, std::istream& (*)(std::istream&))
{
	return tr;
}
inline BTrace& operator>>(BTrace& tr, std::ios& (*)(std::ios&))
{
	return tr;
}

#endif

