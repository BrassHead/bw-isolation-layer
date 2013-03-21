// Main program to exercise CustomFile objects
//

#define NOTRACE
#include <bw/trace.h>

#include <list>
#include <map>

#include <bw/bwassert.h>
#include <bw/string.h>
#include <bw/custom.h>
#include <fstream>
#include <unistd.h>


using namespace bw;

using std::list;

const char* BWAppName = "ini";
const char* BWAppClass = "Ini";

void catfile(const char* fname)
{
	std::ifstream is(fname);
	char chbuf;
	while (is.get(chbuf)) {
		std::cout.put(chbuf);
	}
}

int main(int, char**)
{
	//
	CustomFile f1("inidata1.ini","fr_FR");

	bwverify( !f1.isWritable() );
	bwverify( f1.getString( "", "foo", "default" ) == "bar" );
	bwverify( f1.getNumber( "", "num", -1 ) == 3 );
	bwverify( f1.getString( "", "none", "default" ) == "default" );
	bwverify( f1.getNumber( "", "foo", -1 ) == -1 );
	bwverify( f1.getNumber( "csection", "a", -1 ) == 1 );
	bwverify( f1.getString( "csection", "a", "default" ) == "1" );
	bwverify( f1.getNumber( "csection", "b", -1 ) == 2 );
	bwverify( f1.getString( "worse", "empty", "default" ) == "default" );
	bwverify( f1.getString( "csection", "empty", "default" ) == "" );
	bwverify( f1.getString( "csection", "moreempty", "default" ) == "" );

	trace << f1.getString( "dsection", "foo", "Hi" ) << ".\n";
	trace << f1.getIndxString( "fsection", "foo", 3, "3" ) + ", "
	      << f1.getIndxString( "fsection", "foo", 2, "2" ) + ", "
	      << f1.getIndxString( "fsection", "foo", 1, "1" ) + ", "
	      << f1.getString( "fsection", "go", "go" ) + ".\n";

	bwverify( f1.getString( "dsection", "foo", "default" ) == "Bonjour" );
	bwverify( f1.getIndxString( "esection", "foo", 0, "default" ) == "default" );
	bwverify( f1.getIndxString( "esection", "foo", 1, "default" ) == "a" );
	bwverify( f1.getIndxString( "esection", "foo", 3, "default" ) == "c" );
	bwverify( f1.getIndxNumber( "esection", "foo", 6, -1 ) == 3 );
	bwverify( f1.getNumber( "esection", "foo6", -1 ) == 3 );

	bwverify( f1.getIndxString( "fsection", "foo", 1, "default" ) == "Un" );
	bwverify( f1.getIndxString( "fsection", "foo", 2, "default" ) == "Deux" );
	bwverify( f1.getIndxString( "fsection", "foo", 3, "default" ) == "Trois" );


	// Writing empty file
	CustomFile f2("/tmp/test1.ini");

	trace << "Categories of empty file: ";
	list<String> cats = f2.getCategories();
	list<String>::iterator iter;
	for (iter=cats.begin(); iter!=cats.end(); ++iter) {
		trace << " \"" << *iter << "\"";
	}
	trace << ".\n";

	f2.putString("","foo","bar");
	f2.putNumber("section","foo",2);
	f2.putIndxString("", "item", 1, "one");
	f2.putIndxNumber("", "item", 2, 2 );

	bwverify( f2.getString("","foo","default")=="bar" );
	bwverify( f2.getString("","foo","default")=="bar" );
	bwverify( f2.getNumber("section","foo",-1)==2 );
	bwverify( cats.size()==0 );
	cats = f2.getCategories();
	list<String> shouldbe;
	shouldbe.push_back("");
	shouldbe.push_back("section");
	bwverify( cats==shouldbe );
	shouldbe.clear();
	shouldbe.push_back("foo");
	shouldbe.push_back("item1");
	shouldbe.push_back("item2");
	cats = f2.getKeys("");
	bwverify( cats==shouldbe );

	catfile("cat /tmp/test1.ini");
	unlink("/tmp/test1.ini");
}

