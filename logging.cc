/* logging.cc -- Implementation for event logging stream

Copyright (C) 1999-2013, Brian Bray

*/

#include <iomanip>
#include <fstream>
#include <iostream>

#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/logging.h"

#include <ctime>
using std::time;
using std::localtime;
#include <unistd.h>

namespace bw {

/*: class LogStream

  Output stream for event logging.

  This is a simple logging stream.  It doesn't interface to syslog()
  and expects good co-operation with the caller.

  Example:
    LogStream lout;
    lout(ERR) << "An error message." << endl;

  Rules:
    Ident should be program name
    Don't send '\n's
    Always end message with endl

  Output:
    Outputs lines starting with the date and time followed by the ident:
    and the message.
*/

/*: LogStream::LogStream

  Constructors.

  A LogStream must be 'open'ed before use if the default constructor is used.

  If no filename is given, logging is to clog (same as cerr).

  Prototype: LogStream::LogStream()
  Prototype: LogStream::LogStream(const char* ident)
  Prototype: LogStream::LogStream(const char* ident, const char* filename)
*/

LogStream::LogStream()
{}

LogStream::LogStream(const char* ident, const char* filename)
	: m_ident(ident), m_filename(filename)
{
	if (*filename!='\0')
		os.open(filename,std::ios::app);
}

/*: LogStream::~LogStream

  Destructor.
*/
LogStream::~LogStream()
{}

/*: LogStream::open()

  Open logging stream.

  If a filename is not given, then output goes to cerr.

  The logging stream must have been previously closed with close() or
  created with the default constructor.

  Prototype: void LogStream::open(const char* ident, const char* filename="")
*/
void LogStream::open(const char* ident, const char* filename)
{
	bwassert( !os.is_open() );
	m_ident = ident;
	m_filename = filename;
	if (*filename!='\0')
		os.open(filename,std::ios::app);
}

/*: LogStream::close()

  Closes open logging stream.
*/
void LogStream::close()
{
	bwassert( os.is_open() );
	os.close();
}

/*: LogStream::reopen()

  Close and then re-open logging stream.

  This routine is provided for log rotation.
*/
void LogStream::reopen()
{
	if (os.is_open())
		os.close();
	if (*m_filename!='\0')
		os.open(m_filename,std::ios::app);
}


std::ostream& LogStream::operator()(LogPriority)
{
	using std::clog;
	using std::setw;
	using std::setfill;

	// Note: The log priority is ignored.
	const int hnbuflen = 30;
	static char hostname[hnbuflen] = "";
	// One time
	if (*hostname=='\0') {
		gethostname(hostname,hnbuflen);
		if (*hostname=='\0') {
			hostname[0] = '-';
			hostname[1] = '\0';
		}
	}

	time_t now = time(0);
	struct tm *ptime = localtime(&now);
	std::ostream *pos;
	if (os)
		pos = &os;
	else
		pos = &clog;

	*pos << ptime->tm_year+1900 << '/'
	     << setw(2) << setfill('0') << ptime->tm_mon+1 << '/'
	     << setw(2) << setfill('0') << ptime->tm_mday << ' '
	     << setw(2) << setfill('0') << ptime->tm_hour << ':'
	     << setw(2) << setfill('0') << ptime->tm_min << ':'
	     << setw(2) << setfill('0') << ptime->tm_sec << '\t'
	     << hostname << '\t'
	     << m_ident << ":\t ";
	return *pos;
}


}	//namespace bw


