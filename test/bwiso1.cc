// Test app for bwiso layer
//
// Todo: This is crashing because of 2 bw::Figure classes

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

class Figure {
public:
	virtual void handleEvent(bw::EventRef evr) =0;
};

};

static bool done = false;
static bw::GCHand gc;
static bw::GCHand gcBtn1;
static bw::FontHand helv;
static bw::DisplayHand dh;
static bw::PixmapHand pmh;
static bw::PixmapHand pmhlit;

struct VTTab {
	bw::VisualType	vt;
	const char*	vtName;
};

VTTab	vttab[] = {
	{ bw::PrintOnly, "PrintOnly" },
	{ bw::Monochrome, "Monochrome" },
	{ bw::Grayscale, "Grayscale" },
	{ bw::FixedPalette, "FixedPalette" },
	{ bw::DynamicPalette, "DynamicPalette" },
	{ bw::PrivatePalette, "PrivatePalette" },
	{ bw::TrueColour, "TrueColour" }
};
const int lenvttab = 7;

const char* prVisType(bw::VisualType vt)
{
	for (int i=0; i<lenvttab; ++i) {
		if (vt==vttab[i].vt)
			return vttab[i].vtName;
	}
	return "Unknown";
}


class MainFigure : public bw::Figure {
public:
	virtual void handleEvent(bw::EventRef evr);
	void clockUpdate();
};

class Btn1Figure : public bw::Figure {
public:
	virtual void handleEvent(bw::EventRef evr);
	Btn1Figure() : mouseIn(false) {};
protected:
	bool mouseIn;
};

void doWinProc(bw::Figure* pfig, bw::EventRef evr)
{
	cout << "Event: " << evr->name() << " received." << endl;
	pfig->handleEvent(evr);
}

void MainFigure::handleEvent(bw::EventRef evr)
{
	switch (evr->type()) {
	case bw::EVCloseRequest:
		done = true;
		break;

	case bw::EVExpose:
		gc.drawString(helv,10,40,"Hello out there!");
		clockUpdate();
		break;
	default:
		break;
	}
}

void MainFigure::clockUpdate()
{
	static int tmr=0;

	std::ostringstream ostr;
	++tmr;
	ostr << tmr/60 << ":" << std::setfill('0') << std::setw(2) << tmr%60 << std::ends;
	gc.fillString(helv,10,20,ostr.str().c_str());
}

void Btn1Figure::handleEvent(bw::EventRef evr)
{
	switch (evr->type()) {
	case bw::EVExpose:
		if (mouseIn)
			gcBtn1.copy(0,0,pmhlit,0,0,100,20);
		else
			gcBtn1.copy(0,0,pmh,0,0,100,20);
		break;

	case bw::EVEnterNotify:
		mouseIn = true;
		gcBtn1.copy(0,0,pmhlit,0,0,100,20);
		break;

	case bw::EVLeaveNotify:
		mouseIn = false;
		gcBtn1.copy(0,0,pmh,0,0,100,20);
		break;

	case bw::EVButtonPress:
		if (evr->button()==bw::leftButton) {
			// Font list
			bw::FontSpec allfonts;
			std::set<bw::FontSpec> fl = dh.findFonts(allfonts);
			std::set<bw::FontSpec>::iterator iter;
			char fname[200];
			for (iter=fl.begin(); iter!=fl.end(); ++iter) {
				(*iter).getName(fname,200);
				cout << fname << "\n";
			}
		}

	default:
		break;
	}
}

void centerText(bw::GCHand& gch, const bw::FontHand& font, int midx, int midy, const char* psz)
{
	bw::TextExtent te;
	font.getTextExtent(psz,te);
	int x = midx - (te.width/2);
	int y = midy - (te.height/2) + te.descent;
	gch.drawString(font,x,y,psz);
}

bw::Colour lighten(const bw::Colour& orig)
{
	return bw::Colour( min(65535,orig.red*5/4),
	                   min(65535,orig.green*5/4),
	                   min(65535,orig.blue*5/4) );
}

bw::Colour darken(const bw::Colour& orig)
{
	return bw::Colour( max(0,orig.red*4/5),
	                   max(0,orig.green*4/5),
	                   max(0,orig.blue*4/5) );
}


