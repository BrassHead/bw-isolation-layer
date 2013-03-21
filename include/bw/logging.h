/* loggin.h -- Interface for BW EventLog class

Copyright (C) 1999-2013, Brian Bray

*/

/* Needs:
#include <fstream>
#include "bw/string.h"
*/

enum LogPriority {EMERG, ALERT, CRIT, ERR, WARNING, NOTICE, INFO, DEBUG};

namespace bw {

class String;

class LogStream {
public:

	LogStream();		// Must be opened before use
	LogStream(const char* ident, const char* filename="");
	~LogStream();

	void open(const char* ident, const char* filename="");
	void close();
	void reopen();

	std::ostream& operator()(LogPriority lprio);

private:
	std::ofstream	os;
	String	m_ident;
	String	m_filename;
};

}	//namespace bw
