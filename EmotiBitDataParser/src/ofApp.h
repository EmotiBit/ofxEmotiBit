#pragma once

#include "ofMain.h"
#include "ofxOscilloscope.h"
#include "ofxNetwork.h"
#include "ofxNetworkUtils.h"
#include "ofxThreadedLogger.h"
#include "ofxGui.h"
#include "ofxInputField.h"
#include "EmotiBitPacket.h"
#include "ofxEmotiBitVersion.h"

class ofApp : public ofBaseApp {
public:

	string argFileName;
	string stringData;
	bool cmdLineStart = false;

	ofTrueTypeFont legendFont;
	ofTrueTypeFont subLegendFont;

	//struct EmotibitPacketHeader_V1 {
	//	uint32_t timestamp;  // milliseconds since EmotiBit bootup
	//	uint16_t packetCount;
	//	uint16_t dataReliability;  // Reliability of data 0-100, Reliability=0 no data is sent
	//	uint16_t dataLength;  // length of data value array
	//	uint8_t typeTag;
	//	uint8_t protocolVersion
	//}

	//ofxButton recordingStatus;
	ofParameter<bool> processButton;
	ofxLabel processStatus;
	ofxTextField inputPath;
	ofxTextField outputPath;

	ifstream inFile;
	ofstream outFile;

	const string GUI_STATUS_IDLE = "IDLE";
	const string GUI_STATUS_PROCESSING = "PROCESSING";
	const string GUI_PANEL_LOAD_FILE = "<- Click here to load EmotiBit data file";
	const string READ_TIMESTAMP_FORMAT = "%Y-%m-%d_%H-%M-%S-%i";
	const string WRITE_TIMESTAMP_FORMAT = "%Y/%m/%d %H:%M:%S.%i";

	//ofxFloatSlider batteryStatus;
	//ofxFloatSlider sdCardStatus;
	//ofxColorSlider color;
	//ofxVec2Slider center;
	//ofxIntSlider circleResolution;
	//ofxToggle filled;
	//ofxButton twoCircles;
	//ofxButton ringButton;
	//ofxLabel screenSize;

  vector<ofxPanel> guiPanels;

	unordered_map<string, uint32_t> timestamps;
	unordered_map<string, LoggerThread*> loggers;

	char packetDelimiter;

	uint32_t lineCounter;
	uint32_t nLinesInFile;

	string dataLine;
	//string inFilePath;
	string inFileDir;
	string inFileBase;
	string fileExt;

	int linesPerLoop;

	int drawYTranslate = 0;
	float drawYScale = 1.f;

	enum class State {
		WARNING_INSUFFICIENT_TIMESYNCS = -1,
		IDLE = 0,
		PARSING_TIMESTAMPS = 1,
		PARSING_DATA = 2,
		length = 3
	};

	State currentState = State::IDLE;

	struct TimestampData {
		uint32_t RD = 0;
		string TS_sent = "";
		uint32_t TS_received = 0;
		uint32_t AK = 0;
		int32_t roundTrip = -1;
	};

	vector<TimestampData> allTimestampData;
	
	uint32_t malformedMessages = 0;
	uint32_t bufferUnderruns = 0;
	uint16_t MAX_BUFFER_LENGTH = 64;
	size_t messageLen = 0;

	bool parseLsl = true;
	struct LslTimestampData {
		string localTime = "";
		long double lslTime = 0;
	};
	string timestampFilenameString = "timesyncs";
	std::vector<LslTimestampData> allLslTimestampData;
	struct TimeSyncMap {
		long double e0 = 0;
		long double e1 = 0;
		long double c0 = 0;
		long double c1 = 1;
		long double l0 = 0;
		long double l1 = 0;
	} timeSyncMap;

	std::string timesyncsWarning = "WARNING: Data file was parsed with less than 2 time-sync events, which can reduce the timestamp accuracy.\n"
		"\nEmotiBit periodically generates time-sync events while a connection is established with the EmotiBit Oscilloscope software.\n"
		"At a minimum, it's recommended to keep EmotiBit connected to the EmotiBit Oscilloscope software\n"
		"for at least one minute after starting data recording AND re-establish connection with EmotiBit Oscilloscope\n"
		"software (using the same computer on which recording was started) for at least one minute before stopping data recording.\n"
		"\nTo further improve timestamp accuracy, it's optimal to keep EmotiBit connected to the EmotiBit Oscilloscope software\n"
		"throughout recording to generate many time-sync events in the data file.\n";

	struct RecordedDataTimeRange {
		long double emotibitStartTime = INT_MAX;
		long double emotibitEndTime = 0;
	}recordedDataTimeRange;

	int eofCounter = 0;

	void setup();
	void update();
	void draw();
	void exit();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	void parseDataLine(string dataString);
	void parseUdp();
	void parseSerial();
	vector<string> getLocalIPs();
	void sendEmotiBitPacket(string typeTag, string data = "", uint16_t dataLength = 1, uint16_t protocolVersion = 1, uint16_t dataReliability = 100);

	void recordButtonPressed(bool & recording);
	void sendExperimenterNoteButton();
	template <class T>
	vector<vector<vector<T>>> initBuffer(vector<vector<vector<T>>> buffer);
	float smoother(float smoothData, float newData, float newDataWeight);
	void startProcessing(bool & processing);
	long double getLocalUnixTime(std::string timestamp);
	/*!
			@brief returns the index of the shortest roundtrip
			@param rtIndexes vector<pair<roundtripTime, index>>
			@return index of shortest roundtrip
	*/
	int getShortestRtIndex(vector<pair<int, int>> rtIndexes);

	/*!
		@brief returns the best 2 available TimestampData points for timesync map
		@param vector of all TimestampData
		@return pair<earlier, later> TimestampData points
	*/
	pair<ofApp::TimestampData, ofApp::TimestampData> getBestTimestampIndexes(const vector<TimestampData> &timestampData);
	TimeSyncMap calculateTimeSyncMap(vector<TimestampData> &timestampData);
	std::time_t getEpochTime(const std::wstring& dateTime);
	double GetMedian(double daArray[], int iSize);
	bool timestampDataCompare(pair<int, TimestampData> i, pair<int, TimestampData> j);
	long double linterp(long double x, long double x0, long double x1, long double y0, long double y1);
};
