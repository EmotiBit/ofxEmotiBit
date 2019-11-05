#pragma once
//#include <string>
#include "ofMain.h"
const std::string ofxEmotiBitVersion = "0.7.0";

static void writeOfxEmotiBitVersionFile() {
	string filename = "ofxEmotiBit_Version.txt";
	remove(ofToDataPath(filename).c_str());
	ofstream mFile;
	mFile.open(ofToDataPath(filename).c_str(), ios::out);
	mFile << ofxEmotiBitVersion.c_str();
	mFile.close();
}