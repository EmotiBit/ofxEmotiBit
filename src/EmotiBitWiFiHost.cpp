#include "EmotiBitWiFiHost.h"


EmotiBitWiFiHost::~EmotiBitWiFiHost()
{
	stopDataThread = true;
	dataThread->join();
	delete(dataThread);

	stopAdvertisingThread = true;
	advertisingThread->join();
	delete(advertisingThread);
}

int8_t EmotiBitWiFiHost::begin()
{
	advertisingPort = EmotiBitComms::WIFI_ADVERTISING_PORT;
	getAvailableNetworks();
	if (availableNetworks.size() == 0) {
		ofLogNotice() << "check if network adapters are enabled";
		return FAIL;
	}
	advertisingCxn.Create();
	advertisingCxn.SetNonBlocking(true);
	advertisingCxn.SetReceiveBufferSize(pow(2, 10));

	_startDataCxn(EmotiBitComms::WIFI_ADVERTISING_PORT + 1);

	controlPort = _dataPort + 1;
	controlCxn.setMessageDelimiter(ofToString(EmotiBitPacket::PACKET_DELIMITER_CSV));
	while (!controlCxn.setup(controlPort))
	{
		// Try to setup a controlPort until we find one that's available
		controlPort += 2;
		controlCxn.close();
		ofLogNotice() << "Trying control port: " << controlPort;
	}

	ofLogNotice() << "EmotiBit data port: " << _dataPort;
	ofLogNotice() << "EmotiBit control port: " << controlPort;

	advertisingPacketCounter = 0;
	controlPacketCounter = 0;
	connectedEmotibitIp = "";
	_isConnected = false;
	isStartingConnection = false;
	

	dataThread = new std::thread(&EmotiBitWiFiHost::updateDataThread, this); 
	advertisingThread = new std::thread(&EmotiBitWiFiHost::processAdvertisingThread, this);
	auxNetworkChannelController.begin();
	return SUCCESS;
}

bool EmotiBitWiFiHost::isInNetworkList(string ipAddress, vector<string> networkList) {
	bool out = false;
	vector<string> ipSplit = ofSplitString(ipAddress, ".");
	for (string listIp : networkList) {
		vector<string> listIpSplit = ofSplitString(listIp, ".");
		bool partMatch = true;
		for (uint8_t n = 0; n < ipSplit.size() && n < listIpSplit.size(); n++) {
			if (listIpSplit.at(n).compare("*") == 0 || listIpSplit.at(n).compare(ipSplit.at(n)) == 0) {
				// partial match
				bool breakpoint = true;
			}
			else {
				partMatch = false;
				break;
			}
		}
		if (partMatch == true) {
			// found a match!
			out = true;
			break;
		}
	}
	return out;
}

bool EmotiBitWiFiHost::isInNetworkExcludeList(string ipAddress) {
	bool out = isInNetworkList(ipAddress, _wifiHostSettings.networkExcludeList);
	//cout << "Exclude " << ipAddress << ".* : " << out << endl;
	return out;
}

bool EmotiBitWiFiHost::isInNetworkIncludeList(string ipAddress) {
	bool out =  isInNetworkList(ipAddress, _wifiHostSettings.networkIncludeList);
	//cout << "Include " << ipAddress << ".* : " << out << endl;
	return out;
}


void EmotiBitWiFiHost::getAvailableNetworks() {
	vector<string> ips;
	auto currentavailableNetworks = availableNetworks;
	const int NUM_TRIES_GET_IP = 10;
	int tries = 0;
	
	
	while (ips.size() <= 0 && tries < NUM_TRIES_GET_IP)
	{
		ips = getLocalIPs();
		tries++;
	}
	if (ips.size() > 0) {
		//get all available Networks
		for (int network = 0; network < ips.size(); network++)
		{
			vector<string> ipSplit = ofSplitString(ips.at(network), ".");
			string tempNetwork = ipSplit.at(0) + "." + ipSplit.at(1) + "." + ipSplit.at(2);
			if (ofFind(availableNetworks, tempNetwork) == availableNetworks.size()
				&& isInNetworkIncludeList(tempNetwork) 
				&& !isInNetworkExcludeList(tempNetwork)) {
					availableNetworks.push_back(tempNetwork);
			}
		}
	}
	if (availableNetworks.size() != currentavailableNetworks.size()) { //print all Networks whenever new Networks are detected
		string allAvailableNetworks;
		for (int network = 0; network < availableNetworks.size(); network++) {
			allAvailableNetworks += "[" + availableNetworks.at(network) + ".*] ";
		}
		ofLogNotice() << "All Network(s): " << allAvailableNetworks;
	}
}

