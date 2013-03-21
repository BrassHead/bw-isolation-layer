/* bwsql.cc -- sql isolation layer

Copyright (C) 1999-2013, Brian Bray

*/

#include <sstream>

#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/exception.h"
#include "bw/sql.h"

#include <cstring>
#include <cstdlib>
#include <ctime>
//#include <iodbc.h>
#include <sql.h>
#include <sqlext.h>


namespace bw {

using std::ostringstream;
using std::ends;

SQLHENV SQLConnection::henv = 0;
int SQLConnection::numConnects = 0;

static SQLLEN nts = SQL_NTS;

// Timestamp utility class

#define myTss static_cast<SQL_TIMESTAMP_STRUCT*>(myTs)

SQLTimestamp::SQLTimestamp()
{
	myTs = new SQL_TIMESTAMP_STRUCT;

	time_t tnow = time(0);
	struct tm *now = localtime(&tnow);
	myTss->year = now->tm_year + 1900;
	myTss->month = now->tm_mon + 1;
	myTss->day = now->tm_mday;
	myTss->hour = now->tm_hour;
	myTss->minute = now->tm_min;
	myTss->second = now->tm_sec;
	myTss->fraction = 0;
}


SQLTimestamp::SQLTimestamp
(
    short year,
    unsigned short month,
    unsigned short day,
    unsigned short hour,
    unsigned short minute,
    unsigned short second,
    unsigned int fraction
)
{
	myTs = new SQL_TIMESTAMP_STRUCT;

	myTss->year = year;
	myTss->month = month;
	myTss->day = day;
	myTss->hour = hour;
	myTss->minute = minute;
	myTss->second = second;
	myTss->fraction = fraction;
}

SQLTimestamp::~SQLTimestamp()
{
	delete myTss;
}

short& SQLTimestamp::year()
{
	return myTss->year;
}

unsigned short& SQLTimestamp::month()
{
	return myTss->month;
}

unsigned short& SQLTimestamp::day()
{
	return myTss->day;
}

unsigned short& SQLTimestamp::hour()
{
	return myTss->hour;
}

unsigned short& SQLTimestamp::minute()
{
	return myTss->minute;
}

unsigned short& SQLTimestamp::second()
{
	return myTss->second;
}

unsigned int& SQLTimestamp::fraction()
{
	return myTss->fraction;
}

#define oTss static_cast<SQL_TIMESTAMP_STRUCT*>(sts.myTs)

bool SQLTimestamp::operator<(const SQLTimestamp& sts)
{
	if (myTss->year < oTss->year)
		return true;
	else if (myTss->year > oTss->year)
		return false;

	if (myTss->month < oTss->month)
		return true;
	else if (myTss->month > oTss->month)
		return false;

	if (myTss->day < oTss->day)
		return true;
	else if (myTss->day > oTss->day)
		return false;

	if (myTss->hour < oTss->hour)
		return true;
	else if (myTss->hour > oTss->hour)
		return false;

	if (myTss->minute < oTss->minute)
		return true;
	else if (myTss->minute > oTss->minute)
		return false;

	if (myTss->second < oTss->second)
		return true;
	else if (myTss->second > oTss->second)
		return false;

	if (myTss->fraction < oTss->fraction)
		return true;
	else if (myTss->fraction > oTss->fraction)
		return false;

	return false;
}


// TODO: Documentation

SQLConnection::SQLConnection(
    const char* database,
    const char* userid,
    const char* password )
{
	SQLRETURN rc;

	if (numConnects==0) {
		rc = SQLAllocEnv(&henv);
		if (rc!=SQL_SUCCESS)
			throw BSQLException(rc,"Unable to allocate environment.");
		numConnects = 1;
	} else
		++numConnects;

	rc = SQLAllocConnect(henv, &hdbc);
	if (rc!=SQL_SUCCESS) {
		if (--numConnects==0)
			SQLFreeEnv( henv );
		throw BSQLException(rc,"Unable to allocate connection.", SQL_HANDLE_ENV, henv);
	}

	rc = SQLConnect( hdbc, (UCHAR*)database, SQL_NTS,
	                 (UCHAR*)userid, SQL_NTS,
	                 (UCHAR*)password, SQL_NTS );
	if (rc!=SQL_SUCCESS) {
		BSQLException except(rc,"Database unavailable.",SQL_HANDLE_DBC,hdbc);
		SQLFreeConnect( hdbc );
		if (--numConnects==0)
			SQLFreeEnv( henv );
		throw except;
	}
}


SQLConnection::~SQLConnection()
{
	// Note that errors are not expected here and nothing can be done
	// about them

	SQLDisconnect( hdbc );
	SQLFreeConnect( hdbc );
	if (--numConnects==0)
		SQLFreeEnv( henv );
}


SQLHSTMT SQLConnection::allocStmt() const
{
	SQLRETURN		rc;
	SQLHSTMT	hstmt;

	rc = SQLAllocStmt( hdbc, &hstmt );
	if (rc!=SQL_SUCCESS)
		throw BSQLException(rc,"Unable to allocate statement.",SQL_HANDLE_DBC,hdbc);

	return hstmt;
}


SQLStatement::SQLStatement(const SQLConnection& connect)
{
	hstmt = connect.allocStmt();
}


SQLStatement::~SQLStatement()
{
	SQLFreeStmt( hstmt, SQL_DROP );
}


void SQLStatement::execDirect( const char* stmt )
{
	SQLRETURN rc;

	rc = SQLExecDirect( hstmt, (SQLCHAR*)stmt, SQL_NTS );
	if (rc!=SQL_SUCCESS)
		throw BSQLException(rc, "Direct execution statement failed.",SQL_HANDLE_STMT,hstmt);
}


void SQLStatement::prepare( const char* stmt )
{
	SQLRETURN rc;

	rc = SQLPrepare( hstmt, (SQLCHAR*)stmt, SQL_NTS );
	if (rc!=SQL_SUCCESS)
		throw BSQLException(rc, "SQL statement prepare failed.",SQL_HANDLE_STMT,hstmt);
}


void SQLStatement::execute()
{
	SQLRETURN rc;

	rc = SQLExecute( hstmt );
	if (rc!=SQL_SUCCESS)
		throw BSQLException(rc, "SQL statement execution failed.",SQL_HANDLE_STMT,hstmt);
}


bool SQLStatement::fetch()
{
	// Returns true on success
	SQLRETURN rc;

	rc = SQLFetch( hstmt );
	if (rc==SQL_NO_DATA_FOUND)
		return false;
	if (rc!=SQL_SUCCESS)
		throw BSQLException(rc, "Unexpected fetch error.",SQL_HANDLE_STMT,hstmt);
	return true;
}


void SQLStatement::getData(int column, long* res) const
{
	SQLRETURN rc;

	rc = SQLGetData(hstmt,column,SQL_INTEGER,res,sizeof(long),0);
	if (rc!=SQL_SUCCESS)
		throw BSQLException(rc, "SQL Get Data error.",SQL_HANDLE_STMT,hstmt);
}

void SQLStatement::getData(int column, String& res) const
{
	SQLRETURN rc;
	SQLLEN len;

	rc = SQLColAttributes(hstmt,column,SQL_COLUMN_LENGTH,0,0,0,&len);
	if (rc!=SQL_SUCCESS)
		throw BSQLException(rc, "Unable to determine SQL string size",SQL_HANDLE_STMT,hstmt);
	char* buf = new char[len+1];
	*buf = '\0';		// NULL response becomes NULL string

	rc = SQLGetData(hstmt,column,SQL_C_CHAR,buf,len+1,0);
	if (rc!=SQL_SUCCESS)
		throw BSQLException(rc, "SQL Get Data error.",SQL_HANDLE_STMT,hstmt);

	bwassert(static_cast<unsigned long>(len)>=strlen(buf));

	res = buf;
	delete [] buf;
}


void SQLStatement::getData(int column, char* buf, int buflen) const
{
	SQLRETURN rc;

	rc = SQLGetData(hstmt,column,SQL_C_CHAR,buf,buflen,0);
	if (rc!=SQL_SUCCESS)
		throw BSQLException(rc, "SQL Get Data error.",SQL_HANDLE_STMT,hstmt);
}


void SQLStatement::getData(int column, SQLTimestamp* res) const
{
	SQLRETURN rc;

	rc = SQLGetData(hstmt,column,SQL_C_TIMESTAMP,res->myTs,
	                sizeof(SQL_TIMESTAMP_STRUCT),0);
	if (rc!=SQL_SUCCESS)
		throw BSQLException(rc, "SQL Get Data error.",SQL_HANDLE_STMT,hstmt);
}


void SQLStatement::closeCursor()
{
	SQLRETURN rc;

	rc = SQLFreeStmt(hstmt,SQL_CLOSE);
	if (rc!=SQL_SUCCESS)
		throw BSQLException(rc, "SQL close error.",SQL_HANDLE_STMT,hstmt);
}


void SQLStatement::recycle()
{
	SQLRETURN rc;

	rc = SQLFreeStmt(hstmt,SQL_CLOSE);
	if (rc==SQL_SUCCESS)
		rc = SQLFreeStmt(hstmt,SQL_UNBIND);
	if (rc==SQL_SUCCESS)
		rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);

