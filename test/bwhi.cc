/* hi.cc  Hello world in GUI BW

Copyright (C) 1997-2013, Brian Bray

*/

//
// This is a trail of the simplest (to implement) hello world
//

#include <bw/bwassert.h>
#include <bw/string.h>
#include <bw/process.h>
#include <bw/figure.h>
#include <bw/tools.h>
#include <bw/button.h>

using namespace bw;

const char *BWAppName = "hi";
const char *BWAppClass = "Hi";

/*Resource resGeneral[] = {
    { "AppName",	"hi"},
    { "AppClass",	"Hi"} };

ResourceList TheResources[] = {
    {"*",	"*", 	resGeneral},
    {"",	"",	0} };
*/

class HelloWin : public Figure, Frame, Parent, Main {
public:
	HelloWin()
		: m_btn( "Done", "exit(0);" ) {
		addChild( &m_btn, MidWidth, FullHeight-50, AskChild, AskChild );
	}
	void draw(Canvas& c) {
		Press f(c);
		f.drawText(10,20,"Go ahead...push me around!");
	}
	Button m_btn;
} mymain;

/* int BWMain( int argc, char* argv[] )
{
    String strAction;
    Scene TheScene;
    HelloWin myMain;

    TheScene.addFrame( myMain );

    while ( !strAction.startsWith("exit(") )
	strAction = TheScene.nextAction();

    return atoi(strAction.substring(5));
}
*/







