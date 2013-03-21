/* xml.cc: XML processor

Copyright (C) 1999-2013, Brian Bray

*/

#define NOTRACE
#include "bw/trace.h"

#include <string>
#include <map>
#include <list>
#include <fstream>

#include "bw/bwassert.h"
#include "bw/countable.h"
#include "bw/exception.h"
#include "bw/xml.h"

#include <cstring>


namespace bw {

// Numbered istream
//
// Note: Assume Latin-1 character set
//
class NumStream {
public:
	NumStream(istream& istIn, FileLoc& flocIn) : ist(istIn), floc(flocIn) {}
	int peek() {
		return ist.peek();
	}
	void ignore() {
		addone(ist.get());
	}
	int get() {
		int ch = ist.get();
		addone(ch);
		return ch;
	}
	void putback(char ch) {
		ist.putback(ch);
		subone(ch);
	}
	// Note: This is non-portable because I push back more than one char
	FileLoc location() const {
		return floc;
	}
	operator bool() const {
		return ist;
	}

private:
	void addone(int ch) {
		switch (ch) {
		case '\0':
		case '\r':
		case '\v':
		case '\f':
			break;
		case '\t':
			floc.column = ((floc.column+6) % 8) + 1;
			break;
		case '\n':
			floc.column = 1;
			++floc.line;
			break;
		default:
			++floc.column;
			break;
		}
	}
	void subone(int ch) {
		switch (ch) {
		case '\0':
		case '\r':
		case '\v':
		case '\f':
			break;
		case '\n':
			floc.column = -1;
			--floc.line;
			break;
		case '\t':
		default:
			--floc.column;
			break;
		}
	}