	if (rc!=SQL_SUCCESS)
		throw BSQLException(rc, "SQL error on statement recycle.",SQL_HANDLE_STMT,hstmt);
}

void SQLStatement::bindParameter(int column, const char* buf)
{
	SQLRETURN rc;

	rc = SQLBindParam( hstmt,
	                   column,
	                   SQL_PARAM_INPUT,
	                   SQL_C_CHAR,
	                   strlen(buf),
	                   0,
	                   const_cast<char*>(buf),
	                   &nts );
	if (rc!=SQL_SUCCESS)
		throw BSQLException(rc, "SQL error on parameter bind.",SQL_HANDLE_STMT,hstmt);
}

void SQLStatement::bindParameter(int column, char* buf, int buflen)
{
	SQLRETURN rc;

	rc = SQLBindParameter( hstmt,
	                       column,
	                       SQL_PARAM_INPUT,
	                       SQL_C_CHAR,
	                       SQL_VARCHAR,
	                       buflen,
	                       0,
	                       buf,
	                       buflen,
	                       &nts );
	if (rc!=SQL_SUCCESS)
		throw BSQLException(rc, "SQL error on parameter bind.",SQL_HANDLE_STMT,hstmt);
}

void SQLStatement::bindParameter(int column, long *pValue)
{
	SQLRETURN rc;

	rc = SQLBindParameter( hstmt,
	                       column,
	                       SQL_PARAM_INPUT,
	                       SQL_C_LONG,
	                       SQL_INTEGER,
	                       0,
	                       0,
	                       pValue,
	                       sizeof(int),
	                       0 );
	if (rc!=SQL_SUCCESS)
		throw BSQLException(rc, "SQL error on parameter bind.",SQL_HANDLE_STMT,hstmt);
}

