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
#include "EmotiBitTestingHelper.h"
#include "ofxOsc.h"
#include "PatchboardJson.h"
#include "PatchboardXml.h"
#include "Periodizer.h"
#include "ofxJSON.h"
#include "SoftwareVersionChecker.h"
#include "EmotiBitLsl.h"
#include "AuxInstrQ.h"

class ofApp : public ofBaseApp {
public:
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
	void sendDataSelection(ofAbstractParameter& output);
	void updateDeviceList();
	void processSlowResponseMessage(string message);
	void processSlowResponseMessage(vector<string> splitMessage);
	void processAperiodicData(std::string signalId, std::vector<float> data);
	string ofGetTimestampString(const string& timestampFormat); // Adds %f for microseconds
	void setupGui();
	void setupOscilloscopes();
	void updateLsl();
	void clearOscilloscopes(bool connectedDeviceUpdated);
	void processModePacket(vector<string> &splitPacket);
	void updateMenuButtons();
	void drawConsole();
	void drawOscilloscopes();
	void addDataStream(std::string typetag);
	void removeDataStream(std::string typetag);
	void initMetaDataBuffers();
	void resetScopePlot(int w, int s);
	void setTypeTagPlotAttributes();
	void resetIndexMapping();


	// ToDo: This function is marked to be removed when we complete our move to xmlFileSettings.
	void updatePlotAttributeLists(std::string settingsFile = "ofxOscilloscopeSettings.xml");
	void updateTypeTagList();
	string loadTextFile(string filePath);

	bool startOscOutput();
	bool startUdpOutput();

	// Settings files
	const string commSettingsFile = "emotibitCommSettings.json";
	const string lslOutputSettingsFile = "lslOutputSettings.json";
	const string udpOutputSettingsFile = "udpOutputSettings.xml";
	const string oscOutputSettingsFile = "oscOutputSettings.xml";
	string lslSettings;

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
	float timeWindowOnSetup; // seconds
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
	//struct EmotibitPacketHeader_V1 {
	//	uint32_t timestamp;  // milliseconds since EmotiBit bootup
	//	uint16_t packetCount;
	//	uint16_t dataReliability;  // Reliability of data 0-100, Reliability=0 no data is sent
	//	uint16_t dataLength;  // length of data value array
	//	uint8_t typeTag;
	//	uint8_t protocolVersion
	//}

	struct typeTagPlotAttr {
		std::string plotName;
		ofColor plotColor;
		vector<int> scopeIdx;
	};

	PatchboardXml patchboard;
	// ToDo: change the input aperiodic and ouptut periodic typeTags when we resolve typetags for aperiodic signals
	// NOTE: New periodizers have to be added to the list below
	std::vector<Periodizer> periodizerList{ Periodizer( EmotiBitPacket::TypeTag::HEART_RATE, 
														EmotiBitPacket::TypeTag::PPG_INFRARED,
														EmotiBitPacket::TypeTag::HEART_RATE) ,

											Periodizer( EmotiBitPacket::TypeTag::SKIN_CONDUCTANCE_RESPONSE_AMPLITUDE,
														EmotiBitPacket::TypeTag::EDA,
														EmotiBitPacket::TypeTag::SKIN_CONDUCTANCE_RESPONSE_AMPLITUDE,
														0),

											Periodizer( EmotiBitPacket::TypeTag::SKIN_CONDUCTANCE_RESPONSE_RISE_TIME,
														EmotiBitPacket::TypeTag::EDA,
														EmotiBitPacket::TypeTag::SKIN_CONDUCTANCE_RESPONSE_RISE_TIME,
														0)
										  };
	vector<ofxMultiScope> scopeWins;
	unordered_map<int, vector<size_t>> plotIdIndexes;
	vector<vector<vector<string>>> typeTags;
	unordered_map<string, vector<int>> typeTagIndexes;
	vector<vector<float>> samplingFreqs;
	vector<vector<vector<string>>> plotNames;
	vector<vector<vector<float>>> yLims;
	vector<vector<float>> minYSpans;
	vector<vector<vector<ofColor>>> plotColors;
	//vector<ofColor> plotColors;
	unordered_map<std::string, typeTagPlotAttr>typeTagPlotAttributes;
	vector<vector<vector<int>>> plotIds;

	vector<vector<vector<int>>> bufferSizes;
	vector<vector<vector<int>>> dataCounts;
	vector<vector<vector<float>>> dataFreqs;

