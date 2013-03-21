/* html.h -- ostream controls for html output assistance.

Copyright (C) 1997-2013, Brian Bray

*/

/* Needs:
#include <fstream>
#include "bw/string.h"
*/

namespace bw {

class String;

class html {
public:
	static String httpProlog( const String& sMimeType );
	static String prolog( const String& sTitle, const String& sGenerator="bw" );
	static std::ostream& epilog( std::ostream& os );

	static String heading1( const String& sHead );
	static String heading2( const String& sHead );
	static String heading3( const String& sHead );
	static String heading4( const String& sHead );

	static String beginTable( int nCols );
	static std::ostream& beginRow( std::ostream& os );
	static std::ostream& beginCell( std::ostream& os );
	static std::ostream& nextCell( std::ostream& os );
	static std::ostream& endCell( std::ostream& os );
	static std::ostream& endRow( std::ostream& os );
	static std::ostream& endTable( std::ostream& os );

	static std::ostream& beginDefinitionList( std::ostream& os );
	static String definition( const String& sWhat );
	static std::ostream& endDefinitionList( std::ostream& os );

	static String beginLink( const String& sWhere );
	static String beginLink2Link( const String& sWhere );
	static std::ostream& endLink( std::ostream& os );
	static String defineLink( const String& sWhere );

	static std::ostream& rule( std::ostream& os );
	static std::ostream& newPara( std::ostream& os );
	static std::ostream& boldOn( std::ostream& os );
	static std::ostream& boldOff( std::ostream& os );
	static std::ostream& italicOn( std::ostream& os );
	static std::ostream& italicOff( std::ostream& os );

	static String literal( const String& sWhat, bool isMultiline=false );
	static String smartFormat( const String& sWhat );
};

}	// namespace bw