	istream&	ist;
	FileLoc&	floc;
};



// Character classification
//
// Note: This version assumes that all input is in the Latin-1 character set
//

static bool isLetter(unsigned char ch)
{
	return (0x41<=ch && 0x5a>=ch)
	       || (0x61<=ch && 0x7a>=ch)
	       || (0xc0<=ch && 0xd6>=ch)
	       || (0xd8<=ch && 0xf6>=ch)
	       || (0xf8<=ch && 0xfe>=ch);
}

static bool isNameChar(unsigned char ch)
{
	return isLetter(ch) || (0x30<=ch && 0x39>=ch) || ch=='.' || ch=='-'
	       || ch=='_' || ch==':' || ch==0xb7;
}

static bool isSpace(unsigned char ch)
{
	return ch==0x20 || ch==0x09 || ch==0x0a || ch==0x0d;
}



// Lexical routines

static bool foundSpace(string& contents, NumStream& ist)
{
	unsigned char ch = ist.peek();

	if (isSpace(ch)) {
		contents = ch;
		ist.ignore();

		while( ist && isSpace(ch = ist.peek()) ) {
			contents += ch;
			ist.ignore();
		}
		return true;
	}
	return false;
}


static bool foundName(string& name, NumStream& ist)
{
	unsigned char ch = ist.peek();
	if (isLetter(ch) || ch=='_' || ch==':') {
		name = ch;
		ist.ignore();

		while( ist && isNameChar(ch = ist.peek()) ) {
			name += ch;
			ist.ignore();
		}
		return true;
	}

	return false;
}

static bool foundQuoted(string& value, NumStream& ist)
{
	// Note: reads in References (character and entity) literaly.

	unsigned char ch1 = ist.peek();
	if ( ch1=='"' || ch1=='\'') {
		value = "";
		ist.ignore();
		unsigned char ch;
		while ( ist && (ch=ist.get()) ) {
			if (ch==ch1 || ch=='<')
				break;
			value += ch;
		}
		return true;
	}
	return false;
}


static bool foundAttribute(string& name, string& value, NumStream& ist)
{
	if (foundName(name,ist)) {
		string dmy;
		(void) foundSpace(dmy,ist);

		if (ist.get()!='=')
			throw BXMLException("Missing = for attribute value",ist.location());

		(void) foundSpace(dmy,ist);

		if (!foundQuoted(value,ist))
			throw BXMLException("Attribute value must be quoted string",ist.location());

		return true;
	}
	return false;
}

static bool foundConst(const char* lit, NumStream& ist)
{
	const char* pch = lit;

	while (ist && *pch && ist.peek()==*pch) {
		ist.ignore();
		++pch;
	}

	if (*pch) {
		// Failed
		while (pch!=lit)
			ist.putback(*--pch);
		return false;
	}

	return true;
}


static bool foundToDelim(string& val,const char* delim, NumStream& ist)
{
	val = "";
	while( ist && !foundConst(delim,ist) )
		val += ist.get();

	return ist;		// Note: val is set even on failure
}

void XMLNode::visitChildren(XMLVisitor& vis)
{
	list<XMLNodeRef>::iterator iter;

	for (iter=myChildren.begin(); iter!=myChildren.end(); ++iter) {
		(*iter)->visit(vis);
	}
}

XMLNode::XMLNode(const string& value)
	: myValue(value)
{}


XMLDocument::XMLDocument(const string& filename)
	: XMLNode(filename)
{
	FileLoc floc;
	std::ifstream ist(filename.c_str());
	if (!ist.is_open())
		throw BXMLException("Unable to open XML input file.",floc);
	NumStream nst(ist,floc);
	load(nst);
}


XMLDocument::XMLDocument(istream& ist, FileLoc& floc)
	: XMLNode("")
{
	NumStream nst(ist,floc);
	load(nst);
}


XMLDocument::~XMLDocument()
{}

void XMLDocument::load(NumStream& ist)
{
	XMLNodeRef xnr = XMLDecl::found(ist);
	if (xnr)
		children().push_back(xnr);

	while( (xnr = XMLDocument::foundMisc(ist)) )
		children().push_back(xnr);

	xnr = XMLDocTypeDecl::found(ist);
	if (xnr)
		children().push_back(xnr);

	while( (xnr = XMLDocument::foundMisc(ist)) )
		children().push_back(xnr);

	xnr = XMLElement::found(ist);
	if (xnr)
		children().push_back(xnr);
	else
		throw BXMLException("Null document content.",ist.location());

	while( (xnr = XMLDocument::foundMisc(ist)) )
		children().push_back(xnr);
}

XMLNode* XMLDocument::foundMisc(NumStream& ist)
{
	XMLNode* pxn;

	pxn = XMLText::foundSpace(ist);

	if (!pxn)
		pxn = XMLProcInst::found(ist);

	if (!pxn)
		pxn = XMLComment::found(ist);

	return pxn;
}

XMLNodeRef XMLDocument::getXMLDecl()
{
	XMLNode* pxn = *(children().begin());
	pxn->AddRef();
	return dynamic_cast<XMLDecl*>(pxn);
}

XMLNodeRef XMLDocument::getDocType()
{
	XMLNode* pxn;
	XMLDocTypeDecl* pxdt = 0;
	list<XMLNodeRef>::iterator iter;

	for (iter=children().begin(); iter!=children().end(); ++iter) {
		pxn = *iter;
		if ( (pxdt = dynamic_cast<XMLDocTypeDecl*>(pxn)) )
			break;
	}
	pxdt->AddRef();
	return pxdt;
}


XMLNodeRef XMLDocument::getElement()
{
	XMLNode* pxn;
	XMLElement* pxe = 0;
	list<XMLNodeRef>::iterator iter;

	for (iter=children().begin(); iter!=children().end(); ++iter) {
		pxn = *iter;
		if ( (pxe = dynamic_cast<XMLElement*>(pxn)) )
			break;
	}
	pxe->AddRef();
	return pxe;
}

void XMLDocument::visit(XMLVisitor& vis)
{
	vis.visit(this);
}


XMLElement::XMLElement(const string& tag)
	: XMLNode(tag)
{}

XMLElement* XMLElement::found(NumStream& ist)
{
	char ch = ist.peek();
	if (ch!='<')
		return 0;

	ist.ignore();
	string tag;
	if (!foundName(tag,ist)) {
		ist.putback('<');
		return 0;
	}

	XMLElement* pxe = new XMLElement(tag);

	string dmy;
	string name;
	string value;
	(void) foundSpace(dmy,ist);
	while (foundAttribute(name,value,ist)) {
		pxe->attr()[name] = value;
		(void) foundSpace(dmy,ist);
	}

	ch = ist.get();

	if (ch=='/' && ist.peek()=='>') {
		// Empty Element
		ist.ignore();
		return pxe;
	}

	if (ch!='>')
		throw BXMLException("Illegal attribute syntax in tag.",ist.location());

	XMLNodeRef xnr;
	while( (xnr = XMLElement::foundContent(ist)) )
		pxe->children().push_back(xnr);

	ch = ist.get();
	if (ch!='<' || ist.get()!='/')
		throw BXMLException("Syntax error looking for end tag.",ist.location());

	string endtag;
	if ( !(foundName(endtag,ist) && endtag==tag) ) {
		string err = "Unmatched end tag: </";
		err += endtag;
		err += ">.";
		throw BXMLException(err.c_str(),ist.location());
	}

	(void) foundSpace(dmy,ist);

	if (ist.get()!='>')
		throw BXMLException("Syntax error in end tag.",ist.location());

	trace << "Found XMLElement: " << pxe->value() << std::endl;
	return pxe;
}


XMLNode* XMLElement::foundContent(NumStream& ist)
{
	XMLNode* pxn;

	pxn = XMLElement::found(ist);

	if (!pxn)
		pxn = XMLText::foundCData(ist);

	if (!pxn)
		pxn = XMLText::found(ist);

	if (!pxn)
		pxn = XMLProcInst::found(ist);

	if (!pxn)
		pxn = XMLComment::found(ist);

	return pxn;
}

void XMLElement::visit(XMLVisitor& vis)
{
	vis.visit(this);
}


XMLText::XMLText(const string& text)
	: XMLNode(text)
{}

XMLText* XMLText::found(NumStream& ist)
{
	// Note: I allow ']]>' in character data even though it's not permitted
	// Note: Character and entity references are not replaced

	char ch = ist.peek();

	if (!ist || ch=='<')
		return 0;

	string val = "";

	while( ist && (ch=ist.get())!='<' ) {
		val += ch;
	}
	ist.putback(ch);

	trace << "Found XMLText : " << val << std::endl;
	return new XMLText(val);
}

XMLText* XMLText::foundSpace(NumStream& ist)
{
	char ch = ist.peek();

	if ( !(ist && isSpace(ch)) )
		return 0;

	string val = "";

	while( ist && isSpace(ch=ist.get()) ) {
		val += ch;
	}
	ist.putback(ch);

	trace << "Found spaces." << std::endl;
	return new XMLText(val);
}


XMLText* XMLText::foundCData(NumStream& ist)
{
	if (foundConst("<![CDATA[",ist)) {
		string val;
		if (!foundToDelim(val,"]]>",ist))
			throw BXMLException("Unterminated CDATA section.",ist.location());

		trace << "Found CDATA : " << val << std::endl;
		return new XMLText(val);
	}

	return 0;
}

void XMLText::visit(XMLVisitor& vis)
{
	vis.visit(this);
}


bool XMLText::isAllSpace() const
{
	string::const_iterator pch = value().begin();
	string::const_iterator end = value().end();

	while (pch!=end) {
		if (!isSpace(*pch))
			return false;
		++pch;
	}
	return true;
}


XMLProcInst::XMLProcInst(const string& target, const string& inst)
	: XMLNode(target), myInst(inst)
{}


XMLProcInst* XMLProcInst::found(NumStream& ist)
{
	// Note: this will match <?xml, so that should be checked first

	if (foundConst("<?",ist)) {
		string target;
		string dmy;
		if ( !foundName(target,ist) || !foundSpace(dmy,ist) )
			throw BXMLException("Illegal Processing Instruction target.",ist.location());

		string inst;
		if (!foundToDelim(inst,"?>",ist))
			throw BXMLException("Unterminated processing instructions.",ist.location());

		trace << "Found XMLProcInst: " << target << std::endl;
		return new XMLProcInst(target,inst);
	}

	return 0;
}

void XMLProcInst::visit(XMLVisitor& vis)
{
	vis.visit(this);
}


XMLComment::XMLComment(const string& comment)
	: XMLNode(comment)
{}


XMLComment* XMLComment::found(NumStream& ist)
{
	if (foundConst("<!--",ist)) {
		string comment;
		if ( !(foundToDelim(comment,"--",ist) && ist.get()=='>') )
			throw BXMLException("Unterminated comment.",ist.location());

		trace << "Found Comment." << std::endl;
		return new XMLComment(comment);
	}

	return 0;
}

void XMLComment::visit(XMLVisitor& vis)
{
	vis.visit(this);
}


XMLDecl::XMLDecl(const string& version)
	: XMLNode(version)
{}


XMLDecl* XMLDecl::found(NumStream& ist)
{
	// Note: This is slightly different from the spec in that a space is needed
	//	    after <?xml and not just any S character(s)

	if (foundConst("<?xml ",ist)) {
		string dmy;
		(void) foundSpace(dmy,ist);

		string name;
		string value;
		if (!foundAttribute(name,value,ist) || name!="version")
			throw BXMLException("Version number must be specified in <?xml ...?> declaration.",ist.location());

		XMLDecl* pxd = new XMLDecl(value);

		// remainder of markup is treated just like a list of attributes

		(void) foundSpace(dmy,ist);
		while (foundAttribute(name,value,ist)) {
			pxd->attr()[name] = value;
			(void) foundSpace(dmy,ist);
		}

		if (!foundConst("?>",ist))
			throw BXMLException("Syntax error in <?xml ...?> declaration.",ist.location());

		trace << "Found XMLDecl: version = " << value << std::endl;
		return pxd;
	}
	return 0;
}

void XMLDecl::visit(XMLVisitor& vis)
{
	vis.visit(this);
}


XMLDocTypeDecl::XMLDocTypeDecl(
    const string& name,
    const string& systemname,
    const string& pubname)