void SQLStatement::bindParameter(int column, SQLTimestamp* pValue)
{
	SQLRETURN rc;

	rc = SQLBindParameter( hstmt,
	                       column,
	                       SQL_PARAM_INPUT,
	                       SQL_C_TIMESTAMP,
	                       SQL_TIMESTAMP,
	                       0,
	                       0,
	                       pValue->myTs,
	                       sizeof(SQL_TIMESTAMP_STRUCT),
	                       0 );
	if (rc!=SQL_SUCCESS)
		throw BSQLException(rc, "SQL error on parameter bind.",SQL_HANDLE_STMT,hstmt);
}


void SQLStatement::bindColumn(int column, char* buf, int buflen)
{
	SQLRETURN rc;

	rc = SQLBindCol( hstmt, column, SQL_C_CHAR, buf, buflen, 0 );

	if (rc!=SQL_SUCCESS)
		throw BSQLException(rc, "SQL error on result column bind.",SQL_HANDLE_STMT,hstmt);
}

void SQLStatement::bindColumn(int column, long *pValue)
{
	SQLRETURN rc;

	rc = SQLBindCol( hstmt, column, SQL_C_LONG, pValue, sizeof(long), 0 );

	if (rc!=SQL_SUCCESS)
		throw BSQLException(rc, "SQL error on result column bind.",SQL_HANDLE_STMT,hstmt);
}

