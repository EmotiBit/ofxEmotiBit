#pragma once
//#include <string>
#include "ofMain.h"
const std::string ofxEmotiBitVersion = "1.4.1.fix-dataParser.7";

static void writeOfxEmotiBitVersionFile() {
	string filename = "ofxEmotiBit_Version.txt";
	remove(ofToDataPath(filename).c_str());
	ofstream mFile;
	mFile.open(ofToDataPath(filename).c_str(), ios::out);
	mFile << ofxEmotiBitVersion.c_str();
	mFile.close();
}