void EmotiBitWiFiHost::sendAdvertising() {
	static bool emotibitsFound = false;
	static bool startNewSend = true;
	static bool sendInProgress = true;
	static int unicastNetwork = 0;
	static int broadcastNetwork = 0;
	static int hostId = _wifiHostSettings.unicastIpRange.first; 

	static uint64_t sendAdvertisingTimer = ofGetElapsedTimeMillis();
	uint64_t sendAdvertisingTime = ofGetElapsedTimeMillis() - sendAdvertisingTimer;
	if (sendAdvertisingTime >= _wifiHostSettings.sendAdvertisingInterval)
	{
		// Periodically start a new advertising send
		sendAdvertisingTimer = ofGetElapsedTimeMillis();
		startNewSend = true;
		sendInProgress = true;
	}

	if (emotibitNetworks.size() > 0)
	{
		// only search all networks until an EmotiBit is found
		// ToDo: consider permitting EmotiBits on multiple networks
		emotibitsFound = true;
	}

	if (!emotibitsFound && startNewSend)
	{
		getAvailableNetworks(); // Check if new network appeared after oscilloscope was open (e.g. a mobile hotspot)
	}

	// **** Handle advertising sends ****
	// Handle broadcast advertising
	if (_wifiHostSettings.enableBroadcast && startNewSend) {
		string broadcastIp;

		if (emotibitsFound)
		{
			broadcastIp = emotibitNetworks.at(0) + "." + ofToString(255);
		}
		else
		{
			broadcastIp = availableNetworks.at(broadcastNetwork) + "." + ofToString(255);
		}
		ofLog(OF_LOG_VERBOSE) << "Sending advertising broadcast: " << sendAdvertisingTime;
		ofLog(OF_LOG_VERBOSE) << broadcastIp;
		startNewSend = false;
		string packet = EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::HELLO_EMOTIBIT, advertisingPacketCounter++, "", 0);
		advertisingCxn.SetEnableBroadcast(true);
		advertisingCxn.Connect(broadcastIp.c_str(), advertisingPort);
		advertisingCxn.Send(packet.c_str(), packet.length());

		if (!emotibitsFound)
		{
			broadcastNetwork++;
			if (broadcastNetwork >= availableNetworks.size())
			{
				broadcastNetwork = 0;
			}
		}

		// skip unicast when broadcast sent to avoid network spam
		return;
	}
	// Handle unicast advertising
	if (_wifiHostSettings.enableUnicast && sendInProgress)
	{
		static uint64_t unicastLoopTimer = ofGetElapsedTimeMillis();
		uint64_t unicastLoopTime = ofGetElapsedTimeMillis() - unicastLoopTimer;
		// Limit the rate of unicast sending
		if (unicastLoopTime >= _wifiHostSettings.unicastMinLoopDelay)
		{
			unicastLoopTimer = ofGetElapsedTimeMillis();
			ofLog(OF_LOG_VERBOSE) << "Sending advertising unicast: " << unicastLoopTime;

			for (uint32_t i = 0; i < _wifiHostSettings.nUnicastIpsPerLoop; i++)
			{
				string unicastIp;

				if (emotibitsFound)
				{
					unicastIp = emotibitNetworks.at(0) + "." + ofToString(hostId);
				}
				else
				{
					unicastIp = availableNetworks.at(unicastNetwork) + "." + ofToString(hostId);
				}

				if (_wifiHostSettings.enableUnicast && sendInProgress)
				{
					ofLog(OF_LOG_VERBOSE) << unicastIp;
					string packet = EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::HELLO_EMOTIBIT, advertisingPacketCounter++, "", 0);
					advertisingCxn.SetEnableBroadcast(false);
					advertisingCxn.Connect(unicastIp.c_str(), advertisingPort);
					advertisingCxn.Send(packet.c_str(), packet.length());
				}

				// Iterate IP Address
				if (hostId < _wifiHostSettings.unicastIpRange.second)
				{
					hostId++;
				}
				else
				{
					// Reached end of unicastIpRange
					hostId = _wifiHostSettings.unicastIpRange.first; // loop hostId back to beginning of range
					if (emotibitsFound)
					{
						// finished a send of all IPs
						sendInProgress = false;
						break;
					}
					else
					{
						unicastNetwork++;
						if (unicastNetwork >= availableNetworks.size())
						{
							// reached end of unicastIpRange for the last known network in list
							sendInProgress = false;
							unicastNetwork = 0;
							break;
						}
					}
				}
			}
		}
	}
}

void EmotiBitWiFiHost::updateAdvertisingIpList(string ip) {
	auto currentEmotibitNetworks = emotibitNetworks;
	vector<string> ipSplit = ofSplitString(ip, ".");
	string networkAddr = ipSplit.at(0) + "." + ipSplit.at(1) + "." + ipSplit.at(2);

	if (emotibitNetworks.size() == 0) { //assume emotibits are all on the same network
		emotibitNetworks.push_back(networkAddr);
	}

	//print all emotibit ip adrresses and/or Networks whenever new emotibits are detected
	if (emotibitNetworks.size() != currentEmotibitNetworks.size()) {
		string allEmotibitNetworks;
		for (int network = 0; network < emotibitNetworks.size(); network++) {
			allEmotibitNetworks += "[" + emotibitNetworks.at(network) + ".*] ";
		}
		ofLogNotice() << "Emotibit Network(s): " << allEmotibitNetworks;
	}
}

void EmotiBitWiFiHost::processAdvertisingThread()
{
	while (!stopAdvertisingThread)
	{
		vector<string> infoPackets;
		processAdvertising(infoPackets);
		// ToDo: Handle info packets with mode change information
		threadSleepFor(_wifiHostSettings.advertisingThreadSleep);
	}
}

