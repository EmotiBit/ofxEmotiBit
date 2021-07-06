#include "ofMain.h"
#include "ofApp.h"
#include "ofxLSL.h"
#include "ofAppNoWindow.h"

const char * ofxLSL::resInletInfo = "name = 'CFL'";
//========================================================================
int main( ){
	ofAppNoWindow window;
	ofSetupOpenGL(&window, 1024, 768, OF_WINDOW);
	//ofSetupOpenGL(1500,900,OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