	: XMLNode(name), mySysName(systemname), myPubName(pubname)
{}

XMLDocTypeDecl* XMLDocTypeDecl::found(NumStream& ist)
{
	// Note: ends with space and not any S character

	if (foundConst("<!DOCTYPE ",ist)) {
		string dmy;
		(void) foundSpace(dmy,ist);

		string name;
		if (!foundName(name,ist))
			throw BXMLException("Missing name field <!DOCTYPE > declaration.",ist.location());

		(void) foundSpace(dmy,ist);

		string exttype;
		string pubname;
		string sysname;
		if (foundName(exttype,ist)) {
			if (exttype=="PUBLIC") {
				(void) foundSpace(dmy,ist);
				if (!foundQuoted(pubname,ist))
					throw BXMLException("Missing or invalid public name in <!DOCTYPE > declaration.",ist.location());
			} else if (exttype!="SYSTEM")
				throw BXMLException("Unknown DOCTYPE type.",ist.location());

			(void) foundSpace(dmy,ist);
			if (!foundQuoted(sysname,ist))
				throw BXMLException("Missing or invalid system name in <!DOCTYPE > declaration.",ist.location());
		}

		XMLDocTypeDecl* pxdt = new XMLDocTypeDecl(name,sysname,pubname);

		// Note: This is where we would read the doctype information if we
		// were a validating parser

		(void) foundSpace(dmy,ist);

		if (ist.peek()=='[') {
			// Has internal declarations
			ist.ignore();

			XMLNode* pxn;

			while (true) {
				pxn = XMLText::foundSpace(ist);

				if (!pxn)
					pxn = XMLComment::found(ist);

				if (!pxn)
					pxn = XMLMarkupDecl::found(ist);

				if (!pxn)
					pxn = XMLPEReference::found(ist);

				if (!pxn)
					pxn = XMLProcInst::found(ist);

				if (!pxn)
					break;

				pxdt->children().push_back(pxn);
			}

			if (ist.get()!=']')
				throw BXMLException("Missing ] in <!DOCTYPE > declaration.",ist.location());
		}

		(void) foundSpace(dmy,ist);

		if (ist.get()!='>')
			throw BXMLException("Syntax error in <!DOCTYPE > declaration.",ist.location());

		trace << "Found XMLDocTypeDecl: " << name << std::endl;
		return pxdt;
	}
	return 0;
}

void XMLDocTypeDecl::visit(XMLVisitor& vis)
{
	vis.visit(this);
}


XMLMarkupDecl::XMLMarkupDecl(const string& value)
	: XMLNode(value)
{}

XMLMarkupDecl* XMLMarkupDecl::found(NumStream& ist)
{
	// Note: we don't process these, so we just store the interior text
	// Note: This is not too bright and will be fooled by ">" within quotes

	if (foundConst("<!",ist)) {
		string value;
		if (!foundToDelim(value,">",ist))
			throw BXMLException("Unterminated markup declaration.",ist.location());

		trace << "Found XMLMarkupDecl: " << value << std::endl;
		return new XMLMarkupDecl(value);
	}
	return 0;
}

void XMLMarkupDecl::visit(XMLVisitor& vis)
{
	vis.visit(this);
}


XMLPEReference::XMLPEReference(const string& name)
	: XMLNode(name)
{}


XMLPEReference* XMLPEReference::found(NumStream& ist)
{
	if (ist.peek()=='%') {
		ist.ignore();
		string name;
		if (!foundName(name,ist))
			throw BXMLException("Illegal parameter entity reference name.",ist.location());
		if (ist.get()!=';')
			throw BXMLException("Unterminated parameter entity reference.",ist.location());

		trace << "Found XMLPEReference: " << name << std::endl;
		return new XMLPEReference(name);
	}
	return 0;
}

void XMLPEReference::visit(XMLVisitor& vis)
{
	vis.visit(this);
}


/////////////////////////////////////////////
// BXMLException


BXMLException::BXMLException(const char *msg, FileLoc floc)
	: myLine(floc.line), myColumn(floc.column)
{
	std::strncpy(msgBuffer,msg,msgBufferSize);
	msgBuffer[msgBufferSize-1] = '\0';
}


BXMLException::~BXMLException()
{}


const char* BXMLException::message() const
{
	return msgBuffer;
}


const char* BXMLException::name() const
{
	return "XMLError";
}


BException::Severity BXMLException::severity() const
{
	return Error;
}


int BXMLException::line() const
{
	return myLine;
}


int BXMLException::column() const
{
	return myColumn;
}

}	// namespace bw