int8_t EmotiBitWiFiHost::processAdvertising(vector<string> &infoPackets)
{
	const int maxSize = 32768;
	commSettingsUpdateMutex.lock();
	sendAdvertising();
	commSettingsUpdateMutex.unlock();

	static uint64_t checkAdvertisingTimer = ofGetElapsedTimeMillis();
	uint64_t checkAdvertisingTime = ofGetElapsedTimeMillis() - checkAdvertisingTimer;
	if (checkAdvertisingTime >= _wifiHostSettings.checkAdvertisingInterval)
	{
		checkAdvertisingTimer = ofGetElapsedTimeMillis();
		ofLog(OF_LOG_VERBOSE) << "checkAdvertising: " << checkAdvertisingTime;

		// Receive advertising messages
		static char udpMessage[maxSize];
		int msgSize = advertisingCxn.Receive(udpMessage, maxSize);
		if (msgSize > 0)
		{
			string message = udpMessage;
			ofLogVerbose() << "Received: " << message;

			int port;
			string ip;
			advertisingCxn.GetRemoteAddr(ip, port);

			vector<string> packets = ofSplitString(message, ofToString(EmotiBitPacket::PACKET_DELIMITER_CSV));
			for (string packet : packets)
			{
				EmotiBitPacket::Header header;
				int16_t dataStartChar = EmotiBitPacket::getHeader(packet, header);
				if (dataStartChar != 0)
				{
					if (header.typeTag.compare(EmotiBitPacket::TypeTag::HELLO_HOST) == 0)
					{
						// HELLO_HOST
						string value;
						string emotibitDeviceId = "";
						int16_t valuePos = EmotiBitPacket::getPacketKeyedValue(packet, EmotiBitPacket::PayloadLabel::DATA_PORT, value, dataStartChar);
						if (valuePos > -1)
						{
							updateAdvertisingIpList(ip);
							ofLogVerbose() << "EmotiBit ip: " << ip << ":" << port;
							int16_t deviceIdPos = -1;
							deviceIdPos = EmotiBitPacket::getPacketKeyedValue(packet, EmotiBitPacket::PayloadLabel::DEVICE_ID, emotibitDeviceId, dataStartChar);
							if (deviceIdPos > -1)
							{
								// found EmotiBitSrNum in HELLO_HOST message
								// do nothing. emotibitDeviceid already updated.
								ofLogVerbose() << "EmotiBit DeviceId: " << emotibitDeviceId;
							}
							else
							{
								emotibitDeviceId = ip;
								ofLogVerbose() << "EmotiBit DeviceId: " << "DeviceId not available. using IP address as identifier";
								// Add ip address to our list
							}
							discoveredEmotibitsMutex.lock();
							auto it = _discoveredEmotibits.emplace(emotibitDeviceId, EmotibitInfo(ip, ofToInt(value) == EmotiBitComms::EMOTIBIT_AVAILABLE));
							if (!it.second)
							{
								// if it's not a new ip address, update the status
								it.first->second = EmotibitInfo(ip, ofToInt(value) == EmotiBitComms::EMOTIBIT_AVAILABLE);
							}
							discoveredEmotibitsMutex.unlock();
						}
					}
					else if (header.typeTag.compare(EmotiBitPacket::TypeTag::PONG) == 0)
					{
						// PONG
						if (ip.compare(connectedEmotibitIp) == 0)
						{
							string value;
							int16_t valuePos = EmotiBitPacket::getPacketKeyedValue(packet, EmotiBitPacket::PayloadLabel::DATA_PORT, value, dataStartChar);
							if (valuePos > -1 && ofToInt(value) == _dataPort)
							{
								// Establish / maintain connected status
								if (isStartingConnection)
								{
									flushData();
									_isConnected = true;
									isStartingConnection = false;
									//dataCxn.Create();
								}
								if (_isConnected)
								{
									connectionTimer = ofGetElapsedTimeMillis();
								}
							}
						}
					}
					else
					{
						infoPackets.push_back(packet);
					}
				}
			}
		}
	}

	if (_isConnected)
	{
		// If we're connected, periodically send a PING to EmotiBit
		static uint64_t pingTimer = ofGetElapsedTimeMillis();
		if (ofGetElapsedTimeMillis() - pingTimer > pingInterval)
		{
			pingTimer = ofGetElapsedTimeMillis();

			vector<string> payload;
			payload.push_back(EmotiBitPacket::PayloadLabel::DATA_PORT);
			payload.push_back(ofToString(_dataPort));
			string packet = EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::PING, advertisingPacketCounter++, payload);

			ofLogVerbose() << "Sent: " << packet;

			advertisingCxn.Connect(connectedEmotibitIp.c_str(), advertisingPort);
			advertisingCxn.SetEnableBroadcast(false);
			advertisingCxn.Send(packet.c_str(), packet.length());
		}
	}

	// Handle connecting to EmotiBit
	if (isStartingConnection) {
		// Send connect messages periodically
		static uint64_t startCxnTimer = ofGetElapsedTimeMillis();
		if (ofGetElapsedTimeMillis() - startCxnTimer > startCxnInterval)
		{
			startCxnTimer = ofGetElapsedTimeMillis();

			// Send a connect message to the selected EmotiBit
			vector<string> payload;
			payload.push_back(EmotiBitPacket::PayloadLabel::CONTROL_PORT);
			payload.push_back(ofToString(controlPort));
			payload.push_back(EmotiBitPacket::PayloadLabel::DATA_PORT);
			payload.push_back(ofToString(_dataPort));
			string packet = EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::EMOTIBIT_CONNECT, advertisingPacketCounter++, payload);
			
			ofLogVerbose() << "Sent: " << packet;

			advertisingCxn.Connect(connectedEmotibitIp.c_str(), advertisingPort);
			advertisingCxn.SetEnableBroadcast(false);
			advertisingCxn.Send(packet.c_str(), packet.length());
			
			
		}

		// Timeout starting connection if no response is received
		if (ofGetElapsedTimeMillis() - startCxnAbortTimer > startCxnTimeout)
		{
			isStartingConnection = false;
			connectedEmotibitIp = "";
			connectedEmotibitIdentifier = "";
		}
	}

	// Check to see if connection has timed out
	if (_isConnected)
	{
		if (ofGetElapsedTimeMillis() - connectionTimer > connectionTimeout)
		{
			disconnect();
		}
	}

	// Check to see if EmotiBit availability is stale or needs purging
	discoveredEmotibitsMutex.lock();
	for (auto it = _discoveredEmotibits.begin(); it != _discoveredEmotibits.end(); it++)
	{
		if (ofGetElapsedTimeMillis() - it->second.lastSeen > availabilityTimeout)
		{
			it->second.isAvailable = false;
		}

		//if (ofGetElapsedTimeMillis() - it->second.lastSeen > ipPurgeTimeout)
		//{
		//	_emotibitIps.erase(it);
		//}
		//else
		//{
		//	it++;
		//}
	}
	discoveredEmotibitsMutex.unlock();
	return SUCCESS;
}

