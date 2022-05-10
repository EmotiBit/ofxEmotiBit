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

using namespace EmotiBit;

class EmotiBitStatus
{
public:
	EmotiBitStatus(bool isAvailable = false, uint64_t lastSeen = ofGetElapsedTimeMillis()) :
		isAvailable(isAvailable), lastSeen(lastSeen) {}
	bool isAvailable;
	uint64_t lastSeen;
};

class EmotiBitWiFiHost
{
public:

	struct HostAdvertisingSettings {
		bool enableBroadcast = true;
		bool enableUnicast = false;

		pair<int, int> unicastIpRange = { 2, 254 };

		vector<string> networkIncludeList = { "*.*.*.*" };
		vector<string> networkExcludeList = { "" };
	} _hostAdvSettings;

	static const uint8_t SUCCESS = 0;
	static const uint8_t FAIL = -1;

	uint16_t advertisingInterval = 500; // Milliseconds between sending advertising messages
	uint16_t startCxnInterval = 100;

	ofxUDPManager advertisingCxn;
	ofxUDPManager dataCxn;
	ofxTCPServer controlCxn;

	std::thread* dataThread;

	std::mutex controlCxnMutex;
	std::mutex dataCxnMutex;
	std::mutex emotibitIpsMutex;

	uint16_t advertisingPort;
	uint16_t _dataPort;
	uint16_t sendDataPort;
	uint16_t controlPort;

	vector<string> availableNetworks; // All available networks, with or without emotibits
	vector<string> emotibitNetworks; // Networks that contain emotibits
	bool enableBroadcast = false; 
	uint64_t advertizingTimer;

	unordered_map<string, EmotiBitStatus> _emotibitIps;	// list of EmotiBit IP addresses
	string connectedEmotibitIp;
	bool _isConnected;
	bool isStartingConnection;
	uint16_t startCxnTimeout = 5000;
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
	uint16_t receivedDataPacketNumber = 60000;	// Tracks packet numbers (for multi-send). Starts at arbitrary large number.

	~EmotiBitWiFiHost();
	int8_t begin();
	void getAvailableNetworks();
	void pingAvailableNetworks();
	void updateAdvertisingIpList(string ip); 
	int8_t processAdvertising(vector<string> &infoPackets);
	int8_t connect(string ip);
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
	int8_t flushData();
	//int8_t sendUdp(WiFiUDP& udp, const String& message, const IPAddress& ip, const uint16_t& port);
	unordered_map<string, EmotiBitStatus> getEmotiBitIPs();	// <IP address, availability to connect>
	vector<string> getLocalIPs();
	//string createPacket(string typeTag, string data = "", uint16_t dataLength = 0, uint8_t protocolVersion = 1, uint8_t dataReliability = 100);
	//string createPacket(string typeTag, vector<string> data, uint8_t protocolVersion = 1, uint8_t dataReliability = 100);
	bool isConnected();
	int8_t _startDataCxn(uint16_t dataPort);
	bool isInNetworkIncludeList(string ipAddress);
	bool isInNetworkExcludeList(string ipAddress);
	bool isInNetworkList(string ipAddress, vector<string> networkList);

	void setAdvertTransSettings(bool enableBroadcast, bool enableUnicast, pair<int, int> unicastIpRange);
	void setNetworkIncludeList(vector<string> networkIncludeList);
	void setNetworkExcludeList(vector<string> networkExcludeList);
	void setHostAdvertisingSettings(HostAdvertisingSettings settings);
	HostAdvertisingSettings getHostAdvertisingSettings();

	string ofGetTimestampString(const string& timestampFormat); // Adds %f for microseconds
};
