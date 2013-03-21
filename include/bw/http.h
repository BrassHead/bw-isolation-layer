/* http.h -- interface to parser for http returned form data

Copyright (C) 1997-2013, Brian Bray

*/

/* Needs:
#include <map>
#include <istream> or <sstream>
#include <bw/string.h>

TODO: Forward Declarations
*/


namespace bw {

class String;

std::map< String, String > getAnyData();
std::map< String, String > getPostData();
std::map< String, String > getGetData();
std::map< String, String > getMultiData();
std::map< String, String > parseHTTPData(std::istream& is);

}	// namespace bw