int8_t EmotiBitWiFiHost::sendControl(const string& packet)
{
	controlCxnMutex.lock();
	bool sent = false;
	for (unsigned int i = 0; i < (unsigned int)controlCxn.getLastID(); i++)
	{
		if (!controlCxn.isClientConnected(i)) continue;
		// get the ip and port of the client
		string port = ofToString(controlCxn.getClientPort(i));
		string ip = controlCxn.getClientIP(i);

		if (ip.compare(connectedEmotibitIp) != 0) continue;	// Confirm this is the EmotiBit IP we're connected to
		//_isConnected = true;
		//isStartingConnection = false;
		ofLogVerbose("EmotiBitWiFiHost") << "Sending: " << packet;
		controlCxn.send(i, packet);
		sent = true;
		break; // msg sent. stop scanning through all clients
	}
	controlCxnMutex.unlock();
	if (sent)
	{
		//ofLogWarning("EmotiBitWiFiHost") << "TCP Client not connected. TCP transaction skipped for packet: " + packet;
		return SUCCESS;
	}
	ofLogWarning("EmotiBitWiFiHost") << "TCP Client not connected. TCP transaction skipped for packet: " + packet;
	return FAIL;
}

// ToDo: Implement readControl()
//uint8_t EmotiBitWiFiHost::readControl(string& packet)
//{
//	if (_isConnected) {
//
//		// for each connected client lets get the data being sent and lets print it to the screen
//		for (unsigned int i = 0; i < (unsigned int)controlCxn.getLastID(); i++) {
//
//			if (!controlCxn.isClientConnected(i)) continue;
//
//			// get the ip and port of the client
//			string port = ofToString(controlCxn.getClientPort(i));
//			string ip = controlCxn.getClientIP(i);
//
//			if (ip.compare(connectedEmotibitIp) != 0) continue;
//			//string info = "client " + ofToString(i) + " -connected from " + ip + " on port: " + port;
//			//cout << info << endl;
//
//			packet = "";
//			string tmp;
//			do {
//				packet += tmp;
//				tmp = controlCxn.receive(i);
//			} while (tmp != EmotiBitPacket::PACKET_DELIMITER_CSV);
//
//			// if there was a message set it to the corresponding client
//			if (str.length() > 0) {
//				cout << "Message: " << str << endl;
//			}
//
//			cout << "Sending: m" << endl;
//			messageConn.send(i, "m");
//		}
//	}
//	return SUCCESS;
//}

void EmotiBitWiFiHost::readUdp(ofxUDPManager &udp, string& message, string ipFilter)
{
	const int maxSize = 32768;
	static char udpMessage[maxSize];
	int msgSize;
	string ip;
	int port;
	msgSize = udp.Receive(udpMessage, maxSize);
	udp.GetRemoteAddr(ip, port);
	if (ipFilter.length() == 0 || ip.compare(ipFilter) == 0) // && portFilter > -1 && port == portFilter)
	{
		message = udpMessage;
	}
	else
	{
		message = "";
	}
}

void EmotiBitWiFiHost::updateData()
{
	string message;
	dataCxnMutex.lock();
	readUdp(dataCxn, message, connectedEmotibitIp);
	dataCxnMutex.unlock();

	if (!_isConnected)
	{
		// flush the data if we're not connected
		return;
	}

	if (message.size() > 0)
	{
		string packet;
		EmotiBitPacket::Header header;
		size_t startChar = 0;
		size_t endChar;
		do
		{
			endChar = message.find_first_of(EmotiBitPacket::PACKET_DELIMITER_CSV, startChar);
			if (endChar == string::npos)
			{
				ofLogWarning() << "**** MALFORMED MESSAGE **** : no packet delimiter found";
			}
			else
			{
				if (endChar == startChar)
				{
					ofLogWarning() << "**** EMPTY MESSAGE **** ";
				}
				else
				{
					packet = message.substr(startChar, endChar - startChar);	// extract packet

					int16_t dataStartChar = EmotiBitPacket::getHeader(packet, header);	// read header
					if (dataStartChar == EmotiBitPacket::MALFORMED_HEADER)
					{
						ofLogWarning() << "**** MALFORMED PACKET **** : no header data found";
					}
					else
					{
						// We got a well-formed packet header
						if (startChar == 0)
						{
							// This is the first packet in the message
							if (_isConnected)
							{
								// Connect a channel to handle time syncing
								dataCxnMutex.lock();
								string ip;
								int port;
								dataCxn.GetRemoteAddr(ip, port);
								if (port != sendDataPort)
								{
									sendDataPort = port;
									dataCxn.Connect(ip.c_str(), port);
									advertisingCxn.SetEnableBroadcast(false);
								}
								dataCxnMutex.unlock();
							}
							if (header.packetNumber == receivedDataPacketNumber)
							{
								// THIS DOESN'T WORK YET
								// Skip duplicates packets (e.g. from multi-send protocols)
								// Note this assumes the whole message is a duplicate
								//continue;
							}
							else
							{
								// Keep track of packetNumbers we've seen
								receivedDataPacketNumber = header.packetNumber;
							}
						}
						if (header.typeTag.compare(EmotiBitPacket::TypeTag::REQUEST_DATA) == 0)
						{
							// Process data requests
							processRequestData(packet, dataStartChar);
						}
						dataPackets.push_back(packet);
					}
				}
			}
			startChar = endChar + 1;
		} while (endChar != string::npos && startChar < message.size());	// until all packet delimiters are processed
	}
}

