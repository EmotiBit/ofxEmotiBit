#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main(int argc, char *argv[]){
	ofSetupOpenGL(1500,900,OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:

	//passes the command line arguments to the app
	ofApp *app = new ofApp();
	if (argc != 1) {
		app->argFileName = argv[1];
		app->cmdLineStart = true;
	}
	ofRunApp(app);

}
