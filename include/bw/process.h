/* process.h -- interface for the Process object (X version)

Copyright (C) 1997-2013, Brian Bray

*/

// These must be statically declared in client code.
// Note: not in namespace 'bw'
//
// TODO: Derive class from name and move to GUI section

extern const char *BWAppName;
extern const char *BWAppClass;
extern const char *BWLocale;


namespace bw {

class BException;

class Process {
public:	// static functions for getting process level data
	static int getArgc();
	static char** getArgv();
	static const char* getLocale() {
		return m_locale;
	}
	static const char* getAppName() {
		return BWAppName;
	}
	static const char* getAppClass() {
		return BWAppClass;
	}

	enum state {staticConstruction=0, initializing, running, finishing, staticDestruction};
	static state getState();

protected:	// Overidable functions
	virtual int main(int argc, char** argv);
	// Source for the default main is provided here for reference
	//{
	//	String strAction;
	//	Scene TheScene;
	//	TheScene.addFrame( Main::getMain() );
	//	while ( !strAction.startsWith("exit(") )
	//	    strAction = TheScene.nextAction();
	//	return atoi(strAction.substring(5));
	//}
	virtual void fatalError(const BException&) {}

protected:	// Accessible only by main
	Process();
	virtual ~Process();

private:
	friend class	FProcess;
	static Process*	m_TheProcess;
	static int		m_argc;
	static char**	m_argv;
	static char*	m_locale;
	static state	m_fState;
};

//
// Helper routine
//
const char* getHomeDir();

}	//namespace bw