void EmotiBitWiFiHost::updateDataThread()
{
	while (!stopDataThread)
	{
		updateData();
		threadSleepFor(_wifiHostSettings.dataThreadSleep);
	}
}

void EmotiBitWiFiHost::threadSleepFor(int sleepMicros)
{
	if (sleepMicros < 0)
	{
		//	do nothing, not even yield
		//	WARNING: high spinlock potential
	}
	else if (sleepMicros == 0)
	{
		std::this_thread::yield();
	}
	else
	{
		std::this_thread::sleep_for(std::chrono::microseconds(sleepMicros));
	}
}

void EmotiBitWiFiHost::processRequestData(const string& packet, int16_t dataStartChar)
{
	// Request Data
	string element;
	string outPacket;
	do
	{
		// Parse through requested packet elements and data
		dataStartChar = EmotiBitPacket::getPacketElement(packet, element, dataStartChar);

		if (element.compare(EmotiBitPacket::TypeTag::TIMESTAMP_LOCAL) == 0)
		{
			outPacket = EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::TIMESTAMP_LOCAL, dataPacketCounter++, ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT), 1);
			sendData(outPacket);
		}
		if (element.compare(EmotiBitPacket::TypeTag::TIMESTAMP_UTC) == 0)
		{
			// ToDo: implement UTC timestamp
		}
		//if (lsl.isConnected()) {
		//	double lsltime = lsl::local_clock();
		//	sendEmotiBitPacket(EmotiBitPacket::TypeTag::TIMESTAMP_CROSS_TIME, ofToString(EmotiBitPacket::TypeTag::TIMESTAMP_LOCAL) + "," + ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT) + ",LC," + ofToString(lsltime, 7));
		//	//cout << EmotiBitPacket::TypeTag::TIMESTAMP_CROSS_TIME << "," << ofToString(EmotiBitPacket::TypeTag::TIMESTAMP_LOCAL) + "," + ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT) + ",LC," + ofToString(lsltime, 7) << endl;
		//}
		//sendEmotiBitPacket(EmotiBitPacket::TypeTag::ACK, ofToString(header.packetNumber) + ',' + header.typeTag, 2);
		////cout << EmotibitPacket::TypeTag::REQUEST_DATA << header.packetNumber << endl;
	} while (dataStartChar > 0);
	EmotiBitPacket::Header header;
	EmotiBitPacket::getHeader(packet, header);
	vector<string> payload;
	payload.push_back(ofToString(header.packetNumber));
	payload.push_back(header.typeTag);
	outPacket = EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::ACK, dataPacketCounter++, payload);
	sendData(outPacket);
}

int8_t EmotiBitWiFiHost::sendData(const string& packet)
{
	if (_isConnected)
	{
		dataCxnMutex.lock();
		dataCxn.Send(packet.c_str(), packet.length());
		dataCxnMutex.unlock();
		return SUCCESS;
	}
	else
	{
		return FAIL;
	}
}

void EmotiBitWiFiHost::readData(vector<string> &packets)
{
	 dataPackets.get(packets);
}

int8_t EmotiBitWiFiHost::disconnect()
{
	if (_isConnected)
	{
		controlCxnMutex.lock();
		string packet = EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::EMOTIBIT_DISCONNECT, controlPacketCounter++, "", 0);
		for (int i = controlCxn.getLastID() - 1; i >= 0; i--) {
			string ip = controlCxn.getClientIP(i);
			if (ip.compare(connectedEmotibitIp) == 0)
			{
				controlCxn.send(i, packet);
			}
		}
		controlCxnMutex.unlock();

		dataCxnMutex.lock();
		flushData();
		//dataCxn.Close();
		//_startDataCxn(controlPort + 1);
		dataCxnMutex.unlock();
		connectedEmotibitIp = "";
		connectedEmotibitIdentifier = "";
		_isConnected = false;
		isStartingConnection = false;
	}

	return SUCCESS;
}

int8_t EmotiBitWiFiHost::_startDataCxn(uint16_t dataPort)
{
	_dataPort = dataPort;
	dataCxn.Create();
	dataCxn.SetReuseAddress(false);
	while (!dataCxn.Bind(_dataPort))
	{
		// Try to bind _dataPort until we find one that's available
		_dataPort += 2;
		ofLogNotice() << "Trying data port: " << _dataPort;
	}
	//dataCxn.SetEnableBroadcast(false);
	dataCxn.SetNonBlocking(true);
	dataCxn.SetReceiveBufferSize(pow(2, 15));

	ofLogNotice() << "dataCxn GetMaxMsgSize: " << dataCxn.GetMaxMsgSize();
	ofLogNotice() << "dataCxn GetReceiveBufferSize: " << dataCxn.GetReceiveBufferSize();
	ofLogNotice() << "dataCxn GetTimeoutReceive: " << dataCxn.GetTimeoutReceive();

	return SUCCESS;
}

int8_t EmotiBitWiFiHost::flushData()
{
	const int maxSize = 32768;
	char udpMessage[maxSize];
	
	while (dataCxn.Receive(udpMessage, maxSize) > 0);
	//dataCxn.Close();
	//dataCxn.Create();
	

	return SUCCESS;
}

