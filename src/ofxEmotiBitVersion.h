#pragma once
//#include <string>
#include "ofMain.h"


const std::string ofxEmotiBitVersion = "1.7.3";

static const char SOFTWARE_VERSION_PREFIX = 'v';

static void writeOfxEmotiBitVersionFile() {
	string filename = "ofxEmotiBit_Version.txt";
	remove(ofToDataPath(filename).c_str());
	ofstream mFile;
	mFile.open(ofToDataPath(filename).c_str(), ios::out);
	mFile << ofxEmotiBitVersion.c_str();
	mFile.close();
}
