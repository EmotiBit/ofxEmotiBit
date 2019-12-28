#include "EmotiBitWiFiHost.h"

int8_t EmotiBitWiFiHost::begin()
{
	advertisingPort = EmotiBitComms::WIFI_ADVERTISING_PORT;
	vector<string> ips = getLocalIPs();
	advertisingCxn.Create();
	vector<string> ipSplit = ofSplitString(ips.at(0), ".");
	advertisingIp = ipSplit.at(0) + "." + ipSplit.at(1) + "." + ipSplit.at(2) + "." + ofToString(255);
	cout << advertisingIp << endl;
	advertisingCxn.Connect(advertisingIp.c_str(), advertisingPort);
	advertisingCxn.SetEnableBroadcast(true);
	advertisingCxn.SetNonBlocking(true);
	advertisingCxn.SetReceiveBufferSize(pow(2, 10));

	dataPort = 3002;
	dataCxn.Create();
	while (!dataCxn.Bind(dataPort))
	{
		// Try to bind dataPort until we find one that's available
		dataPort++;
	}
	//dataCxn.SetEnableBroadcast(false);
	dataCxn.SetNonBlocking(true);
	dataCxn.SetReceiveBufferSize(pow(2, 15));

	controlPort = 11998;
	controlCxn.setMessageDelimiter(ofToString(EmotiBitPacket::PACKET_DELIMITER_CSV));
	while (!controlCxn.setup(controlPort))
	{
		// Try to setup a controlPort until we find one that's available
		controlPort++;
	}

	cout << "EmotiBit data port: " << dataPort << endl;
	cout << "EmotiBit control port: " << controlPort << endl;

	advertisingPacketCounter = 0;
	controlPacketCounter = 0;
	connectedEmotibitIp = "";
	isConnected = false;
	isStartingConnection = false;

	return SUCCESS;
}

int8_t EmotiBitWiFiHost::processAdvertising()
{
	const int maxSize = 32768;

	// Send advertising messages periodically
	static uint64_t advertizingTimer = ofGetElapsedTimeMillis();
	if (ofGetElapsedTimeMillis() - advertizingTimer > advertisingInterval)
	{
		advertizingTimer = ofGetElapsedTimeMillis();

		advertisingCxn.Connect(advertisingIp.c_str(), advertisingPort);
		advertisingCxn.SetEnableBroadcast(true);

		// Send advertising message
		string packet = EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::HELLO_EMOTIBIT, advertisingPacketCounter++, "", 0);
		cout << "Sent: " << packet;
		advertisingCxn.Send(packet.c_str(), packet.length());
	}


	// Receive advertising messages
	static char udpMessage[maxSize];
	int msgSize = advertisingCxn.Receive(udpMessage, maxSize);
	if (msgSize > 0)
	{
		string message = udpMessage;
		cout << "Received: " << message;

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
					int16_t valuePos = EmotiBitPacket::getPacketKeyedValue(packet, EmotiBitPacket::PayloadLabel::DATA_PORT, value, dataStartChar);
					if (valuePos > -1)
					{
						cout << "Emotibit: " << ip << ", " << port << endl;
						// Add ip address to our list
						auto it = _emotibitIps.emplace(ip, EmotiBitStatus(ofToInt(value) == EMOTIBIT_AVAILABLE));
						if (!it.second)
						{
							// if it's not a new ip address, update the status
							it.first->second = EmotiBitStatus(ofToInt(value) == EMOTIBIT_AVAILABLE);
						}
					}
				}
				else if (header.typeTag.compare(EmotiBitPacket::TypeTag::PONG) == 0)
				{
					// PONG
					if (ip.compare(connectedEmotibitIp) == 0)
					{
						string value;
						int16_t valuePos = EmotiBitPacket::getPacketKeyedValue(packet, EmotiBitPacket::PayloadLabel::DATA_PORT, value, dataStartChar);
						if (valuePos > -1 && ofToInt(value) == dataPort)
						{
							// Establish / maintain connected status
							if (isStartingConnection)
							{
								isConnected = true;
								isStartingConnection = false;
								//dataCxn.Create();
							}
							if (isConnected)
							{
								connectionTimer = ofGetElapsedTimeMillis();
							}
						}
					}
				}
			}
		}
	}

	if (isConnected)
	{
		// If we're connected, periodically send a PING to EmotiBit
		static uint64_t pingTimer = ofGetElapsedTimeMillis();
		if (ofGetElapsedTimeMillis() - pingTimer > pingInterval)
		{
			pingTimer = ofGetElapsedTimeMillis();

			vector<string> data;
			data.push_back(EmotiBitPacket::PayloadLabel::DATA_PORT);
			data.push_back(ofToString(dataPort));
			string packet = EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::PING, advertisingPacketCounter++, data);

			cout << "Sent: " << packet;
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
			vector<string> data;
			data.push_back(EmotiBitPacket::PayloadLabel::CONTROL_PORT);
			data.push_back(ofToString(controlPort));
			data.push_back(EmotiBitPacket::PayloadLabel::DATA_PORT);
			data.push_back(ofToString(dataPort));
			string packet = EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::EMOTIBIT_CONNECT, advertisingPacketCounter++, data);
			
			cout << "Sent: " << packet;
			advertisingCxn.Connect(connectedEmotibitIp.c_str(), advertisingPort);
			advertisingCxn.SetEnableBroadcast(false);
			advertisingCxn.Send(packet.c_str(), packet.length());
		}
	}

	// Check to see if connection has timed out
	if (isConnected)
	{
		if (ofGetElapsedTimeMillis() - connectionTimer > connectionTimeout)
		{
			disconnect();
		}
	}

	// Check to see if EmotiBit availability is stale or needs purging
	for (auto it = _emotibitIps.begin(); it != _emotibitIps.end(); it++)
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

	return SUCCESS;
}

