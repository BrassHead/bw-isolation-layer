// Test app for bwiso layer
//

#include <sstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <list>
#include <map>

#include <bw/bwassert.h>
#include <bw/string.h>
#include <bw/countable.h>
#include <bw/bwiso.h>
#include <bw/exception.h>
//#include <bw/custom.h>
//#include <bw/process.h>
#include <clocale>

using std::cout;
using std::cerr;
using std::endl;
using std::min;
using std::max;
using std::hex;
using std::dec;

const char *BWAppName = "testhello";
const char *BWAppClass = "TestHello";
const char *XBWLocale = "";


namespace bw {
class Figure
{};
};

static bool done = false;
static bw::GCHand gc;
static bw::FontHand helv;

void doWinProc(bw::Figure*, bw::EventRef evr)
{
	cout << "Event: " << evr->name() << " received." << endl;
	switch (evr->type()) {
	case bw::EVCloseRequest:
		done = true;
		break;

	case bw::EVExpose:
		gc.drawString(helv,10,40,"Hello out there!");
		break;
	default:
		break;
	}
}

void fatalError(bw::DisplayHand)
{
	cout << "Fatal Error... terminating" << endl;
	exit(1);
}

void errorProc(bw::DisplayHand, bw::EventRef evr)
{
	cout << "Error... " << evr->name() << ". Continuing" << endl;
}

int main(int argc, char* argv[])
{
	try {
		// Set locale
		XBWLocale = setlocale(LC_ALL, "");
		if (!XBWLocale)
			throw "Locale not supported by operating system.";
		if (!bw::DisplayHand::localeIsSet())
			throw "Locale not supported by windowing system.";

		// Open display connection
		bw::DisplayHand dh;
		dh.open(argc,argv);
		dh.setWinProc(doWinProc);
		dh.setErrorHandler(errorProc);
		dh.setFatalErrorHandler(fatalError);

		// Some checks and reports
		bwassert( dh.isValid() );
		cout << "       Plane Mask = " << hex << dh.planeMask() << dec << "\n";
		cout << " Number of planes = " << dh.numPlanes() << "\n";
		cout << "      Screen name = \"" << dh.screenName() << "\"\n";
		cout << "Screen size (wxh) = " << dh.screenWidth() << " x " << dh.screenHeight() << "\n";
		cout << " Pixels per meter = " << dh.assumedPixelsPerMeter() << "\n";
		cout << "  Pixels per inch = " << dh.assumedPixelsPerInch() << "\n";
		cout << "Screen size in MM = " << dh.assumedScreenWidthMM() << " x " << dh.assumedScreenHeightMM() << "\n";
		cout << "    Colormap size = " << dh.colormapSize() << "\n";

		// Create window
		bw::EventMask em = (bw::EventMask)(bw::EMKeyPress | bw::EMExposure | bw::EMFocus | bw::EMButtonPress);
		bw::WinHand wh = dh.newFrameWindow(
		                     em,
		                     0,
		                     "Test Window",
		                     0, 0, 320, 240);

		// Show window
		wh.setBackground(0);
		wh.show();
		wh.takeFocus();

		// get font and graphics context
		bw::FontSpec fs("clean 24");
		helv=dh.newFont(fs);
		bw::GCSpec gcs;
		gcs.foreground()=-1;
		gcs.background()=0;
		gc = wh.newGC(gcs);

		// Font list
		bw::FontSpec allfonts;
		std::set<bw::FontSpec> fl = dh.findFonts(allfonts);
		std::set<bw::FontSpec>::iterator iter;
		char fname[200];
		for (iter=fl.begin(); iter!=fl.end(); ++iter) {
			(*iter).getName(fname,200);
			cout << fname << "\n";
		}


		// Event Loop
		bw::EventRef evr;
		while(!done) {
			evr = dh.nextEvent();
			dh.dispatchEvent(evr);
		}

		// Clean up
		bwassert( dh.isValid() );
		dh.close();
	} catch (const bw::BException& e) {
		cerr << "Unexpected error: " << e.message() << endl;
	} catch (const char* psz) {
		cerr << "Error: " << psz << endl;
	} catch (const std::exception& e) {
		cerr << "Unexpected C++ exception: " << e.what() << endl;
	} catch (...) {
		cerr << "Unexpected error of unknown type." << endl;
	}

	return 1;
}