// Connecting is done asynchronously because attempts are repeated over UDP until connected
int8_t EmotiBitWiFiHost::connect(string deviceId)
{
	if (!isStartingConnection && !_isConnected)
	{
		discoveredEmotibitsMutex.lock();
		string ip = _discoveredEmotibits[deviceId].ip;
		bool isAvailable = _discoveredEmotibits[deviceId].isAvailable;
		discoveredEmotibitsMutex.unlock();
		try
		{
			if (ip.compare("") != 0 && isAvailable)	// If the ip is on our list and available
			{
				connectedEmotibitIp = ip;
				connectedEmotibitIdentifier = deviceId;
				isStartingConnection = true;
				startCxnAbortTimer = ofGetElapsedTimeMillis();
			}
		}
		catch (const std::out_of_range& oor) {
			ofLogWarning() << "EmotiBit " << ip << " not found";
			oor;
		}
	}

	return SUCCESS;
}

unordered_map<string, EmotibitInfo> EmotiBitWiFiHost::getdiscoveredEmotibits()
{
	discoveredEmotibitsMutex.lock();
	auto output = _discoveredEmotibits;
	discoveredEmotibitsMutex.unlock();

	return output;
}



string EmotiBitWiFiHost::ofGetTimestampString(const string& timestampFormat)
{
	std::stringstream str;
	auto now = std::chrono::system_clock::now();
	auto t = std::chrono::system_clock::to_time_t(now);    std::chrono::duration<double> s = now - std::chrono::system_clock::from_time_t(t);
	int us = s.count() * 1000000;
	auto tm = *std::localtime(&t);
	constexpr int bufsize = 256;
	char buf[bufsize];

	// Beware! an invalid timestamp string crashes windows apps.
	// so we have to filter out %i (which is not supported by vs)
	// earlier.
	auto tmpTimestampFormat = timestampFormat;
	ofStringReplace(tmpTimestampFormat, "%i", ofToString(us / 1000, 3, '0'));
	ofStringReplace(tmpTimestampFormat, "%f", ofToString(us, 6, '0'));

	if (strftime(buf, bufsize, tmpTimestampFormat.c_str(), &tm) != 0) {
		str << buf;
	}
	auto ret = str.str();


	return ret;
}

vector<string> EmotiBitWiFiHost::getLocalIPs()
{
	vector<string> result;

#ifdef TARGET_WIN32

	string commandResult = ofSystem("ipconfig");
	//ofLogVerbose() << commandResult;

	for (int pos = 0; pos >= 0; )
	{
		pos = commandResult.find("IPv4", pos);

		if (pos >= 0)
		{
			pos = commandResult.find(":", pos) + 2;
			int pos2 = commandResult.find("\n", pos);

			string ip = commandResult.substr(pos, pos2 - pos);

			pos = pos2;

			if (ip.substr(0, 3) != "127") // let's skip loopback addresses
			{
				result.push_back(ip);
				//ofLogVerbose() << ip;
			}
		}
	}

#else

	string commandResult = ofSystem("ifconfig");

	for (int pos = 0; pos >= 0; )
	{
		pos = commandResult.find("inet ", pos);

		if (pos >= 0)
		{
			int pos2 = commandResult.find("netmask", pos);

			string ip = commandResult.substr(pos + 5, pos2 - pos - 6);

			pos = pos2;

			if (ip.substr(0, 3) != "127") // let's skip loopback addresses
			{
				result.push_back(ip);
				//ofLogVerbose() << ip;
			}
		}
	}

#endif

	return result;
}

bool EmotiBitWiFiHost::isConnected()
{
	return _isConnected;
}

void EmotiBitWiFiHost::setWifiHostSettings(WifiHostSettings settings)
{
	_wifiHostSettings = settings;
}

EmotiBitWiFiHost::WifiHostSettings EmotiBitWiFiHost::getWifiHostSettings()
{
	return _wifiHostSettings;
}

// saveEmotiBitCommSettings no longer matches settings and may or may not be used in the future
// Code is left here commented out in case it might be useful at a later time
//void ofApp::saveEmotiBitCommSettings(string settingsFilePath, bool absolute, bool pretty)
//{
//	// ToDo: find a nice home like EmotiBitFileIO.h/cpp
//
//	try
//	{
//		EmotiBitWiFiHost::WifiHostSettings settings = emotiBitWiFi.getWifiHostSettings();
//		ofxJSONElement jsonSettings;
//
//		jsonSettings["wifi"]["advertising"]["transmission"]["broadcast"]["enabled"] = settings.enableBroadcast;
//		jsonSettings["wifi"]["advertising"]["transmission"]["unicast"]["enabled"] = settings.enableUnicast;
//		jsonSettings["wifi"]["advertising"]["transmission"]["unicast"]["ipMin"] = settings.unicastIpRange.first;
//		jsonSettings["wifi"]["advertising"]["transmission"]["unicast"]["ipMax"] = settings.unicastIpRange.second;
//
//		int numIncludes = settings.networkIncludeList.size();
//		for (int i = 0; i < numIncludes; i++)
//		{
//			jsonSettings["wifi"]["network"]["includeList"][i] = settings.networkIncludeList.at(i);
//		}
//
//		int numExcludes = settings.networkExcludeList.size();
//		for (int i = 0; i < numExcludes; i++)
//		{
//			jsonSettings["wifi"]["network"]["excludeList"][i] = settings.networkExcludeList.at(i);
//		}
//
//		jsonSettings.save(ofToDataPath(settingsFilePath, absolute), pretty);
//		ofLog(OF_LOG_NOTICE, "Saving " + settingsFilePath + ": \n" + jsonSettings.getRawString(true));
//	}
//	catch (exception e)
//	{
//		ofLog(OF_LOG_ERROR, "ERROR: Failed to save " + settingsFilePath);
//	}
//}