int8_t EmotiBitWiFiHost::sendControl(string& packet)
{
	controlCxnMutex.lock();
	for (unsigned int i = 0; i < (unsigned int)controlCxn.getLastID(); i++)
	{
		if (!controlCxn.isClientConnected(i)) continue;
		// get the ip and port of the client
		string port = ofToString(controlCxn.getClientPort(i));
		string ip = controlCxn.getClientIP(i);

		if (ip.compare(connectedEmotibitIp) != 0) continue;	// Confirm this is the EmotiBit IP we're connected to

		//isConnected = true;
		//isStartingConnection = false;

		cout << "Sending: " << packet << endl;
		controlCxn.send(i, packet);
	}
	controlCxnMutex.unlock();

	return SUCCESS;
}

// ToDo: Implement readControl()
//uint8_t EmotiBitWiFiHost::readControl(string& packet)
//{
//	if (isConnected) {
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

int8_t EmotiBitWiFiHost::readData(string& message)
{
	const int maxSize = 32768;
	static char udpMessage[maxSize];
	string ip;
	int port;
	int msgSize;
	msgSize = dataCxn.Receive(udpMessage, maxSize);
	dataCxn.GetRemoteAddr(ip, port);
	if (ip.compare(connectedEmotibitIp) == 0 && port == dataPort)
	{
		message = udpMessage;
		if (message.length() > 0)
		{
			//isConnected = true;
			//isStartingConnection = false;
		}
	}

	return SUCCESS;
}

int8_t EmotiBitWiFiHost::disconnect()
{
	if (isConnected)
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

		flushData();
		connectedEmotibitIp = "";
		isConnected = false;
		isStartingConnection = false;
	}

	return SUCCESS;
}

int8_t EmotiBitWiFiHost::flushData()
{
	const int maxSize = 32768;
	char udpMessage[maxSize];
	while (dataCxn.Receive(udpMessage, maxSize) > 0);

	return SUCCESS;
}

// Connecting is done asynchronously because attempts are repeated over UDP until connected
int8_t EmotiBitWiFiHost::connect(string ip)
{
	if (!isStartingConnection && !isConnected)
	{
		emotibitIpsMutex.lock();
		try
		{
			if (ip.compare("") != 0 && _emotibitIps.at(ip).isAvailable)	// If the ip is on our list and available
			{
				connectedEmotibitIp = ip;
				isStartingConnection = true;
			}
		}
		catch (const std::out_of_range& oor) {
			std::cout << "EmotiBit " << ip << " not found" << endl;
		}
		emotibitIpsMutex.unlock();
	}

	return SUCCESS;
}

unordered_map<string, EmotiBitStatus> EmotiBitWiFiHost::getEmotiBitIPs()
{
	//emotibitIpsMutex.lock();
	//unordered_map<string, bool> output;
	return _emotibitIps;
	//emotibitIpsMutex.unlock();
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