	//ofxButton recordingStatus;
	ofParameter<bool> recordingButton;
	ofxLabel recordingStatus;
	ofxLabel batteryStatus; 
	//ofParameter<string> batteryStatus;
	//ofParameter<float> batteryStatus;
	ofxTextField userNote;
	ofxButton sendUserNote;
	ofParameter<int> dataClippingCount;
	ofParameter<int> dataOverflowCount;
	//ofParameterGroup deviceMenuGroup;
	//ofParameter<string> deviceSelected;
	ofxLabel deviceSelected;
	vector<ofParameter<bool>> deviceList;
	ofParameterGroup deviceGroup;
	// ToDo: encapsulate sendData variables to be more portable/usable
	vector<string> sendDataOptions;
	vector<ofParameter<bool>> sendDataList;
	vector<bool> sendDataDisabled;
	ofxLabel sendOptionSelected;
	//ofParameter<string> sendOptionSelected;
	//ofParameterGroup sendDataMenuGroup;
	ofParameterGroup sendDataGroup;
	//ofParameterGroup powerStatusMenuGroup;
	ofParameterGroup powerModeGroup;
	vector<string> powerModeOptions;
	vector<ofParameter<bool>> powerModeList;

	string _recordingFilename = "";

	const string GUI_STRING_NOT_RECORDING = "NOT RECORDING";
	const string GUI_STRING_RECORDING = "RECORDING";
	const string GUI_STRING_MODE_HIBERNATE = "HIBERNATING";
	const string GUI_STRING_MODE_ACTIVE = "ACTIVE";
	const string GUI_STRING_NO_EMOTIBIT_SELECTED = "None Selected";
	//const string GUI_DEVICE_GROUP_MENU_NAME = "Emotibit Device Menu";
	const string GUI_DEVICE_GROUP_NAME = "EmotiBit Device List";
	const string GUI_STRING_CLIPPING_EVENTS = "Clipping Events:";
	const string GUI_STRING_OVERFLOW_EVENTS = "Overflow Events:";
	//const string GUI_SEND_DATA_MENU_NAME = "Send Data Menu";
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
	const string GUI_STRING_BATTERY_LEVEL = "Battery Level";
	const string GUI_STRING_EMOTIBIT_SELECTED = "EmotiBit";
	const string GUI_STRING_EMPTY_USER_NOTE = "[Add a note]";
	//const string GUI_POWER_STATUS_MENU_NAME = "RECORD";
	const string GUI_POWER_MODE_GROUP_NAME = "Power Mode";
	const string GUI_STRING_NORMAL_POWER =	 "Normal         (data streaming)";
	const string GUI_STRING_LOW_POWER =		 "Low Power      (no streaming)";
	const string GUI_STRING_WIRELESS_OFF = "Wireless Off";
	const string GUI_STRING_HIBERNATE = "Sleep";

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
	//ofxColorSlider color;
	//ofxVec2Slider center;
	//ofxIntSlider circleResolution;
	//ofxToggle filled;
	//ofxButton twoCircles;
	//ofxButton ringButton;
	//ofxLabel screenSize;

	vector<ofxPanel> guiPanels = vector<ofxPanel>(6); // OF v0.11.2 requires vector initialization to avoid "attempting to reference a deleted function" error

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
	EmotiBitTestingHelper _testingHelper;
	enum class PowerMode {
		HIBERNATE,
		WIRELESS_OFF,					// fully shutdown wireless
		MAX_LOW_POWER,	// data not sent, time-syncing accuracy low
		LOW_POWER,			// data not sent, time-syncing accuracy high
		NORMAL_POWER,				// data sending, time-syncing accuracy high
		length
	};
	PowerMode _powerMode = PowerMode::LOW_POWER;

	// ToDo: generalize patchboard management
	PatchboardXml oscPatchboard;
	ofxOscSender oscSender;
	bool sendOsc = false; // ToDo: generalize sendOsc to sendData

	PatchboardXml udpPatchboard;
	ofxUDPManager udpSender;
	bool sendUdp = false; // ToDo: generalize sendOsc to sendData
	
	EmotiBitLsl emotibitLsl;
	bool sendLsl = false;
	bool _processAuxCtrl = false;
	AuxInstrQ auxCtrlQ;  ///< Main application queue for aux messages

	void initAuxControl(std::string commSettingsFile);
};

