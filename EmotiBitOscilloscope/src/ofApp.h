#pragma once

#include "ofMain.h"
#include "ofxOscilloscope.h"
//#include "ofxNetwork.h"
//#include "ofxNetworkUtils.h"
#include "ofxThreadedLogger.h"
#include "ofxGui.h"
#include "ofxInputField.h"
#include "ofxLSL.h"
#include "DoubleBuffer.h"
#include "EmotiBitPacket.h"
#include "EmotiBitWiFiHost.h"
#include "ofxEmotiBitVersion.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void exit();

	ofxLSL lsl;
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	//void parsePacket(string dataString);
	//void parseUdp();
	//void parseSerial();
	//vector<string> getLocalIPs();
	//void sendEmotiBitPacket(string typeTag, string data = "", uint16_t dataLength = 1, uint16_t protocolVersion = 1, uint16_t dataReliability = 100);
	//void sendEmotiBitPacket(ofxUDPManager &udpManager, string typeTag, string data = "", uint16_t dataLength = 1, uint16_t protocolVersion = 1, uint16_t dataReliability = 100);
	//void parseIncomingAck(vector<string> splitPacket);
	//void parseIncomingRequestData(EmotiBitPacket::Header header, vector<string> splitPacket);

	void recordButtonPressed(bool & recording);
	void sendExperimenterNoteButton();
	template <class T>
	vector<vector<vector<T>>> initBuffer(vector<vector<vector<T>>> buffer);
	float smoother(float smoothData, float newData, float newDataWeight);
	void deviceGroupSelection(ofAbstractParameter& device);
	void powerModeSelection(ofAbstractParameter& mode);
	void sendDataSelection(bool & selected);
	void updateDeviceList();
	void processSlowResponseMessage(string message);
	void processSlowResponseMessage(vector<string> splitMessage);
	string ofGetTimestampString(const string& timestampFormat); // Adds %f for microseconds
	void setupGui();
	void setupOscilloscopes();
	void updateLsl();
	void clearOscilloscopes();
	void processModePacket(vector<string> &splitPacket);
	void updateMenuButtons();
	void drawConsole();
	void drawOscilloscopes();
	void printTestingData(vector<string> splitPacket, EmotiBitPacket::Header packetHeader);

	//ofxMultiScope scopeWin;
	//ofxMultiScope scopeWin2;
	//int newPoints;
	//int nScopes;
	//std::vector<std::vector<float> > gyroData;
	//std::vector<std::vector<float> > accData;
	//std::vector<std::vector<float> > magData;
	//std::vector<std::vector<float> > gsrData;
	//std::vector<std::vector<float> > ppgIRData;
	//std::vector<std::vector<float> > ppgRData;
	//std::vector<std::vector<float> > ppgGData;
	//std::vector<std::vector<float> > tempData;
	//std::vector<std::vector<float> > thermData;
	//std::vector<std::vector<float> > humData;
	int counter;
	int counter2;
	ofPoint min;
	ofPoint max;

	int selectedScope;

	bool isPaused;
	ofSerial mySerial;

	string stringData;

	ofxUDPManager udpConnection;
	LoggerThread dataLogger;
	LoggerThread consoleLogger;
	bool logData;
	bool logConsole;
	ofTrueTypeFont legendFont;
	ofTrueTypeFont axesFont;
	ofTrueTypeFont subLegendFont;

	EmotiBitWiFiHost emotiBitWiFi;
	unordered_map<string, EmotiBitStatus> emotibitIps;

	//struct EmotibitPacketHeader_V1 {
	//	uint32_t timestamp;  // milliseconds since EmotiBit bootup
	//	uint16_t packetCount;
	//	uint16_t dataReliability;  // Reliability of data 0-100, Reliability=0 no data is sent
	//	uint16_t dataLength;  // length of data value array
	//	uint8_t typeTag;
	//	uint8_t protocolVersion
	//}

	vector<ofxMultiScope> scopeWins;
	vector<vector<vector<string>>> typeTags;
	unordered_map<string, vector<int>> typeTagIndexes;
	vector<vector<float>> samplingFreqs;
	vector<vector<vector<string>>> plotNames;
	vector<vector<vector<float>>> yLims;
	vector<vector<vector<ofColor>>> plotColors;
	//vector<ofColor> plotColors;

	vector<vector<vector<int>>> bufferSizes;
	vector<vector<vector<int>>> dataCounts;
	vector<vector<vector<float>>> dataFreqs;

	//ofxButton recordingStatus;
	ofParameter<bool> recordingButton;
	ofxLabel recordingStatus;
	ofParameter<bool> hibernateButton;
	ofxLabel hibernateStatus;
	//ofxLabel batteryStatus; 
	ofParameter<string> batteryStatus;
	ofxLabel sdCardStatus;
	//ofParameter<float> batteryStatus;
	//ofParameter<float> sdCardStatus;
	ofxTextField userNote;
	ofxButton sendUserNote;
	ofParameter<int> dataClippingCount;
	ofParameter<int> dataOverflowCount;
	ofParameterGroup deviceMenuGroup;
	ofParameter<string> deviceSelected;
	//ofxLabel currentDevice;
	vector<ofParameter<bool>> deviceList;
	ofParameterGroup deviceGroup;
	vector<string> sendDataOptions;
	vector<ofParameter<bool>> sendDataList;
	//ofxLabel currentSendOption;
	ofParameter<string> sendOptionSelected;
	ofParameterGroup sendDataMenuGroup;
	ofParameterGroup sendDataGroup;
	ofParameterGroup powerStatusMenuGroup;
	ofParameterGroup powerModeGroup;
	vector<string> powerModeOptions;
	vector<ofParameter<bool>> powerModeList;

	const string GUI_STRING_NOT_RECORDING = "NOT RECORDING";
	const string GUI_STRING_RECORDING = "RECORDING";
	const string GUI_STRING_MODE_HIBERNATE = "HIBERNATING";
	const string GUI_STRING_MODE_ACTIVE = "ACTIVE";
	const string GUI_STRING_NO_EMOTIBIT_SELECTED = "NONE SELECTED";
	const string GUI_DEVICE_GROUP_MENU_NAME = "Emotibit Device Menu";
	const string GUI_DEVICE_GROUP_NAME = "EmotiBit Device List";
	const string GUI_STRING_CLIPPING_EVENTS = "Clipping Events:";
	const string GUI_STRING_OVERFLOW_EVENTS = "Overflow Events:";
	const string GUI_SEND_DATA_MENU_NAME = "Send Data Menu";
	const string GUI_STRING_SEND_DATA_VIA = "Send data via";
	const string GUI_OUTPUT_GROUP_NAME = "Output List";
	const string GUI_STRING_SEND_DATA_NONE = "None";
	const string GUI_STRING_SEND_DATA_LSL = "LSL";
	const string GUI_STRING_SEND_DATA_OSC = "OSC";
	const string GUI_STRING_SEND_DATA_MQTT = "MQTT";
	const string GUI_STRING_SEND_DATA_TCP = "TCP";
	const string GUI_STRING_SEND_DATA_UDP = "UDP";
	const string GUI_STRING_NOTE_BUTTON = "LOG NOTE";
	const string GUI_STRING_CONTROL_RECORD = "RECORD";
	const string GUI_STRING_CONTROL_HIBERNATE = "HIBERNATE";

	const string GUI_POWER_STATUS_MENU_NAME = "RECORD";
	const string GUI_POWER_MODE_GROUP_NAME = "Power Mode";
	const string GUI_STRING_NORMAL_POWER =	"Normal            (data streaming)";
	const string GUI_STRING_LOW_POWER =			"Low Power      (no streaming)";
	const string GUI_STRING_WIRELESS_OFF = "Wireless Off";
	const string GUI_STRING_HIBERNATE = "Hibernate";

	ofColor recordControlColor = ofColor(255, 69, 78);
	ofColor hibernateControlColor = ofColor(10, 135, 210);
	ofColor noteControlColor = ofColor(1, 204, 115);
	ofColor deviceAvailableColor = ofColor(255, 255, 255);
	ofColor notAvailableColor = ofColor(128, 128, 128);

	int guiPanelDevice;
	int guiPanelRecord;
	//int guiPanelMode;
	//int guiPanelLevels;
	int guiPanelPowerStatus;
	int guiPanelErrors;
	int guiPanelUserNote;
	int guiPanelSendData;

	int _consoleHeight;

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

	bool plotUdpData = true;
	bool DEBUGGING = false;

	int drawYTranslate = 0;
	float drawYScale = 1.f;
	uint32_t malformedMessages = 0;
	uint32_t bufferUnderruns = 0;
	uint16_t MAX_BUFFER_LENGTH = 64;
	size_t messageLen = 0;

	//DoubleBuffer<string> messageBuffer; 
	//ofMutex connectionLock;
	//std::thread* connectionThread;
	//bool runConnectionThread = true;

	//int connectionPort;
	bool drawDataInfo = false;

	int nDataClippingEvents = 0;
	int nDataOverflowEvents = 0;

	bool _recording = false;
	bool testingMode = false;
	enum class PowerMode {
		HIBERNATE,
		WIRELESS_OFF,					// fully shutdown wireless
		MAX_LOW_POWER,	// data not sent, time-syncing accuracy low
		LOW_POWER,			// data not sent, time-syncing accuracy high
		NORMAL_POWER,				// data sending, time-syncing accuracy high
		length
	};
	PowerMode _powerMode = PowerMode::LOW_POWER;

};
