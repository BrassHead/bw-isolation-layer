/* exception.h -- Exception class

Copyright (C) 1997-2013, Brian Bray

*/

namespace bw {

class BException {
public:
	enum Severity {
		Minor=1,
		Warning=2,
		Error=3,
		Severe=4,
		Fatal=5
	};

	BException() {};
	// Purpose:	 construct an exception
	// nothrows

	virtual ~BException() {}
	// Purpose:	Destroy an exception

	virtual const char* message() const =0;
	// Purpose:	 returns descriptive error message

	virtual const char* name() const =0;
	// Purpose:	returns error name

	virtual Severity severity() const =0;

protected:
	// BExceptions cannot be copied or assigned (Because it would slice off derived class data)
	BException& operator=( const BException& ) {
		return *this;
	}
	// Note that the copy constructor cannot be made protected in thrown classes,
	// so users need to take care not to cause copies of the base class.
};


// Sample usage
//
//	class TestException : public BException
//	{
//	public:
//		TestException() {}
//		~TestException() {}
//		const char* message() const {return "Test Exception";}
//		const char* name() const {return "Test";}
//		Severity severity() const {return Error;}
//	};
//
//	...
//		throw TestException();
//

// Builtin Exceptions

class BMemoryException : public BException {
public:
	BMemoryException();
	virtual ~BMemoryException();
	virtual const char* message() const;
	virtual const char* name() const;
	virtual Severity severity() const;
};

class BFileException : public BException {
public:
	enum FileErrorCode {
		NoError=0,
		UnexpectedEof=1,
		SystemError=2,
		FileNotFound=3
	};

	BFileException( FileErrorCode errnum );
	virtual ~BFileException();
	virtual const char* message() const;
	virtual const char* name() const;
	virtual Severity severity() const;

private:
	enum { msgBufferSize=100 };
	FileErrorCode	errorCode_;
	int			errno_;
	char		messageBuffer_[msgBufferSize];
};

class BFormatException : public BException {
public:
	BFormatException( const char* msg );
	virtual ~BFormatException();
	virtual const char* message() const;
	virtual const char* name() const;
	virtual Severity severity() const;

private:
	enum { msgBufferSize=100 };
	char		messageBuffer_[msgBufferSize];
};

class BGUIException : public BException {
public:
	enum GUIErrorCode {
		NoError=0,
		NoMainWindow=1,
		SystemError=2,
		ConnectionLost=3,
		NoDisplay=4
	};

	BGUIException( GUIErrorCode errnum, const char* psz=0 );
	virtual ~BGUIException();
	virtual const char* message() const;
	virtual const char* name() const;
	virtual Severity severity() const;

private:
	enum { msgBufferSize=100 };
	GUIErrorCode	m_errorCode;
	char		m_messageBuffer[msgBufferSize];
};

class BSystemError : public BException {
public:
	BSystemError( const char* psz=0 );
	virtual ~BSystemError();
	virtual const char* message() const;
	virtual const char* name() const;
	virtual Severity severity() const;

private:
	enum { msgBufferSize=100 };
	char		m_messageBuffer[msgBufferSize];
};

class BDebugException : public BException {
public:
	BDebugException( const char* psz=0 );
	virtual ~BDebugException();
	virtual const char* message() const;
	virtual const char* name() const;
	virtual Severity severity() const;

private:
	enum { msgBufferSize=100 };
	char		m_messageBuffer[msgBufferSize];
};


}	// namespace bw