void EmotiBitWiFiHost::parseCommSettings(string jsonStr)
{
	Json::Reader reader;
	Json::Value jsonSettings;
	
	try
	{
		if (reader.parse(jsonStr, jsonSettings))
		{
			// ToDo: Move specifics of WiFi settings into EmotiBitWiFiHost
			EmotiBitWiFiHost::WifiHostSettings settings;
			EmotiBitWiFiHost::WifiHostSettings defaultSettings = getWifiHostSettings();// if setter is not called, getter returns default values

			if (jsonSettings["wifi"]["advertising"].isMember("sendAdvertisingInterval_msec"))
			{
				settings.sendAdvertisingInterval = jsonSettings["wifi"]["advertising"]["sendAdvertisingInterval_msec"].asInt();
			}
			else
			{
				ofLogNotice("sendAdvertisingInterval_msec settings not found. Using default value");
				settings.sendAdvertisingInterval = defaultSettings.sendAdvertisingInterval;
			}
			if (jsonSettings["wifi"]["advertising"].isMember("checkAdvertisingInterval_msec"))
			{
				settings.checkAdvertisingInterval = jsonSettings["wifi"]["advertising"]["checkAdvertisingInterval_msec"].asInt();
			}
			else
			{
				ofLogNotice("checkAdvertisingInterval_msec settings not found. Using default value");
				settings.checkAdvertisingInterval = defaultSettings.checkAdvertisingInterval;
			}
			if (jsonSettings["wifi"]["advertising"].isMember("threadSleep_usec"))
			{
				settings.advertisingThreadSleep = jsonSettings["wifi"]["advertising"]["threadSleep_usec"].asInt();
			}
			else
			{
				ofLogNotice("advertising:threadSleep_usec settings not found. Using default value");
				settings.advertisingThreadSleep = defaultSettings.advertisingThreadSleep;
			}
			if (jsonSettings["wifi"]["advertising"]["transmission"]["broadcast"].isMember("enabled"))
			{
				settings.enableBroadcast = jsonSettings["wifi"]["advertising"]["transmission"]["broadcast"]["enabled"].asBool();
			}
			else
			{
				ofLogNotice("Broadcast settings not found. Using default value");
				settings.enableBroadcast = defaultSettings.enableBroadcast;
			}
			if (jsonSettings["wifi"]["advertising"]["transmission"]["unicast"].isMember("enabled"))
			{
				settings.enableUnicast = jsonSettings["wifi"]["advertising"]["transmission"]["unicast"]["enabled"].asBool();
			}
			else
			{
				ofLogNotice("Unicast enable settings not found in. Using default value");
				settings.enableUnicast = defaultSettings.enableUnicast;
			}
			if (jsonSettings["wifi"]["advertising"]["transmission"]["unicast"].isMember("ipMin") &&
				jsonSettings["wifi"]["advertising"]["transmission"]["unicast"].isMember("ipMax"))
			{
				settings.unicastIpRange = make_pair(
					jsonSettings["wifi"]["advertising"]["transmission"]["unicast"]["ipMin"].asInt(),
					jsonSettings["wifi"]["advertising"]["transmission"]["unicast"]["ipMax"].asInt()
				);
			}
			else
			{
				ofLogNotice("unicast ipRange settings not found. Using default value");
				settings.unicastIpRange = defaultSettings.unicastIpRange;
			}
			if (jsonSettings["wifi"]["advertising"]["transmission"]["unicast"].isMember("nUnicastIpsPerLoop"))
			{
				settings.nUnicastIpsPerLoop = jsonSettings["wifi"]["advertising"]["transmission"]["unicast"]["nUnicastIpsPerLoop"].asInt();
			}
			else
			{
				ofLogNotice("nUnicastIpsPerLoop settings not found in. Using default value");
				settings.nUnicastIpsPerLoop = defaultSettings.nUnicastIpsPerLoop;
			}
			if (jsonSettings["wifi"]["advertising"]["transmission"]["unicast"].isMember("unicastMinLoopDelay_msec"))
			{
				settings.unicastMinLoopDelay = jsonSettings["wifi"]["advertising"]["transmission"]["unicast"]["unicastMinLoopDelay_msec"].asInt();
			}
			else
			{
				ofLogNotice("unicastMinLoopDelay_msec settings not found. Using default value");
				settings.unicastMinLoopDelay = defaultSettings.unicastMinLoopDelay;
			}

			if (jsonSettings["wifi"]["data"].isMember("threadSleep_usec"))
			{
				settings.dataThreadSleep = jsonSettings["wifi"]["data"]["threadSleep_usec"].asInt();
			}
			else
			{
				ofLogNotice("advertising:threadSleep_usec settings not found. Using default value");
				settings.dataThreadSleep = defaultSettings.dataThreadSleep;
			}

			if (jsonSettings["wifi"]["network"].isMember("includeList"))
			{
				int numIncludes = jsonSettings["wifi"]["network"]["includeList"].size();
				settings.networkIncludeList.clear();
				for (int i = 0; i < numIncludes; i++)
				{
					settings.networkIncludeList.push_back(jsonSettings["wifi"]["network"]["includeList"][i].asString());
				}
			}
			else
			{
				ofLogNotice("networkIncludeList settings not found. Using default value");
				settings.networkIncludeList = defaultSettings.networkIncludeList;
			}

			if (jsonSettings["wifi"]["network"].isMember("excludeList"))
			{
				int numExcludes = jsonSettings["wifi"]["network"]["excludeList"].size();
				settings.networkExcludeList.clear();
				for (int i = 0; i < numExcludes; i++)
				{
					settings.networkExcludeList.push_back(jsonSettings["wifi"]["network"]["excludeList"][i].asString());
				}
			}
			else
			{
				ofLogNotice("networkExcludeList settings not found. Using default value");
				settings.networkExcludeList = defaultSettings.networkExcludeList;
			}

			setWifiHostSettings(settings);

			ofLog(OF_LOG_NOTICE, "[EmotiBitWifiHost] CommSettings loaded: \n" + jsonStr);
		}
		else
		{
			ofLogError("[EmotiBitWifiHost] Failed to parse CommSettings");
			ofLog(OF_LOG_NOTICE, "using default network parameters");
			setWifiHostSettings(getWifiHostSettings()); // if setter is not called, getter returns default values
			ofLog(OF_LOG_ERROR, "ERROR: Failed to load: \n" + jsonStr);

		}
	}
	catch (exception e)
	{
		ofLogError("[EmotiBitWifiHost] CommSettings settings parse exception: ") << e.what();
		ofLog(OF_LOG_NOTICE, "using default network parameters");
		setWifiHostSettings(getWifiHostSettings()); // if setter is not called, getter returns default values
		ofLog(OF_LOG_ERROR, "ERROR: Failed to load: \n" + jsonStr);
	}
}

