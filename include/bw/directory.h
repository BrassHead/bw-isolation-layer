/* directory.h -- interface for directory search routines

Copyright (C) 1997-2013, Brian Bray

*/

/*
#include "bw/filename.h"
*/

struct dirent;

namespace bw {


class Directory
// A Directory acts like a forward iterator over a list of FileNames
// TODO: Make more like a container
{
public:
	Directory(const FileName& fnDir, const FileName& fnPattern="*");
	~Directory();

	Directory& operator++();
	void operator++(int);
	FileName* operator->();
	FileName& operator*();
	bool atEof() const;
	void rewind();

	FileName::Type getType() const;

private:
	FileName		m_fnDir;
	FileName		m_fnPattern;
	FileName		m_fnResult;
	void*		m_pvdir;
	dirent*		m_pde;
	bool		m_atEof;

	// Prohibit copying
	Directory( const Directory& );
	Directory& operator=( const Directory& );

	// Internal
	void step();
};


}   // namespace





