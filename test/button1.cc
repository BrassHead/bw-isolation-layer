/* test1.cpp  Button test 1

Copyright (C) 1998-2013, Brian Bray

*/

#include "bw/bwassert.h"
#include "bw/string.h"
#include "bw/process.h"
#include "bw/figure.h"
#include "bw/tools.h"
#include "bw/button.h"

using namespace bw;

const char *BWAppName = "test1";
const char *BWAppClass = "Test1";

class Buttons : public Figure, Frame, Parent, Main {
public:
	Buttons()
		: m_btn11( "Button", "exit(0);" ),
		  m_btn21( "Button", ";" ),
		  m_btn31( "Button", ";" ),
		  m_btn41( "Button", ";" ),
		  m_btn51( "Button", ";" ),
		  m_btn12( "Button", ";" ),
		  m_btn13( "Button", ";" ),
		  m_btn14( "Button", ";" ),
		  m_btn15( "Button", ";" ),
		  m_btn22( "Button", ";" ),
		  m_btn33( "Button", ";" ),
		  m_btn44( "Button", ";" ),
		  m_btn55( "Button", ";" ) {
		addChild( &m_btn11, 10, 30, AskChild, AskChild );
		m_btn11.makeDefault();
		addChild( &m_btn21, 120, 30, 50, AskChild );
		addChild( &m_btn31, 190, 30, 20, AskChild );
		addChild( &m_btn41, 220, 30, 10, AskChild );
		addChild( &m_btn51, 240, 30, 1, AskChild );
		addChild( &m_btn12, 10, 70, AskChild, 20 );
		addChild( &m_btn13, 10, 100, AskChild, 10 );
		addChild( &m_btn14, 10, 120, AskChild, 5 );
		addChild( &m_btn15, 10, 130, AskChild, 1 );
		addChild( &m_btn22, 120, 70, 40, 20 );
		addChild( &m_btn33, 190, 100, 20, 10 );
		addChild( &m_btn44, 220, 120, 10, 5 );
		addChild( &m_btn55, 240, 130, 1, 1 );
	}
	void draw(Canvas& c) {
		Press f(c);
		f.drawText(10,20,"Push first button to exit");
	}
	Button m_btn11;
	Button m_btn21;
	Button m_btn31;
	Button m_btn41;
	Button m_btn51;
	Button m_btn12;
	Button m_btn13;
	Button m_btn14;
	Button m_btn15;
	Button m_btn22;
	Button m_btn33;
	Button m_btn44;
	Button m_btn55;
} mymain;





