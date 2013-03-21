// Compile test

#include <string>
#include <map>
#include <list>
#include <iostream>
#include <fstream>

#include "bw/bwassert.h"
#include "bw/countable.h"
#include "bw/exception.h"
#include <bw/xml.h>

using bw::XMLVisitor;
using bw::XMLDocument;
using bw::XMLDocRef;

using std::ostream;
using std::ofstream;
using std::cout;
using std::cerr;
using std::endl;
using std::map;
using std::string;

class XMLEcho : public XMLVisitor {
public:
	XMLEcho(ostream& out);
	virtual ~XMLEcho() {};
	virtual void visit(XMLDocument* px);
	virtual void visit(bw::XMLElement* px);
	virtual void visit(bw::XMLText* px);
	virtual void visit(bw::XMLProcInst* px);
	virtual void visit(bw::XMLComment* px);
	virtual void visit(bw::XMLDecl* px);
	virtual void visit(bw::XMLDocTypeDecl* px);
	virtual void visit(bw::XMLMarkupDecl* px);
	virtual void visit(bw::XMLPEReference* px);

private:
	ostream&	ost;
};


XMLEcho::XMLEcho(ostream& out)
	: ost(out)
{}

void XMLEcho::visit(XMLDocument* px)
{
	px->visitChildren(*this);
}

void XMLEcho::visit(bw::XMLElement* px)
{
	ost << "<" << px->value();

	map<string,string>::iterator iattr;

	for (iattr=px->attr().begin(); iattr!=px->attr().end(); ++iattr) {
		// TODO: Handle "s and 's
		ost << " " << (*iattr).first << "=\"" << (*iattr).second << "\"";
	}

	if (px->children().empty()) {
		ost << "/>";
	} else {
		ost << ">";
		px->visitChildren(*this);
		ost << "</" << px->value() << ">";
	}
}

void XMLEcho::visit(bw::XMLText* px)
{
	if (px->value().find('<')==string::npos)
		ost << px->value();
	else
		ost << "<![CDATA[" << px->value() << "]]>";
}

void XMLEcho::visit(bw::XMLProcInst* px)
{
	ost << "<?" << px->value() << " " << px->myInst << "?>";
}

void XMLEcho::visit(bw::XMLComment* px)
{
	ost << "<!--" << px->value() << "-->";
}

void XMLEcho::visit(bw::XMLDecl* px)
{
	ost << "<?xml version=\"" << px->value() << "\"";

	map<string,string>::iterator iattr;

	for (iattr=px->attr().begin(); iattr!=px->attr().end(); ++iattr) {
		// TODO: Handle "s and 's in values
		ost << " " << (*iattr).first << "=\"" << (*iattr).second << "\"";
	}

	ost << "?>";
}

void XMLEcho::visit(bw::XMLDocTypeDecl* px)
{
	ost << "<!DOCTYPE " << px->value();
	if (px->myPubName!="")
		ost << " PUBLIC \"" << px->myPubName << "\" \"" << px->mySysName << "\"";
	else if (px->mySysName!="")
		ost << " SYSTEM \"" << px->mySysName << "\"";
	if (!px->children().empty()) {
		ost << " [";
		px->visitChildren(*this);
		ost << "]";
	}
	ost << ">";
}

void XMLEcho::visit(bw::XMLMarkupDecl* px)
{
	ost << "<!" << px->value() << ">";
}

void XMLEcho::visit(bw::XMLPEReference* px)
{
	ost << "%" << px->value() << ";";
}



int main( int argc, char *argv[])
{
	try {
		XMLDocRef theDoc;

		if (argc!=3) {
			cerr << "Usage: test1 <filename> <output>\n";
			return 1;
		}
		theDoc = new XMLDocument(argv[1]);
		ofstream myout(argv[2]);

		cout << "Document read\n";

		cout << theDoc->children().size() << " elements from the document.\n";

		XMLEcho echo(myout);
		theDoc->visit(echo);
	} catch( const char* msg) {
		cerr << msg << endl;
	}
}