void SQLStatement::bindColumn(int column, SQLTimestamp* pValue)
{
	SQLRETURN rc;

	rc = SQLBindCol( hstmt, column, SQL_C_TIMESTAMP, pValue->myTs, sizeof(SQL_TIMESTAMP_STRUCT), 0 );

	if (rc!=SQL_SUCCESS)
		throw BSQLException(rc, "SQL error on result column bind.",SQL_HANDLE_STMT,hstmt);
}


BSQLException::BSQLException(SQLRETURN rc, const char *msg)
	: retcode(rc)
{
	strncpy(msgBuffer,msg,msgBufferSize);
	msgBuffer[msgBufferSize-1] = '\0';
}

BSQLException::BSQLException(SQLRETURN rc, const char* msg, short hndtype, SQLHANDLE hnd )
	: retcode(rc)
{
	strncpy(msgBuffer,msg,msgBufferSize);
	msgBuffer[msgBufferSize-1] = '\0';

	String& ext = extmessage;

	switch (rc) {
	case SQL_SUCCESS_WITH_INFO:
		ext = "SQL_SUCCESS_WITH_INFO\n";
		break;

	case SQL_NO_DATA_FOUND:
		ext = "SQL_NO_DATA_FOUND\n";
		break;

	case SQL_ERROR:
		ext = "SQL_ERROR\n";
		break;

	case SQL_INVALID_HANDLE:
		ext = "SQL_INVALID_HANDLE\n";
		break;

	case SQL_SUCCESS:
		ext = "SQL_SUCCESS\n";
		break;

	default: {
		ext = "UNKNOWN ERROR RC=";
		ostringstream ost;
		ost << rc << "\n" << ends;
		ext += ost.str().c_str();
	}
	}

	const int ERRBUFSIZE = 400;
	char errmsg[ERRBUFSIZE];
	short retlen;

	SQLINTEGER nmsg;
	SQLRETURN rc2 = SQLGetDiagField( hndtype,
	                                 hnd,
	                                 0,
	                                 SQL_DIAG_NUMBER,
	                                 &nmsg,
	                                 sizeof(SQLINTEGER),
	                                 &retlen );
	bwassert(rc2 == SQL_SUCCESS);
	for (int i=1; i<=nmsg; ++i) {
		SQLRETURN rc2 = SQLGetDiagField( hndtype,
		                                 hnd,
		                                 i,
		                                 SQL_DIAG_MESSAGE_TEXT,
		                                 errmsg,
		                                 ERRBUFSIZE,
		                                 &retlen );
		bwassert(rc2 == SQL_SUCCESS);
		ext += errmsg;
	}
}


BSQLException::~BSQLException()
{
}

const char* BSQLException::message() const
{
	return msgBuffer;
}

const char* BSQLException::name() const
{
	return "SQLError";
}

BException::Severity BSQLException::severity() const
{
	return Error;
}

SQLRETURN BSQLException::rc() const
{
	return retcode;
}

const char* BSQLException::details() const
{
	return extmessage.c_str();
}

}	// namespace bw

