#pragma once
#include "ofMain.h"

class ThreadedSystemCall :public ofThread{
public:
	std::string cmd;
	std::string targetResponse;
	bool cmdResult;
	std::string systemOutput;

	void setup(std::string cmd, std::string targetResponse = "");
	void threadedFunction();
};