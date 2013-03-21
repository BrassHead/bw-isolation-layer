/* scene.h - Interface for Scene figure class

Copyright (C) 1997-2013, Brian Bray

*/

/* Needs:
#include <bw/figure.h>
*/

namespace bw {

class Scene;
class String;

class Atoms;

class Scene : private virtual FigureBase, public Figure, public Parent {
public:
	Scene(String strScreen="", int msecFlashRate=0);
	~Scene() throw();

	const String& getScreen();
	const String& nextAction();
	void addFrame( Figure* pfig, LogPos lposWidth=MidWidth, LogPos lposHeight=MidHeight );
	void addFrame( Figure* pfig, LogPos lposX, LogPos lposY,
	               LogPos lposWidth, LogPos lposHeight );
	void removeFrame( Figure* pfig );
	void setFlashRate( int msec );
	void dispatch( const Event& ev, EventRet& evr );
	void doDraw();
	virtual void onResizeRequest( Figure* fig, LogPos lposWidth, LogPos lposHeight );
	virtual void onMoveRequest( Figure* fig, LogPos lposX, LogPos lposY );
	virtual void onRelocateRequest( Figure* fig, LogPos lposX, LogPos lposY,
	                                LogPos lposWidth, LogPos lposHeight );

private:
	friend class FScene;
	bool onAction( const String& strCommand );
	void draw( Canvas& cvs );
	void messagePump();
	void absLoc(Figure*,LogPos,LogPos,LogPos,LogPos,int&,int&,int&,int&);

	void*	m_pDisplay;
	int		m_iScreen;
	String	m_strDisplayName;
	String	m_strCommand;
	bool	m_haveInvalidRegions;
	Atoms*	m_patoms;
};

}	//Namespace bw
