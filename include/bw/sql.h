/* bw/sql.h -- SQL/CLI (ODBC) isolation layer interface

Copyright (C) 1999-2013, Brian Bray

*/

/* Needs:
#include "bw/string.h"
#include "bw/exception.h"
*/

// Forward declarations

typedef void* SQLHANDLE;	// Used to be long
//typedef long SQLHANDLE;
typedef SQLHANDLE SQLHSTMT;
typedef SQLHANDLE SQLHENV;
typedef SQLHANDLE SQLHDBC;
typedef short int SQLRETURN;

namespace bw {

class String;
class BException;

class SQLTimestamp {
public:
	SQLTimestamp();	// Now
	SQLTimestamp(short year, unsigned short month, unsigned short day,
	             unsigned short hour=0, unsigned short minute=0,
	             unsigned short second=0, unsigned int fraction=0 );
	~SQLTimestamp();
	short& year();
	unsigned short& month();
	unsigned short& day();
	unsigned short& hour();
	unsigned short& minute();
	unsigned short& second();
	unsigned int& fraction();

	bool operator<(const SQLTimestamp& sts);

private:
	friend class SQLStatement;

	void* myTs;
};

class SQLConnection {
public:
	SQLConnection( const char* database, const char* userid, const char* password );
	~SQLConnection();

private:
	friend class SQLStatement;
	friend class BSQLException;

	SQLHSTMT allocStmt() const;

	static int	numConnects;
	static SQLHENV	henv;
	SQLHDBC		hdbc;
};

class SQLStatement {
public:
	SQLStatement(const SQLConnection& connect);
	~SQLStatement();

	void execDirect( const char* stmt );
	void prepare( const char* stmt );
	void execute();
	bool fetch();

	void getData(int column, long* res) const;
	void getData(int column, String& res) const;
	void getData(int column, char* buf, int buflen) const;
	void getData(int column, SQLTimestamp* res) const;

	void bindParameter(int column, const char* buf);
	void bindParameter(int column, char* buf, int buflen);
	void bindParameter(int column, long* pValue);
	void bindParameter(int column, SQLTimestamp* pValue);

	void bindColumn(int column, char* buf, int buflen);
	void bindColumn(int column, long* pValue);
	void bindColumn(int column, SQLTimestamp* pValue);

	void closeCursor();
	void recycle();

	operator SQLHSTMT() const {
		return hstmt;
	}

private:
	SQLHSTMT	hstmt;
};


class BSQLException : public BException {
public:
	BSQLException(SQLRETURN rc, const char *msg);
	BSQLException(SQLRETURN rc, const char *msg, short handletype, SQLHANDLE handle );
	virtual ~BSQLException();

	virtual const char* message() const;
	virtual const char* name() const;
	virtual Severity severity() const;

	// Specific routines

	virtual SQLRETURN rc() const;
	const char* details() const;

private:
	enum		{msgBufferSize=100};
	SQLRETURN	retcode;
	char		msgBuffer[msgBufferSize];
	String		extmessage;
};

}	// namespace bw




