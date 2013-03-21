/* html.cpp -- ostream controls for html output assistance.

Copyright (C) 1997-2013, Brian Bray

*/

#include <ostream>
using std::ostream;

#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/html.h"



namespace bw {


/*: class html

	Ostream manipulators for writing HTML files.

	Prototype: os << html::httpProlog( const String& sMimeType );
	Prototype: os << html::prolog( const String& sTitle, const String& sGenerator="bw" );
	Prototype: os << html::epilog;

	Prototype: os << html::heading1( const String& sHead );
	Prototype: os << html::heading2( const String& sHead );
	Prototype: os << html::heading3( const String& sHead );
	Prototype: os << html::heading4( const String& sHead );

	Prototype: os << html::beginTable( int nCols );
	Prototype: os << html::beginRow;
	Prototype: os << html::beginCell;
	Prototype: os << html::nextCell;
	Prototype: os << html::endCell;
	Prototype: os << html::endRow;
	Prototype: os << html::endTable;

	Prototype: os << html::beginDefinitionList;
	Prototype: os << html::definition( const String& sWhat );
	Prototype: os << html::endDefinitionList;

	Prototype: os << html::beginLink( const String& sWhere );
	Prototype: os << html::beginLink2Link( const String& sWhere );
	Prototype: os << html::endLink;
	Prototype: os << html::defineLink( const String& sWhere );

	Prototype: os << html::rule;
	Prototype: os << html::newPara;
	Prototype: os << html::boldOn;
	Prototype: os << html::boldOff;
	Prototype: os << html::italicOn;
	Prototype: os << html::italicOff;

	Prototype: os << html::literal( const String& sWhat );
	Prototype: os << html::smartFormat( const String& sWhat );

	Description:

	These manipulators send HTML formatting codes to an ostream.

	The prolog and epilog manipulators write everything necessary to
	start and end the BODY of an HTML document.

	html::rule inserts a horizontal rule.

	html::literal( foo ) outputs foo expanding &lt;'s, &quot;'s
	and &amp;'s so that
	they appear literally in the output and are not interpreted as HTML
	commands.  Newlines are converted to "<br>" directives if isMultiLine
	is true.

	html::smartFormat( foo ) inserts paragraph markers into foo wherever
	blank lines are found.

	Here are a few examples to clarify the other ones that are not self evident:

	Definitions:
	<BR>os << html::beginDefinitionList;
	<BR>os << html::definition( "Item1" );
	<BR>os << "The definition of Item1";
	<BR>os << html::definition( "Item2" );
	<BR>os << "The definition of Item2";
	<BR>...
	<BR>os << html:endDefinitionList;

	Links:
	<BR>os << html::beginLink( "http://www.somewhere.com/" );
	<BR>os << "Click here";
	<BR>os << html::endLink;
	<BR>
	<BR>
	<BR>os << html::defineLink( "top" );
	<BR>os << "Some text.";
	<BR>...
	<BR>os << "Blah blah blah...";
	<BR>os << html::beginLink2Link( "top" );
	<BR>os << "Return to top of page.";
	<BR>os << endLink;

*/
String html::httpProlog( const String& sMimeType)
{
	return "Content-type: " + sMimeType + "\n\n";
}

String html::prolog( const String& sTitle, const String& sGenerator )
{
	return "<!DOCTYPE html>\n<HTML>\n<HEAD>\n"
	       "<META HTTP-EQUIV=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
	       "<TITLE>"
	       + sTitle +
	       "</TITLE>\n"
	       "<META name=\"GENERATOR\" content=\""
	       + sGenerator +
	       "\">\n<style type=\"text/css\">\n  body { max-width:43em; margin-left:auto; margin-right:auto }\n</style>\n</HEAD>\n<BODY BGCOLOR=white>\n";
}

ostream& html::epilog( ostream& os )
{
	os << "</BODY>\n</HTML>\n";
	return os;
}

String html::heading1( const String& sHead )
{
	return "<H1>" + sHead + "</H1>\n";
}

String html::heading2( const String& sHead )
{
	return "<H2>" + sHead + "</H2>\n";
}

String html::heading3( const String& sHead )
{
	return "<H3>" + sHead + "</H3>\n";
}

String html::heading4( const String& sHead )
{
	return "<H4>" + sHead + "</H4>\n";
}



String html::beginTable( int nCols )
{
	bwassert( nCols>0 );
	bwassert( nCols<100 );

	String s;
	s = "<TABLE COLS=";
	s.append( '0'+nCols/10 );
	s.append( '0'+nCols%10 );
	s.append( ">\n" );

	return s;
}

ostream& html::beginRow( ostream& os )
{
	os << "<TR>\n";
	return os;
}

ostream& html::beginCell( ostream& os )
{
	os << "<TD>\n";
	return os;
}

ostream& html::nextCell( ostream& os )
{
	os << "</TD><TD>\n";
	return os;
}

ostream& html::endCell( ostream& os )
{
	os << "</TD>\n";
	return os;
}

ostream& html::endRow( ostream& os )
{
	os << "</TR>\n";
	return os;
}

ostream& html::endTable( ostream& os )
{
	os << "</TABLE>\n";
	return os;
}


ostream& html::beginDefinitionList( ostream& os )
{
	os << "<DL>\n";
	return os;
}

String html::definition( const String& sWhat )
{
	return "<DT>" + sWhat + "\n<DD>";
}

ostream& html::endDefinitionList( ostream& os )
{
	os << "</DL>\n";
	return os;
}


String html::beginLink( const String& sWhere )
{
	return "<A HREF=\"" + sWhere + "\">";
}

String html::beginLink2Link( const String& sWhere )
{
	return "<A HREF=\"#" + sWhere + "\">";
}

ostream& html::endLink( ostream& os )
{
	os << "</A>\n";
	return os;
}

String html::defineLink( const String& sWhere )
{
	return "<A NAME=\"" + sWhere + "\"></A>\n";
}


ostream& html::rule( ostream& os )
{
	os << "\n<HR>\n";
	return os;
}

ostream& html::newPara( ostream& os )
{
	os << "<P>\n";
	return os;
}

ostream& html::boldOn( ostream& os )
{
	os << "<B>";
	return os;
}

ostream& html::boldOff( ostream& os )
{
	os << "</B>";
	return os;
}

ostream& html::italicOn( ostream& os )
{
	os << "<I>";
	return os;
}

ostream& html::italicOff( ostream& os )
{
	os << "</I>";
	return os;
}


String html::literal( const String& sWhat, bool isMultiLine )
{
	String s;

	int len = sWhat.length();
	for (int i=0; i<len; ++i) {
		switch (sWhat[i]) {
		case '<':
			s.append("&lt;");
			break;

		case '&':
			s.append("&amp;");
			break;

		case '"':
			s.append("&quot;");
			break;

		case '\n':
			s.append("\n");
			if (isMultiLine)
				s.append("<br>");
			break;

		default:
			s.append( sWhat[i] );
		}
	}
	return s;
}

String html::smartFormat( const String& sWhat )
{
	String s;
	int		iS = 0;
	int		indx;

	while ( (indx=iS+sWhat.substring(iS).indexOf("\n\n"))>=iS ) {
		s.append( sWhat.substring(iS,indx+1) + "<P>" );
		iS = indx+1;
	}
	s.append( sWhat.substring(iS) );

	return s;
}

}	// namespace bw
