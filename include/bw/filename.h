/* filename.h -- Interface for FileName class (expert in file names)

Copyright (C) 1997-2013, Brian Bray

*/

/* Needs:
#include "bw/string.h"
*/

namespace bw {

class String;

class FileName {
public:
	FileName(const char* pszFileName="");
	void setName(const char* pszFileName);
	bool isAbsolute() const;
	bool isPattern() const;
	void empty();
	String root() const;
	String directory() const;
	String baseName() const;
	String extension() const;
	String relativeName() const;
	String fullName() const;
	void makeAbsolute();
	void append(const FileName& fn);
	static FileName currentDirectory();
	operator const char*() const;

	enum Type {
		NoAccess = 0,
		RegularFile = 1,
		Directory = 2,
		Pipe = 3,
		Device = 4
	};
	FileName::Type getFileType() const;

private:
	void parse();

	String	m_sFileName;
	short	m_nRoot;
	short	m_nDir;
	short	m_nBase;
	short	m_nExt;
};

}   // namespace

