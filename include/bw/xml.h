/* xml.h: interface to the XML processor

Copyright (C) 1999-2013, Brian Bray

*/

/* Needs:
#include "bw/countable.h"
#include "bw/exception.h"
#include <string>
#include <map>
#include <list>
#include <iostream>
*/

namespace bw {

class BException;

using std::string;
using std::map;
using std::list;
using std::istream;

class NumStream;
class XMLNode;
class XMLDocument;
typedef cptr<XMLNode> XMLNodeRef;
typedef cptr<XMLDocument> XMLDocRef;

class XMLVisitor;

class XMLNode : public Countable {
public:
	virtual ~XMLNode() {}
	string& value() {
		return myValue;
	}
	const string& value() const {
		return myValue;
	}
	map<string,string>& attr() {
		return myAttr;
	}
	list<XMLNodeRef>& children() {
		return myChildren;
	}

	virtual void visit(XMLVisitor& vis) =0;
	void visitChildren(XMLVisitor& vis);

protected:
	XMLNode(const string& value); // : myValue(value) {}

private:
	string			myValue;
	map<string,string>	myAttr;
	list<XMLNodeRef>		myChildren;
};

struct FileLoc {
	FileLoc() : line(1), column(1) {}

	int	line;
	int	column;
};

class XMLDocument : public XMLNode {
public:
	XMLDocument(const string& filename);
	XMLDocument(istream& ist, FileLoc& floc);
	~XMLDocument();

	XMLNodeRef getXMLDecl();
	XMLNodeRef getDocType();
	XMLNodeRef getElement();

	virtual void visit(XMLVisitor& vis);

private:
	void load(NumStream& ist);
	static XMLNode* foundMisc(NumStream& ist);
};

class XMLElement : public XMLNode {
public:
	XMLElement(const string& tag);

	static XMLElement* found(NumStream& ist);

	string& tag() {
		return value();
	}

	virtual void visit(XMLVisitor& vis);

private:
	static XMLNode* foundContent(NumStream& ist);
};


class XMLText : public XMLNode {
public:
	XMLText(const string& text);

	static XMLText* found(NumStream& ist);
	static XMLText* foundSpace(NumStream& ist);
	static XMLText* foundCData(NumStream& ist);

	virtual void visit(XMLVisitor& vis);

	bool isAllSpace() const;
};

class XMLProcInst : public XMLNode {
public:
	XMLProcInst(const string& target, const string& inst);

	static XMLProcInst* found(NumStream& ist);

	virtual void visit(XMLVisitor& vis);

	string	myInst;
};

class XMLComment : public XMLNode {
public:
	XMLComment(const string& comment);

	static XMLComment* found(NumStream& ist);

	virtual void visit(XMLVisitor& vis);
};


class XMLDecl : public XMLNode {
public:
	XMLDecl(const string& version);

	static XMLDecl* found(NumStream& ist);

	virtual void visit(XMLVisitor& vis);
};



class XMLDocTypeDecl : public XMLNode {
public:
	XMLDocTypeDecl(const string& name,
	               const string& systemname,
	               const string& pubname);

	static XMLDocTypeDecl* found(NumStream& ist);

	virtual void visit(XMLVisitor& vis);

	string mySysName;
	string myPubName;
};


class XMLMarkupDecl : public XMLNode {
public:
	XMLMarkupDecl(const string& value);

	static XMLMarkupDecl* found(NumStream& ist);

	virtual void visit(XMLVisitor& vis);
};


class XMLPEReference : public XMLNode {
public:
	XMLPEReference(const string& name);

	static XMLPEReference* found(NumStream& ist);

	virtual void visit(XMLVisitor& vis);
};

class XMLVisitor {
public:
	virtual ~XMLVisitor() {};
	virtual void visit(XMLDocument* px) =0;
	virtual void visit(XMLElement* px) =0;
	virtual void visit(XMLText* px) =0;
	virtual void visit(XMLProcInst* px) =0;
	virtual void visit(XMLComment* px) =0;
	virtual void visit(XMLDecl* px) =0;
	virtual void visit(XMLDocTypeDecl* px) =0;
	virtual void visit(XMLMarkupDecl* px) =0;
	virtual void visit(XMLPEReference* px) =0;
};

class BXMLException : public BException {
public:
	BXMLException(const char *msg, FileLoc floc);
	virtual ~BXMLException();

	virtual const char* message() const;
	virtual const char* name() const;
	virtual Severity severity() const;

	// Specific routines

	int line() const;
	int column() const;		// Note: column<0 ==> near end of line

private:
	enum 	{msgBufferSize=100};
	char		msgBuffer[msgBufferSize];
	int		myLine;
	int		myColumn;
};


}	// namespace bw
