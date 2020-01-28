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
	float _thermopile;
	
	bool testingOn = false;
	bool edaTestStarted = false;
	bool arduinoConnected = false;
	uint64_t edlChangeTimer;
	uint64_t edrChangeTimer;
	vector<int> _edlTestPins = { 6, 7, 8, 9 };
	vector<int> _edrTestPins = { 10, 11, 12, 13 };
	vector<vector<bool>> _edlPinStates = 
	{ 
		{0, 0, 0, 0}, 
		{1, 0, 0, 0}, 
		{0, 1, 0, 0}, 
		{0, 0, 1, 0}, 
		{0, 0, 0, 1} 
	};

	vector<vector<bool>> _edrPinStates =
	{
		{0, 0, 0, 0},
		{1, 0, 0, 0},
		{0, 1, 0, 0},
		{1, 1, 0, 0},
		{0, 0, 1, 0},
		{1, 0, 1, 0},
		{0, 1, 1, 0},
		{1, 1, 1, 0},
		{0, 0, 0, 1},
		{1, 0, 0, 1},
		{0, 1, 0, 1},
		{1, 1, 0, 1},
		{0, 0, 1, 1},
		{1, 0, 1, 1},
		{0, 1, 1, 1},
		{1, 1, 1, 1},
		{0, 1, 1, 1},
		{1, 0, 1, 1},
		{0, 0, 1, 1},
		{1, 1, 0, 1},
		{0, 1, 0, 1},
		{1, 0, 0, 1},
		{0, 0, 0, 1},
		{1, 1, 1, 0},
		{0, 1, 1, 0},
		{1, 0, 1, 0},
		{0, 0, 1, 0},
		{1, 1, 0, 0},
		{0, 1, 0, 0},
		{1, 0, 0, 0}
	};

	ofArduino	arduino;
	ofSerial serial;

	struct Results
	{
		string serialNumber;
		string sdCardFilename;
		vector<float> edl;
		vector<float> edr;
		vector<float> edrP2P;
		vector<float> thermopile;
		float ppgRed;
		float ppgIR;
		float ppgGreen;
		string testStatus = "";
	};

	Results _results;
	LoggerThread _testingResultsLog;

	EmotiBitTestingHelper();
	void setLogFilename(const string &filename);
	void update(const vector<string> &splitPacket, const EmotiBitPacket::Header &packetHeader);
	void updateSerialNumber(const string &serialNumber);
	void updateTestStatus(const string &userNote);
	void updateSdCardFilename(const string &filename);
	void updateEda(const vector<string> &splitPacket, const EmotiBitPacket::Header &packetHeader);
	void updatePpg(const vector<string> &splitPacket, const EmotiBitPacket::Header &packetHeader);
	void updateThermopile(const vector<string> &splitPacket, const EmotiBitPacket::Header &packetHeader);
	void printResults();
	void pushEdlEdrResult();
	void pushEdrP2pResult();
	void recordPpgResult();
	void clearEdaResults();
	void clearPpgResults();
	void popEdlEdrResult();
	void popEdrP2pResult();
	void pushThermopileResult();
	void popThermopileResult();
	void clearAllResults();
	void connectArduino(int i);
	void startEdaTest();
	void updateEdaTestOutput();
	void setEdlPins(int state);
	void setEdrPins(int state);
	
};