bw::PixmapHand drawButton(int w, int h, const bw::FontHand& font, const bw::Colour baseColour, const char* label)
{
	bw::PhysicalPalette& pyp = dh.getScreenPalette();
	bw::PixmapHand pmh = dh.newPixmap(100,20);
	bw::GCSpec gcs;

	gcs.foreground() = pyp.lookup(baseColour);
	bw::GCHand pmgc = pmh.newGC(gcs);
	pmgc.fillRectangle(0,0,w,h);

	gcs.foreground() = pyp.lookup(darken(baseColour));
	pmgc.change(gcs);
	pmgc.drawLine(1,h-1,w,h-1);
	pmgc.drawLine(w-1,h-1,w-1,0);

	gcs.foreground() = pyp.lookup(lighten(baseColour));
	pmgc.change(gcs);
	pmgc.drawLine(0,0,w,0);
	pmgc.drawLine(0,h-1,0,0);

	gcs.foreground() = pyp.lookup(bw::Colour(0,0,0));
	pmgc.change(gcs);
	centerText(pmgc,font,w/2,h/2,label);

	return pmh;
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
		cout << "Visual type of root = " << prVisType(dh.typeOfVisual()) << "\n";

		// Physical Palette information
		bw::PhysicalPalette& pyp = dh.getScreenPalette();
		bw::Pixel pxRed = pyp.lookup(bw::Colour(65535,0,0));
		bw::Pixel pxGreen = pyp.lookup(bw::Colour(0,65535,0));
		bw::Pixel pxBlue = pyp.lookup(bw::Colour(0,0,65535));
		bw::Pixel pxBlack = pyp.lookup(bw::Colour(0,0,0));
		bw::Pixel pxWhite = pyp.lookup(bw::Colour(65535,65535,65535));

		cout << "        Visual type = " << prVisType(pyp.visualType()) << "\n";
		cout << "        Pixel range = " << pyp.pixelRange() << "\n";
		cout << "  Number of colours = " << pyp.numColours() << "\n";
		bw::Colour c = pyp[0];
		cout << "           Pixel[0] = [" << c.red << "," << c.green << "," << c.blue << "]\n";
		c = pyp[1];
		cout << "           Pixel[1] = [" << c.red << "," << c.green << "," << c.blue << "]\n";
		c = pyp[255];
		cout << "         Pixel[255] = [" << c.red << "," << c.green << "," << c.blue << "]\n";
		cout << "                Red = Pixel[" << hex << pxRed << dec << "]\n";
		cout << "              Green = Pixel[" << hex << pxGreen << dec << "]\n";
		cout << "               Blue = Pixel[" << hex << pxBlue << dec << "]\n";
		cout << "Allocated Off white = Pixel[" << hex << pyp.allocate(bw::Colour(65279,65279,65279)) << dec << "]\n";
		cout << "        Black Pixel = Pixel[" << hex << pxBlack << dec << "]\n";
		cout << "        White Pixel = Pixel[" << hex << pxWhite << dec << "]\n";

		// Root window playground
		dh.rootWindow().setCursor(dh.newCursor(bw::Finger));

		// Create main window
		MainFigure mfig;
		bw::EventMask em = (bw::EventMask)(bw::EMKeyPress | bw::EMExposure | bw::EMFocus | bw::EMButtonPress );
		bw::WinHand wh = dh.newFrameWindow(
		                     em,
		                     &mfig,
		                     "Test Window",
		                     0, 0, 320, 240);
		wh.setBackground(0);

		// and a button window
		Btn1Figure b1fig;
		em = (bw::EventMask)(bw::EMKeyPress | bw::EMExposure | bw::EMFocus | bw::EMButtonPress | bw::EMEnterWindow | bw::EMLeaveWindow );
		bw::WinHand btn1 = wh.newChild(em,&b1fig,40,60,100,20);
		btn1.setBackground(pxWhite);
		btn1.show();

		// get font and graphics context
		bw::FontSpec fs("bitstream charter 14");
		helv=dh.newFont(fs);
		// TODO: Need a way to verify that a font was found!
		bw::GCSpec gcs;
		gcs.foreground()=-1;
		gcs.background()=0;
		gc = wh.newGC(gcs);
		gcBtn1 = btn1.newGC(gcs);

		// Show window
		wh.show();
		wh.takeFocus();
		dh.flush();

		// Now, draw a button or two
		pmh = drawButton(100,20,helv,bw::Colour(0xa4a4,0xb6b6,0xeeee),"Font list");
		pmhlit = drawButton(100,20,helv,bw::Colour(0xcdcd,0xe4e4,0xffff),"Font list");
		dh.sync();

		// Clock startup
		dh.addTimer(1000);

		// Try sending an event
		// TODO: Need an AppEvent class

		// Event Loop
		bw::EventRef evr;
		while(!done) {
			if (dh.countEventsPending()>2)
				cout << "More than two events pending\n";

			evr = dh.nextEvent();

			if (evr->type()==bw::EVAlarm)
				mfig.clockUpdate();
			else
				dh.dispatchEvent(evr);
		}

		// Clean up
		dh.rootWindow().setCursor(dh.newCursor(bw::DefaultCursor));
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

/*

  Remains to test:

  DisplayHand::newBitMap
  DisplayHand::sendEvent
  DisplayHand::setTextClipboard
  DisplayHand::getTextClipboard
  DisplayHand::undoTextClipboard


*/
