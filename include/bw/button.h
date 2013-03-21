/* button.h -- Interface for the Button class

Copyright (C) 1997-2013, Brian Bray

*/


/*
#include "bw/figure.h"
*/

namespace bw {

class Button : public Figure, public Focus, public Mouse {
public: // Ctor, Dtor
	Button( const char* pszLabel, const char* pszAction );
	~Button();

public: // Services
	void makeDefault( bool isDefault=true );

public: // Event handlers
	void onQuerySize( LogPos& lposWidth, LogPos& lposHeight );
	void draw( Canvas& cvs );
	void onMouseButton( int nButton, bool isPush );
	void onMouseEnter();
	void onMouseExit();
	void onGotFocus();
	void onLostFocus();

private:
	String	m_strLabel;
	String	m_strAction;
	enum {Inactive, Released, Pushed}
	m_state;
	bool	m_isDefault;
	bool	m_hasFocus;
};


}   // namespace
