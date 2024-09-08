/// EmotiBitWifiHost
///
/// Supports WiFi communications with the EmotiBit 
///
/// Written by produceconsumerobot Dec 2019


#pragma once

#include <unordered_set>
#include <atomic>

// ToDo: Extend code to work with Android
#include "ofMain.h"
#include "ofxNetwork.h"
#include "EmotiBitPacket.h"
#include "EmotiBitComms.h"
#include "DoubleBuffer.h"
#include "json/json.h"
#include "AuxCxnController.h"
#include "ofUtils.h"
#include "EmotiBitOfUtils.h"
using namespace EmotiBit;

class EmotibitInfo
{
public:
	EmotibitInfo(string ip = "", bool isAvailable = false, uint64_t lastSeen = ofGetElapsedTimeMillis()) :
		ip(ip), isAvailable(isAvailable), lastSeen(lastSeen) {}
	string ip;
	bool isAvailable;
	uint64_t lastSeen;
	// Additional parameters like Name, Fs etc can be stored in this struct
	// ToDo: Consider if a copy/assignment constructor is needed
};

class EmotiBitWiFiHost
{
public:

	struct WifiHostSettings {
		int sendAdvertisingInterval = 1000; // msec interval between advertising blasts
		int checkAdvertisingInterval = 100; // msec interval between checks for advertising replies
		int advertisingThreadSleep = 0;	// usec duration to sleep between thread loops
		int dataThreadSleep = 0;	// usec duration to sleep between thread loops

		bool enableBroadcast = true;

		bool enableUnicast = true;
		pair<int, int> unicastIpRange = { 2, 254 };
		int nUnicastIpsPerLoop = 1;
		int unicastMinLoopDelay = 3;

		vector<string> networkIncludeList = { "*.*.*.*" };
		vector<string> networkExcludeList = { "" };
	} _wifiHostSettings;

	static const uint8_t SUCCESS = 0;
	static const uint8_t FAIL = -1;

	uint16_t startCxnInterval = 100;

	ofxUDPManager advertisingCxn;
	ofxUDPManager dataCxn;
	ofxTCPServer controlCxn;
	AuxCxnController auxNetworkChannelController;

	std::thread* dataThread;
	std::thread* advertisingThread;

	std::mutex controlCxnMutex;
	std::mutex dataCxnMutex;
	std::mutex discoveredEmotibitsMutex;

	uint16_t advertisingPort;
	uint16_t _dataPort;
	uint16_t sendDataPort;
	uint16_t controlPort;

	vector<string> availableNetworks; // All available networks, with or without emotibits
	vector<string> emotibitNetworks; // Networks that contain emotibits
	bool enableBroadcast = false; 
	uint64_t advertizingTimer;

	unordered_map<string, EmotibitInfo> _discoveredEmotibits;	// list of EmotiBit IP addresses
	string connectedEmotibitIp;
	// ToDo: Find a scalable solution to store connected EmotiBit details.
	// Ex. If we want to change the selected emotibit name to be displayed, instead of ID
	string connectedEmotibitIdentifier;  //!< stores the ID of the connected EmotiBit 
	bool _isConnected;
	bool isStartingConnection;
	uint16_t startCxnTimeout = 5000;	// milliseconds
	uint64_t startCxnAbortTimer;

	uint16_t advertisingPacketCounter = 0;
	uint16_t controlPacketCounter = 0;
	uint16_t dataPacketCounter = 0;

	DoubleBuffer<string> dataPackets;

	uint16_t pingInterval = 500;
	uint64_t connectionTimer;
	uint16_t connectionTimeout = 10000;
	uint16_t availabilityTimeout = 5000;
	uint16_t ipPurgeTimeout = 15000;

  atomic_bool stopDataThread = {false};
	atomic_bool stopAdvertisingThread = { false };
	uint16_t receivedDataPacketNumber = 60000;	// Tracks packet numbers (for multi-send). Starts at arbitrary large number.

	~EmotiBitWiFiHost();
	int8_t begin();
	void getAvailableNetworks();
	void sendAdvertising();
	void updateAdvertisingIpList(string ip); 
	int8_t processAdvertising(vector<string> &infoPackets);
	int8_t connect(string deviceId);
	int8_t connect(uint8_t i);
	int8_t disconnect();
	int8_t sendControl(const string& packet);
	//uint8_t readControl(string& packet);
	void readUdp(ofxUDPManager &udp, string& message, string ipFilter = "");	// If ipFilter is empty it reports the result
	void readData(vector<string> &packets);
	void updateData();
	int8_t sendData(const string& packet);
	void processRequestData(const string& packet, int16_t dataStartChar);
	void updateDataThread();
	void processAdvertisingThread();
	int8_t flushData();
	//int8_t sendUdp(WiFiUDP& udp, const String& message, const IPAddress& ip, const uint16_t& port);
	unordered_map<string, EmotibitInfo> getdiscoveredEmotibits();	// <device ID, device Information>
	vector<string> getLocalIPs();
	//string createPacket(string typeTag, string data = "", uint16_t dataLength = 0, uint8_t protocolVersion = 1, uint8_t dataReliability = 100);
	//string createPacket(string typeTag, vector<string> data, uint8_t protocolVersion = 1, uint8_t dataReliability = 100);
	bool isConnected();
	int8_t _startDataCxn(uint16_t dataPort);

	/*!
			@brief returns whether the passed ipAddress is in the network includeList
			@param ipAddress
			@return true if ipAddress is in the includeList
	*/
	bool isInNetworkIncludeList(string ipAddress);

	/*!
		@brief returns whether the passed ipAddress is in the network excludeList
		@param ipAddress
		@return true if ipAddress is in the excludeList
	*/
	bool isInNetworkExcludeList(string ipAddress);

	/*!
	@brief returns whether the passed ipAddress is in passed network list
	@param ipAddress to check
	@param networkList of ipAddresses to check ipAddress against
	@return true if ipAddress is in networkList
	*/
	bool isInNetworkList(string ipAddress, vector<string> networkList);

	/*!
	@brief Parses EmotiBit host comm settings
	@param jsonStr comm settings in JSON format
	*/
	void parseCommSettings(string jsonStr);

	/*!
	@brief Sets EmotiBit host advertising settings
	@param settings
	*/
	void setWifiHostSettings(WifiHostSettings settings);

	/*!
	@brief Gets EmotiBit host advertising settings
	@return settings
	*/
	WifiHostSettings getWifiHostSettings();

	string ofGetTimestampString(const string& timestampFormat); // Adds %f for microseconds

	/*!
	@brief Handles sleeping or yeilding the the active thread
	@param sleepMicros <0 does nothing, ==0 yeilds, >0 sleep_for
	*/
	void threadSleepFor(int sleepMicros);

	/**
	 * \brief Read messages received on the aux. network channel.
	 * 
	 */
	void readAuxNetworkChannel();

	/**
	 * \brief Attach reference to the main application to the aux network controller.
	 * 
	 * \param q pointer to the main application queue
	 * \return true if successful, else false
	 */
	bool attachAppQ(AuxInstrQ* q);

	/**
	 * \brief move packets from aux network controller buffer queue to main application queue.
	 * 
	 */
	void updateAuxInstrQ();

	/**
	 * \brief Process the packets received on the auc channel.  All instructions that require network communication are processed by wifi host.
	 * 
	 * \param q Pointer to the main application queue
	 */
	void processAppQ();
};


