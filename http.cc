/* http.cpp -- parser for http returned form data

Copyright (C) 1997-2013, Brian Bray


*/

#include <map>
#include <sstream>

#define NOTRACE
#include "bw/trace.h"
#include "bw/http.h"
#include "bw/exception.h"
#include "bw/bwassert.h"
#include "bw/string.h"

#include <cstdlib>
#include <cstring>
#include <cctype>



extern char **environ;

#define INBUFSIZE 4000

namespace bw {

using std::map;
using std::istream;

// internal routines
bool allspace(const char* psz)
{
	while(isspace(*psz))
		++psz;
	return *psz=='\0';
}

bool startstr(const char* buf, const char* prefix)
{
	return strncmp(buf,prefix,strlen(prefix))==0;
}

String extractkey(const char* buf, const String key)
{
	String res = buf;
	int loc = res.indexOf(key+"=");
	if (loc<0)
		return "";
	res = res.substring(loc+key.length()+1);
	if (res[0]=='"') {
		res = res.substring(1);
		loc = res.indexOf('"');
		if (loc>=0)
			res = res.substring(0,loc);
	}
	trace << "Extracting " << key << " from " << buf << " :" << res << std::endl;
	return res;
}

map< String, String > getAnyData()
{
	char* psz = getenv("REQUEST_METHOD");
	String method;
	if (psz)
		method = psz;
	if (method=="GET")
		return getGetData();
	else if (method=="POST") {
		psz = getenv("CONTENT_TYPE");
		String datatype;
		if (psz)
			datatype = getenv("CONTENT_TYPE");
		if (datatype.startsWith("multipart/form-data;"))
			return getMultiData();
		else
			return getPostData();
	}

	return map< String, String>();
}


map< String, String > getPostData()
{
	return parseHTTPData( std::cin );
}


map< String, String > getGetData()
{
	char *psz = getenv("QUERY_STRING");
	if (psz) {
		std::istringstream is(psz);
		return parseHTTPData(is);
	}
	map< String, String > nullmap;
	return nullmap;
}

map< String, String > getMultiData()
{
	using std::cin;
	using std::endl;

	String divide;
	char inbuf[INBUFSIZE];
	String keyword;
	String filename;
	map< String, String > res;

	char *contenttype = getenv("CONTENT_TYPE");
	if (contenttype) {
		trace << "Content type is \"" << contenttype << "\"." << endl;
		String str = contenttype;
		if (startstr(contenttype,"multipart/form-data;"))
			divide = extractkey(str,"boundary");
	}
	if (divide=="") {
		throw BFormatException("Incorrect data format");
	}

	divide = "--" + divide;
	*inbuf = '\0';
	while( cin && !startstr(inbuf,divide) )
		cin.getline( inbuf, INBUFSIZE );
	while (cin) {
		bool inheader=true;
		keyword = "";
		filename = "";
		while( cin && inheader) {
			cin.getline( inbuf, INBUFSIZE );
			if (allspace(inbuf)) {
				inheader = false;
			} else if (startstr(inbuf,"Content-Disposition:")) {
				keyword = extractkey(inbuf,"name");
				filename = extractkey(inbuf,"filename");
			}
			// Other header lines ignored
		}
		if (cin && keyword=="")
			throw BFormatException("Multipart data format error");
		if (filename!="")
			res[keyword+"_name"] = filename;

		// Data starts with next input line
		while( cin && !inheader) {
			cin.getline( inbuf, INBUFSIZE );
			if (startstr( inbuf, divide ))
				inheader = true;
			else {
				int lastloc = strlen(inbuf)-1;
				if (inbuf[lastloc]=='\r')
					inbuf[lastloc] = '\0';
				res[keyword] = res[keyword] + inbuf + "\n";
			}
		}
		if (keyword!="")
			trace << "Read " << keyword << "=" << res[keyword] << endl;
	}
	trace << "Exiting multiread" << endl;
	return res;
}


String getValue(istream& is , char term)
{
	const String digits("0123456789ABCDEF");
	char ic;
	int nc1, nc2;
	String ret;
	while (is.get(ic)) {
		if (ic==term)
			break;
		if (ic=='%') {
			if (!is.get(ic))
				break;
			nc1 = digits.indexOf(toupper(ic));
			if (!is.get(ic))
				break;
			nc2 = digits.indexOf(toupper(ic));
			if (nc1==-1 || nc2==-1)
				throw BFormatException("Invalid hex digit");
			ic = (char)( (nc1<<4) | nc2 );
		} else if (ic=='+') {
			ic = ' ';
		}
		if (ic=='\0')
			throw BFormatException("Null character in input");

		ret.append(ic);
	}

	return ret;
}


map< String, String > parseHTTPData(istream& is)
{
	map< String, String > varmap;
	String key;
	String value;

	while (is) {
		key = getValue( is, '=' );
		value = getValue( is, '&' );
		if (key!="")
			varmap[key] = value;
	}

	if (!is.eof())
		throw BFormatException("Unexpected I/O error");

	return varmap;
}

}	// namespace bw
