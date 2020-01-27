/// EmotiBitTestingHelper
///
/// Supports EmotiBit Testing
///
/// Written by produceconsumerobot Jan 2020

#pragma once

#include "EmotiBitPacket.h"
#include "ofxBiquadFilter.h"
#include "ofxThreadedLogger.h"
#include "ofMain.h"

class EmotiBitTestingHelper
{
public:
	float _edl;
	float _edr;
	float _edrFiltP2P;
	float _ppgRed;
	float _ppgIR;
	float _ppgGreen;
	string _serialNumber;
	bool testingOn = false;

	struct Results
	{
		vector<float> edl;
		vector<float> edr;
		vector<float> edrP2P;
		float ppgRed;
		float ppgIR;
		float ppgGreen;
	};

	Results _results;
	LoggerThread _testingResultsLog;

	EmotiBitTestingHelper();
	void setLogFilename(const string &filename);
	void update(const vector<string> &splitPacket, const EmotiBitPacket::Header &packetHeader);
	void updateSerialNumber(const string &serialNumber);
	void updateEda(const vector<string> &splitPacket, const EmotiBitPacket::Header &packetHeader);
	void updatePpg(const vector<string> &splitPacket, const EmotiBitPacket::Header &packetHeader);
	void printResults();
	void pushEdlEdrResult();
	void pushEdrP2pResult();
	void recordPpgResult();
	void clearEdaResults();
	void clearPpgResults();
	void popEdlEdrResult();
	void popEdrP2pResult();
};