void EmotiBitWiFiHost::readAuxNetworkChannel()
{
	// ToDo: This function definition will change when a TCP channel is implemented.
	auxNetworkChannelController.readAuxCxn(AuxCxnController::AuxChannel::CHANNEL_UDP);
}

bool EmotiBitWiFiHost::attachAppQ(AuxInstrQ* q)
{
	bool status = auxNetworkChannelController.attachAppQ(q);
	if (status)
	{
		ofLogVerbose("EmotiBitWiFiHost") << "AuxInstrQ attached";
		return true;
	}
	else
	{
		ofLogError("EmotiBitWiFiHost") << "AuxInstrQ nullptr exception";
		return false;
	}
}

void EmotiBitWiFiHost::updateAuxInstrQ()
{
	auxNetworkChannelController.pushToAppQ();
}

void EmotiBitWiFiHost::processAppQ()
{
	Json::Reader reader;
	Json::Value jsonSettings;
	std::string instruction;
	
	try
	{
		// Process all messages of type WH - WiFiHost
		if (auxNetworkChannelController.appQ->getSize())
		{
			bool status = auxNetworkChannelController.appQ->front(instruction);
			if (status)
			{
				if (reader.parse(instruction, jsonSettings))
				{
					if (jsonSettings["version"].asInt() == 0)
					{
						// parsing for V0 format
						if (jsonSettings["target"].asString().compare("WIFI_HOST") == 0)
						{
							// Message meant to be parsed by WIFI_HOST
							// pop queue element
							ofLogVerbose("EmotiBitWiFiHost") << "Processing Aux Instruction";
							{
								// locally scoped to destroy variable after popping
								std::string str;
								auxNetworkChannelController.appQ->pop(str);
								auxNetworkChannelController.appQ->updateLastPopTime();
							}
							// loop through all the actions
							// perform required action
							while(jsonSettings.isMember("action") && jsonSettings["action"].size())
							{
								if (jsonSettings["action"][0].asString().compare("EMOTIBIT_CONNECT") == 0)
								{
									ofLogVerbose("EmotiBitWiFiHost::processAuxQ()") << "Executing " + ofToString(EmotiBitPacket::TypeTag::EMOTIBIT_CONNECT);
									std::string emotibitId = jsonSettings["action"][1].asString();
									// ToDo: We probably need to also call "clear Oscilloscope" before we can connect.
									// Since that function belongs to ofApp, it probably needs to be called using an Event handle
									connect(emotibitId);
									break;
								}
								else if (jsonSettings["action"][0].asString().compare("EMOTIBIT_DISCONNECT") == 0)
								{
									ofLogVerbose("EmotiBitWiFiHost::processAuxQ()") << "Executing " + ofToString(EmotiBitPacket::TypeTag::EMOTIBIT_DISCONNECT);
									// ToDo: We probably need to also call "clear Oscilloscope" before we can connect.
									// Since that function belongs to ofApp, it probably needs to be called using an Event handle
									disconnect();
									break;
								}
								else if (jsonSettings["action"][0].asString().compare("RECORD_BEGIN") == 0)
								{
									// RECORD_BEGIN
									ofLogVerbose("EmotiBitWiFiHost::processAuxQ()") << "Executing " + ofToString(EmotiBitPacket::TypeTag::RECORD_BEGIN);
									string localTime = EmotiBit::ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT);
									sendControl(EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::RECORD_BEGIN, controlPacketCounter++, localTime, 1));
									break;
								}
								else if (jsonSettings["action"][0].asString().compare("RECORD_END") == 0)
								{
									// RECORD_END
									ofLogVerbose("EmotiBitWiFiHost::processAuxQ()") << "Executing " + ofToString(EmotiBitPacket::TypeTag::RECORD_END);
									string localTime = EmotiBit::ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT);
									sendControl(EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::RECORD_END, controlPacketCounter++, localTime, 1));
									break;
								}
								else if (jsonSettings["action"][0].asString().compare("DIRECT_MESSAGE") == 0)
								{
									// ToDo: implement sending message to emotibit using CTR/DAT/ADV channels
									// Arguments are meant to be stitched together and sent directly to EmotiBit
								}
								else
								{
									// future functionality.
									ofLogVerbose("EmotiBitWiFiHost") << "Action currently not defined.";
									break;
								}
							}
						}
					}
				}
				else
				{
					// not valid JSON
					ofLogNotice("EmotiBitWiFiHost") << "Aux Instruction not in JSON format. Removing from Queue.";
					// pop from Queue
					std::string str;
					auxNetworkChannelController.appQ->pop(str);
					auxNetworkChannelController.appQ->updateLastPopTime();
				}

			}
		}
	}
	catch (exception e)
	{
		ofLogWarning("[EmotiBitWiFiHost::processAppQ] Failed to parse message ") << e.what();
		ofLogWarning("Skipping: ") << instruction;
	}
}