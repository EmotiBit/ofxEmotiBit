#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	connectionPort = 30000;


	ofBackground(255, 255, 255);
	legendFont.load(ofToDataPath("verdana.ttf"), 12, true, true);
	subLegendFont.load(ofToDataPath("verdana.ttf"), 7, true, true);

	recordingButton.addListener(this, &ofApp::recordButtonPressed);
	hibernateButton.addListener(this, &ofApp::hibernateButtonPressed);
	sendUserNote.addListener(this, &ofApp::sendExperimenterNoteButton);

	int guiXPos = 0;
	int guiYPos = 20;
	int guiWidth = 200;
	int guiPosInc = guiWidth + 1;
	guiPanels.resize(6);
	int p = 0;
	guiPanelDevice = p;
	guiPanels.at(guiPanelDevice).setup("selectDevice", "junk.xml", guiXPos, -guiYPos*2.2);
	deviceMenuGroup.setName(GUI_DEVICE_GROUP_MENU_NAME);
	deviceMenuGroup.add(deviceSelected.set("EmotiBit", GUI_STRING_NO_EMOTIBIT_SELECTED));
	deviceGroup.setName(GUI_DEVICE_GROUP_NAME);
	//deviceList.emplace_back("Message All Emotibits", true);
	//deviceGroup.add(deviceList.at(deviceList.size() - 1));
	deviceMenuGroup.add(deviceGroup);
	guiPanels.at(guiPanelDevice).add(deviceMenuGroup);
	p++;
	guiXPos += guiPosInc;
	guiPanelRecord = p;
	guiPanels.at(guiPanelRecord).setup("startRecording", "junk.xml", guiXPos, -guiYPos);
	guiPanels.at(guiPanelRecord).add(recordingStatus.setup("Status", GUI_STRING_NOT_RECORDING));
	guiPanels.at(guiPanelRecord).add(recordingButton.set("Record", false));
	//guiPanels.at(0).getControl("Record")->setSize(guiWidth, guiYPos * 2);
	p++;
	guiXPos += guiPosInc;
	guiPanelMode = p;
	guiPanels.at(guiPanelMode).setup("hibernate", "junk.xml", guiXPos, -guiYPos);
	guiPanels.at(guiPanelMode).add(hibernateStatus.setup("Mode", GUI_STRING_MODE_ACTIVE));
	guiPanels.at(guiPanelMode).add(hibernateButton.set("Hibernate", false));
	p++;
	guiXPos += guiPosInc;
	guiPanelLevels = p;
	guiPanels.at(guiPanelLevels).setup("batteryStatus", "junk.xml", guiXPos, -guiYPos);
	guiPanels.at(guiPanelLevels).add(batteryStatus.setup("Battery Level", "?"));
	guiPanels.at(guiPanelLevels).add(sdCardStatus.setup("SD Card Remaining", "?"));
	p++;
	guiXPos += guiPosInc;
	guiPanelErrors = p;
	guiPanels.at(guiPanelErrors).setup("errorStatus", "junk.xml", guiXPos, -guiYPos);
	guiPanels.at(guiPanelErrors).add(dataClippingCount.set(GUI_STRING_CLIPPING_EVENTS, 0, 0, 0));
	guiPanels.at(guiPanelErrors).add(dataOverflowCount.set(GUI_STRING_OVERFLOW_EVENTS, 0, 0, 0));
	p++;
	guiXPos += guiPosInc;
	guiPanelUserNote = p;
	guiPanels.at(guiPanelUserNote).setDefaultWidth(ofGetWindowWidth() - guiXPos);
	guiPanels.at(guiPanelUserNote).setup("userNote", "junk.xml", guiXPos, -guiYPos);
	guiPanels.at(guiPanelUserNote).add(userNote.setup("Note:", "[Add a note]"));
	guiPanels.at(guiPanelUserNote).add(sendUserNote.setup("Send Note"));

	//gui.setup("panel"); // most of the time you don't need a name but don't forget to call setup
	//gui.add(recordingStatus.set("Recording", false));
	//gui.add(batteryStatus.set("Battery", 0, 0, 100));
	//gui.add(sdCardStatus.set("SD Card", 0, 0, 100));

	typeTags = vector<vector<vector<string>>>
	{
		{ // scope panel 1
			{ EmotiBitPacket::TypeTag::ACCELEROMETER_X, EmotiBitPacket::TypeTag::ACCELEROMETER_Y, EmotiBitPacket::TypeTag::ACCELEROMETER_Z },
			{ EmotiBitPacket::TypeTag::GYROSCOPE_X, EmotiBitPacket::TypeTag::GYROSCOPE_Y, EmotiBitPacket::TypeTag::GYROSCOPE_Z },
			{ EmotiBitPacket::TypeTag::MAGNETOMETER_X, EmotiBitPacket::TypeTag::MAGNETOMETER_Y, EmotiBitPacket::TypeTag::MAGNETOMETER_Z },
			{ EmotiBitPacket::TypeTag::EDA },
			//{ EmotiBitPacket::TypeTag::EDL, EmotiBitPacket::TypeTag::EDR },
			{ EmotiBitPacket::TypeTag::HUMIDITY_0}
		},
		{ // scope panel 2
			{ EmotiBitPacket::TypeTag::PPG_RED },
			{ EmotiBitPacket::TypeTag::PPG_INFRARED },
			{ EmotiBitPacket::TypeTag::PPG_GREEN },
			{ EmotiBitPacket::TypeTag:: TEMPERATURE_0 },
			{ EmotiBitPacket::TypeTag::THERMISTOR}
		}
	};
	// Create an index mapping for each type tag
	for (int w = 0; w < typeTags.size(); w++) {
		for (int s = 0; s < typeTags.at(w).size(); s++) {
			for (int p = 0; p < typeTags.at(w).at(s).size(); p++) {
				vector<int> indexes{ w, s, p };
				typeTagIndexes.emplace(typeTags.at(w).at(s).at(p), indexes);
			}
		}
	}

	bufferSizes = initBuffer(bufferSizes);
	dataCounts = initBuffer(dataCounts);
	dataFreqs = initBuffer(dataFreqs);

	samplingFreqs = vector<vector<float>>
	{
		{ // scope panel 1
			{ 60.f },
			{ 60.f },
			{ 60.f },
			{ 15.f },
			{ 7.5f }
		},
		{ // scope panel 2
			{ 25.f },
			{ 25.f },
			{ 25.f },
			{ 7.5f },
			{ 7.5f }
		}
	};

	plotNames = vector<vector<vector<string>>>
	{
		{ // scope panel 1
			{ "ACC:X", "ACC:Y", "ACC:Z" },
			{ "GYRO:X", "GYRO:Y", "GYRO:Z" },
			{ "MAG:X", "MAG:Y", "MAG:Z" },
			{ "EDA" },
			//{ "EDL", "EDR" },
			{ "HUMIDITY" }
		},
		{ // scope panel 2
			{ "PPG:RED" },
			{ "PPG:IR" },
			{ "PPG:GRN" },
			{ "TEMP" },
			{ "THERM" }
		}
	};
	yLims = vector<vector<vector<float>>>
	{
		{ // scope panel 1
			{  -8.f, 8.f  },
			{  -1000.f, 1000.f  },
			{  0.f,0.f  },
			{ 0.f,0.f },
			//{ -0.01f, 3.31f },
			{ 0.f, 0.f  }
		},
		{ // scope panel 2
			{  0.f,0.f  },
			{  0.f,0.f  },
			{  0.f,0.f  },
			{  0.f,0.f  },
			{  0.f,0.f  }
		}
	};

	plotColors = { ofColor(0,0,0), ofColor(255,0,0) , ofColor(0,191,0), ofColor(0,0,255) };
	float timeWindow = 20.; // seconds

	int guiHeight = guiPanels.at(guiPanels.size() - 1).getPosition().y + guiPanels.at(guiPanels.size() - 1).getHeight();
	ofRectangle scopeArea = ofRectangle(ofPoint(0, guiHeight), ofPoint(ofGetWidth() / 2, ofGetHeight()));
	ofRectangle scopeArea2 = ofRectangle(ofPoint(ofGetWidth() / 2, guiHeight), ofPoint(ofGetWidth(), ofGetHeight()));


	scopeWins.emplace_back(plotNames.at(0).size(), scopeArea, legendFont); // Setup the multiScope panel
	scopeWins.emplace_back(plotNames.at(1).size(), scopeArea2, legendFont); // Setup the multiScope panel

	for (int w = 0; w < plotNames.size(); w++) {
		for (int s = 0; s < plotNames.at(w).size(); s++) {
			scopeWins.at(w).scopes.at(s).setup(timeWindow, samplingFreqs.at(w).at(s), plotNames.at(w).at(s), plotColors,
				0, 1); // Setup each oscilloscope panel
			if (yLims.at(w).at(s).at(0) == yLims.at(w).at(s).at(1)) {
				scopeWins.at(w).scopes.at(s).autoscaleY(true);
			}
			else {
				scopeWins.at(w).scopes.at(s).setYLims(pair<float, float>(yLims.at(w).at(s).at(0), yLims.at(w).at(s).at(1)));
			}
		}
	}

	counter = 0;
	counter2 = 0;

	selectedScope = 0; // Select all scopes for increment/decrement

	isPaused = false;

	vector<string> ips = getLocalIPs();
	//sendBroadcast(ips.at(0));

	udpConnection.Create();
	vector<string> ipSplit = ofSplitString(ips.at(0), ".");
	string ipAddress = ipSplit.at(0) + "." + ipSplit.at(1) + "." + ipSplit.at(2) + "." + ofToString(255);
	cout << ipAddress << endl;
	udpConnection.Connect(ipAddress.c_str(), connectionPort);
	udpConnection.SetEnableBroadcast(true);
	udpConnection.SetNonBlocking(true);
	string message = "hi";
	string localTime = ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT);
	sendEmotiBitPacket(EmotiBitPacket::TypeTag::PPG_INFRARED, localTime);
	// udpConnection.Send(message.c_str(), message.length());
	udpConnection.GetMaxMsgSize();
	udpConnection.GetReceiveBufferSize();
	udpConnection.GetTimeoutReceive();
	udpConnection.SetReceiveBufferSize(pow(2, 15));

	cout << "GetMaxMsgSize" <<  udpConnection.GetMaxMsgSize() << endl;
	cout << "GetReceiveBufferSize" <<  udpConnection.GetReceiveBufferSize() << endl;
	cout << "GetTimeoutReceive" <<  udpConnection.GetTimeoutReceive() << endl;

	//mySerial.getDeviceList();
	//char devPort[] = mySerial.getDevicePath();
	//printf("%s",devPort);
	dataLogger.setFilename("dataLog.txt");
	dataLogger.startThread();
	consoleLogger.setFilename("consoleLog.txt");
	consoleLogger.startThread();

	connectionThread = new std::thread(&ofApp::parseUdp, this);
	//Start up lsl connection on a seperate thread
	lsl.start();

	//mySerial.setup("COM35", 115200);
}

//--------------------------------------------------------------
void ofApp::parseUdp() {
	static char udpMessage[100000];

	string ip;
	int port;
	int msgSize;
	while (runConnectionThread) {
		// Packet parsing thread loop
		if (true) {
			ofScopedLock lock(connectionLock);
			msgSize = udpConnection.Receive(udpMessage, 100000);
			udpConnection.GetRemoteAddr(ip, port);
		}
		if (port > 0 && msgSize > 0) {
			//cout << "Remote ip: " << ip << ", port: " << port << " , size: " << msgSize << endl;
			//consoleLogger.push("Remote ip: " + ip + ", port: " + ofToString(port) + " , size: " + ofToString(msgSize) + '\n');
			
			if (checkDeviceList(ip)) {
				// Device is selected, process the message!
				string message = udpMessage;
				messageLen = message.length();
				vector<string> splitPacket = ofSplitString(message, ofToString(EmotiBitPacket::PACKET_DELIMITER_CSV));	// split data into separate value pairs
				for (int i = 0; i < splitPacket.size(); i++) {
					if (splitPacket.at(i).find(",,") < splitPacket.at(i).size()) {
						malformedMessages++;
						cout << "**** MALFORMED MESSAGE " << malformedMessages << ", " << messageLen << " ',,' ****" << endl;
						cout << splitPacket.at(i) << endl;
					}
					if (splitPacket.at(i).length() > 0) {
						parsePacket(splitPacket.at(i));
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::parseSerial() {
	// Read the incoming data
	bool noData = false;
	int myByte;
	while (!noData) {
		myByte = mySerial.readByte();
		if (myByte == OF_SERIAL_NO_DATA) {
			noData = true;
		}
		else if (myByte == OF_SERIAL_ERROR)
		{
			printf("an error occurred");
			noData = true;
		}
		else {
			if ((char)myByte != EmotiBitPacket::PACKET_DELIMITER_CSV) {
				// Add the char to the incoming string until we have a line break
				stringData = stringData + (char)myByte;
			}
			else {
				// If we finished a data line, parse the data
				parsePacket(stringData);
				stringData = "";
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::parsePacket(string packet) {

	dataLogger.push(packet);
	dataLogger.push(ofToString(EmotiBitPacket::PACKET_DELIMITER_CSV));
	ofToInt("1");

	vector<string> splitPacket = ofSplitString(packet, ",");	// split data into separate value pairs

	EmotiBitPacket::Header packetHeader;
	if (!EmotiBitPacket::getHeader(splitPacket, packetHeader)) {
		malformedMessages++;
		cout << "**** MALFORMED MESSAGE " << malformedMessages << ", " << messageLen << " ****" << endl;
		cout << packet << endl;
		return;
	}

	// process fast response packet types
	if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::REQUEST_DATA) == 0) {
		parseIncomingRequestData(packetHeader, splitPacket); // ToDo
	}
	else if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::ACK) == 0) {
		parseIncomingAck(splitPacket); // ToDo
	}
	else {
		// add it to the buffer of messages to parse
		// messageBuffer is thread-safe
		messageBuffer.push_back(packet);
	}
}

void ofApp::processSlowResponseMessage(string packet) {
	vector<string> splitPacket = ofSplitString(packet, ",");	// split data into separate value pairs
	processSlowResponseMessage(splitPacket);
}

void ofApp::processSlowResponseMessage(vector<string> splitPacket) {

	EmotiBitPacket::Header packetHeader;
	if (EmotiBitPacket::getHeader(splitPacket, packetHeader)) {
		if (packetHeader.dataLength >= MAX_BUFFER_LENGTH) {
			bufferUnderruns++;
			cout << "**** POSSIBLE BUFFER UNDERRUN EVENT " << bufferUnderruns << ", " << packetHeader.dataLength << " ****" << endl;
		}

		auto indexPtr = typeTagIndexes.find(packetHeader.typeTag);	// Check whether we're plotting this typeTage
		if (indexPtr != typeTagIndexes.end()) {	// We're plotting this packet's typeTag!
			vector<vector<float>> data;
			int w = indexPtr->second.at(0); // Scope window
			int s = indexPtr->second.at(1); // Scope
			int p = indexPtr->second.at(2); // Plot
			data.resize(typeTags.at(w).at(s).size());
			for (int n = EmotiBitPacket::Header::length; n < splitPacket.size(); n++) {
				data.at(p).emplace_back(ofToFloat(splitPacket.at(n))); // 
			}
			if (!isPaused) {
				scopeWins.at(w).scopes.at(s).updateData(data);
			}
			bufferSizes.at(w).at(s).at(p) = packetHeader.dataLength;
			dataCounts.at(w).at(s).at(p) = dataCounts.at(w).at(s).at(p) + packetHeader.dataLength;

		}
		else {
			if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::BATTERY_VOLTAGE) == 0) {
					batteryStatus.getParameter().fromString(splitPacket.at(6) + "V");
			}
			else if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::BATTERY_PERCENT) == 0) {
					batteryStatus.getParameter().fromString(splitPacket.at(6) + "%");
			}
			else if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::DATA_CLIPPING) == 0) {
				for (int n = EmotiBitPacket::Header::length; n < splitPacket.size(); n++) {
					for (int w = 0; w < typeTags.size(); w++) {
						for (int s = 0; s < typeTags.at(w).size(); s++) {
							for (int p = 0; p < typeTags.at(w).at(s).size(); p++) {
								if (splitPacket.at(n).compare(typeTags.at(w).at(s).at(p)) == 0) {
									dataClippingCount++;
									guiPanels.at(guiPanelErrors).getControl(GUI_STRING_CLIPPING_EVENTS)->setBackgroundColor(ofColor(255, 0, 0));
								}
							}
						}
					}
				}
			}
			else if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::DATA_OVERFLOW) == 0) {
				for (int n = EmotiBitPacket::Header::length; n < splitPacket.size(); n++) {
					for (int w = 0; w < typeTags.size(); w++) {
						for (int s = 0; s < typeTags.at(w).size(); s++) {
							for (int p = 0; p < typeTags.at(w).at(s).size(); p++) {
								if (splitPacket.at(n).compare(typeTags.at(w).at(s).at(p)) == 0) {
									dataOverflowCount++;
									guiPanels.at(guiPanelErrors).getControl(GUI_STRING_OVERFLOW_EVENTS)->setBackgroundColor(ofColor(255, 0, 0));
								}
							}
						}
					}
				}
			}
			else if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::RESET) == 0) {
				if (guiPanels.at(guiPanelMode).getControl("Hibernate") != NULL) {
					hibernateButton.set("Hibernate", false);
					guiPanels.at(guiPanelMode).getControl("Hibernate")->setBackgroundColor(ofColor(0, 0, 0));
					hibernateStatus.setBackgroundColor(ofColor(0, 0, 0));
					hibernateStatus.getParameter().fromString(GUI_STRING_MODE_ACTIVE);
				}
				if (guiPanels.at(guiPanelRecord).getControl("Record") != NULL) {
					recordingButton.set("Record", false);
					guiPanels.at(guiPanelRecord).getControl("Record")->setBackgroundColor(ofColor(0, 0, 0));
					recordingStatus.setBackgroundColor(ofColor(0, 0, 0));
					recordingStatus.getParameter().fromString(GUI_STRING_NOT_RECORDING);
				}
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::update() {
	if (plotUdpData) {
		//parseUdp();
		vector<string> messages = messageBuffer.get();
		for (int j = 0; j < messages.size(); j++) {
			processSlowResponseMessage(messages.at(j));
		}
	}

	if (lsl.isConnected()) {
		auto buffer = lsl.flush();

		if (buffer.size()) {
			auto sampleToUse = buffer.back();
			std::stringstream ss;
			for (auto channel : sampleToUse.sample) {
				ss << "," << ofToString(channel) ;
			}
			
			sendEmotiBitPacket(EmotiBitPacket::TypeTag::LSL_MARKER, "TSC," + ofToString(sampleToUse.timestampLocal, 7) + ",TS," + ofToString(sampleToUse.timestamp, 7) + ",LC," + ofToString(sampleToUse.localClock, 7) + ",LD"+ ss.str());
			//cout << EmotiBitPacket::TypeTag::LSL_MARKER << ",LC," << ofToString(sampleToUse.localClock, 7) << ",TSC," << ofToString(sampleToUse.timestampLocal, 7) << ",TS," << ofToString(sampleToUse.timestamp, 7) + ss.str() << endl;
		}
	}
	static uint64_t heartBeatTimer;
	int heartBeatDelay = 2000;
	if (ofGetElapsedTimeMillis() - heartBeatTimer > heartBeatDelay) {
		string localTime = ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT);
		guiPanels.at(guiPanelErrors).getControl(GUI_STRING_CLIPPING_EVENTS)->setBackgroundColor(ofColor(0, 0, 0));
		guiPanels.at(guiPanelErrors).getControl(GUI_STRING_OVERFLOW_EVENTS)->setBackgroundColor(ofColor(0, 0, 0));
		if (true) {
			sendEmotiBitPacket(EmotiBitPacket::TypeTag::HELLO_EMOTIBIT, localTime);
		}
		heartBeatTimer = ofGetElapsedTimeMillis();

		for (int w = 0; w < typeTags.size(); w++) {
			for (int s = 0; s < typeTags.at(w).size(); s++) {
				for (int p = 0; p < typeTags.at(w).at(s).size(); p++) {
					float newDataWeight = (dataFreqs.at(w).at(s).at(p) == 0) ? 1.f : 0.1f;
					dataFreqs.at(w).at(s).at(p) = smoother(dataFreqs.at(w).at(s).at(p), dataCounts.at(w).at(s).at(p) * 1000 / heartBeatDelay, newDataWeight);
					dataCounts.at(w).at(s).at(p) = 0;
				}
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofPushMatrix();
	ofTranslate(0, drawYTranslate);
	ofScale(((float)ofGetWidth()) / 1500.f, ((float)ofGetHeight()) / 900.f * drawYScale);

	for (int w = 0; w < scopeWins.size(); w++) {
		scopeWins.at(w).plot();
	}
	for (int i = 0; i < guiPanels.size(); i++) {
		guiPanels.at(i).draw();
	}

	// Draw dataFreqs and bufferSizes for each stream
	if (drawDataInfo) {
		for (int w = 0; w < typeTags.size(); w++) {
			for (int s = 0; s < typeTags.at(w).size(); s++) {
				ofPoint bl = scopeWins.at(w).scopes.at(s).getPosition().getBottomLeft();
				for (int p = 0; p < typeTags.at(w).at(s).size(); p++) {
					int padding = 10;
					int fontHeight = 9;
					ofPushMatrix();
					ofPushStyle();

					//ofScale(0.5f, 0.5f);

					ofSetColor(plotColors.at(p));
					ofTranslate(bl.x + padding, bl.y - fontHeight * typeTags.at(w).at(s).size());
					//ofScale(0.75f, 0.75f);
					ofTranslate(0, p * fontHeight);

					subLegendFont.drawString(ofToString(bufferSizes.at(w).at(s).at(p)) + " (Bffr)", 0, 0);

					ofTranslate(0, (-fontHeight) * (int)(typeTags.at(w).at(s).size() + 1));

					subLegendFont.drawString(ofToString((int)dataFreqs.at(w).at(s).at(p)) + " (Hz)", 0, 0);

					ofPopStyle();
					ofPopMatrix();
				}
			}
		}
	}
	ofPopMatrix();

	//legendFont.drawString(ofToString(ofGetFrameRate()), 100, 100);
}

//--------------------------------------------------------------
void ofApp::exit() {
	printf("exit()");
	//recordingStatus.removeListener(this, &ofApp::recordButtonPressed);
	if (true) {
		ofScopedLock lock(connectionLock);
		runConnectionThread = false;
	}

	udpConnection.Close();
	//connectionThread->join();
	//try {
	//	delete connectionThread;
	//}
	//catch (exception e) {
	//	cout << e.what();
	//}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	// Increment the timeWindow
	if (key == OF_KEY_RIGHT) { // Right Arrow
		for (int w = 0; w < scopeWins.size(); w++) {
			scopeWins.at(w).incrementTimeWindow();
		}
	}

	// Decrement the timeWindow
	if (key == OF_KEY_LEFT) { // Left Arrow
		for (int w = 0; w < scopeWins.size(); w++) {
			scopeWins.at(w).decrementTimeWindow();
		}
	}
	if (key == OF_KEY_UP) {
		drawYTranslate--;
		drawYScale = (drawYScale * 900.f + 1.f) / 900.f;
	}
	if (key == OF_KEY_DOWN) {
		drawYTranslate++;
		drawYScale = (drawYScale * 900.f - 1.f) / 900.f;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
	cout << "Key Released: " << key << "\n";

	if (key == ' ') {
		isPaused = !isPaused;
	}
	if (key == 'b') {
		vector<string> ips = getLocalIPs();
		sendBroadcast(ips.at(0));
	}
	if (key == 'r') { 
		recordingButton.set("Record", !recordingButton.get());
	}
	if (key == 'h') { 
		hibernateButton.set("Hibernate", !hibernateButton.get());
	}
	if (key == 'i') {
		drawDataInfo = !drawDataInfo;
	}
	if (key == OF_KEY_BACKSPACE || key == OF_KEY_DEL) {
		for (int w = 0; w < scopeWins.size(); w++) {
			scopeWins.at(w).clearData();
		}
	}
}


//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}

vector<string> ofApp::getLocalIPs()
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

void ofApp::sendEmotiBitPacket(string typeTag, string data, uint16_t dataLength, uint16_t protocolVersion, uint16_t dataReliability) {
	ofScopedLock lock(connectionLock);
	sendEmotiBitPacket(udpConnection, typeTag, data, dataLength, protocolVersion, dataReliability);
}

void ofApp::sendEmotiBitPacket(ofxUDPManager &udpManager, string typeTag, string data, uint16_t dataLength, uint16_t protocolVersion, uint16_t dataReliability) {
	uint32_t timestamp = ofGetElapsedTimeMillis();  // milliseconds since EmotiBit bootup
	static uint16_t packetCount;
	packetCount++;
	string message = "";
	message += ofToString(timestamp);
	message += ",";
	message += ofToString(packetCount);
	message += ",";
	message += ofToString(dataLength);
	message += ",";
	message += typeTag;
	message += ",";
	message += ofToString(protocolVersion);
	message += ",";
	message += ofToString(dataReliability);
	message += ",";
	message += data;
	message += "\n";
	udpManager.SendAll(message.c_str(), message.length());
	consoleLogger.push(message); //  << message << endl;
}

void ofApp::recordButtonPressed(bool & recording) {
	if (recording) {
		string localTime = ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT);
		sendEmotiBitPacket(EmotiBitPacket::TypeTag::RECORD_BEGIN, localTime);
	}
	else {
		string localTime = ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT);
		sendEmotiBitPacket(EmotiBitPacket::TypeTag::RECORD_END, localTime);
	}
}

void ofApp::hibernateButtonPressed(bool & hibernate) {
	if (hibernate) {
		string localTime = ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT);
		sendEmotiBitPacket(EmotiBitPacket::TypeTag::MODE_HIBERNATE, localTime);
	}
}

void ofApp::sendExperimenterNoteButton() {
	string note = userNote.getParameter().toString();
	if (note.compare("[Add a note]") != 0) {
		string localTime = ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT);
		sendEmotiBitPacket(EmotiBitPacket::TypeTag::USER_NOTE, localTime + "," + note, 2);
		//cout << note << endl;
	}
	//sendEmotiBitPacket(EmotiBitPacket::TypeTag::USER_NOTE, userNote.getParameter().toString());
}

template <class T>
vector<vector<vector<T>>> ofApp::initBuffer(vector<vector<vector<T>>> buffer) {
	buffer.resize(typeTags.size());
	for (int w = 0; w < typeTags.size(); w++) {
		buffer.at(w).resize(typeTags.at(w).size());
		for (int s = 0; s < typeTags.at(w).size(); s++) {
			buffer.at(w).at(s).resize(typeTags.at(w).at(s).size());
			for (int p = 0; p < typeTags.at(w).at(s).size(); p++) {
				buffer.at(w).at(s).at(p) = 0;
			}
		}
	}
	return buffer;
}

float ofApp::smoother(float smoothData, float newData, float newDataWeight) {
	smoothData = smoothData * (1 - newDataWeight) + newData * newDataWeight;
	return smoothData;
}

void ofApp::parseIncomingRequestData(EmotiBitPacket::Header header, vector<string> splitPacket) {
	int dataStart = EmotiBitPacket::Header::length;
	string ackData = "";
	for (int j = dataStart; j < splitPacket.size(); j++) {
		if (splitPacket.at(j).compare(EmotiBitPacket::TypeTag::TIMESTAMP_LOCAL) == 0) {
			sendEmotiBitPacket(EmotiBitPacket::TypeTag::TIMESTAMP_LOCAL, ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT));
		}
		if (splitPacket.at(j).compare(EmotiBitPacket::TypeTag::TIMESTAMP_UTC) == 0) {
			// ToDo: implement UTC timestamp
		}
		//ackData += splitPacket.at(j);
		//if (j < splitPacket.size() - 1) {
		//	ackData += ',';
		//}
	}
	if (lsl.isConnected()) {
		double lsltime = lsl::local_clock();
		sendEmotiBitPacket(EmotiBitPacket::TypeTag::TIMESTAMP_CROSS_TIME, ofToString(EmotiBitPacket::TypeTag::TIMESTAMP_LOCAL) + "," + ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT) + ",LC," + ofToString(lsltime, 7));
		//cout << EmotiBitPacket::TypeTag::TIMESTAMP_CROSS_TIME << "," << ofToString(EmotiBitPacket::TypeTag::TIMESTAMP_LOCAL) + "," + ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT) + ",LC," + ofToString(lsltime, 7) << endl;
	}
	sendEmotiBitPacket(EmotiBitPacket::TypeTag::ACK, ofToString(header.packetNumber) + ',' + header.typeTag, 2);
	//cout << EmotibitPacket::TypeTag::REQUEST_DATA << header.packetNumber << endl;
}

void ofApp::parseIncomingAck(vector<string> splitPacket) {
	int dataStart = EmotiBitPacket::Header::length;
	if (splitPacket.size() > dataStart + 1) {
		consoleLogger.push("ACK: " + splitPacket.at(dataStart) +  ", " + splitPacket.at(dataStart + 1) + "\n");
		//cout << "ACK: " << splitPacket.at(dataStart) << ", " << splitPacket.at(dataStart + 1) << endl;
		if (splitPacket.at(dataStart + 1).compare(EmotiBitPacket::TypeTag::RECORD_BEGIN) == 0) {
			if (guiPanels.at(guiPanelRecord).getControl("Record") != NULL) {
				guiPanels.at(guiPanelRecord).getControl("Record")->setBackgroundColor(ofColor(255, 0, 0));
				recordingStatus.setBackgroundColor(ofColor(255, 0, 0));
				recordingStatus.getParameter().fromString(GUI_STRING_RECORDING);
			}
		}
		else if (splitPacket.at(dataStart + 1).compare(EmotiBitPacket::TypeTag::RECORD_END) == 0) {
			if (guiPanels.at(guiPanelRecord).getControl("Record") != NULL) {
				guiPanels.at(guiPanelRecord).getControl("Record")->setBackgroundColor(ofColor(0, 0, 0));
				recordingStatus.setBackgroundColor(ofColor(0, 0, 0));
				recordingStatus.getParameter().fromString(GUI_STRING_NOT_RECORDING);
			}
		}
		else if (splitPacket.at(dataStart + 1).compare(EmotiBitPacket::TypeTag::USER_NOTE) == 0) {
			userNote.getParameter().fromString("[Add a note]");
		}
		else if (splitPacket.at(dataStart + 1).compare(EmotiBitPacket::TypeTag::MODE_HIBERNATE) == 0) {
			if (guiPanels.at(guiPanelMode).getControl("Hibernate") != NULL) {
				guiPanels.at(guiPanelMode).getControl("Hibernate")->setBackgroundColor(ofColor(255, 0, 0));
				hibernateStatus.setBackgroundColor(ofColor(255, 0, 0));
				hibernateStatus.getParameter().fromString(GUI_STRING_MODE_HIBERNATE);
			}
			if (guiPanels.at(guiPanelRecord).getControl("Record") != NULL) {
				recordingButton.set("Record", false);
				guiPanels.at(guiPanelRecord).getControl("Record")->setBackgroundColor(ofColor(0, 0, 0));
				recordingStatus.setBackgroundColor(ofColor(0, 0, 0));
				recordingStatus.getParameter().fromString(GUI_STRING_NOT_RECORDING);
			}
		}
	}
}

bool ofApp::checkDeviceList(string ip) {
	int ipPosition = -1;
	ofScopedLock lock(connectionLock);
	for (int j = 0; j < deviceList.size(); j++) {
		if (ip.compare(deviceList.at(j).getName()) == 0) {
			ipPosition = j;
		}
	}
	if (ipPosition == -1) {
		//bool startChecked = false;
		deviceList.emplace_back(ip, false);
		if (deviceList.size() == 1) { // This is the first device in the list
																	// ToDo: perform positive confirmation that this is an EmotiBit
			deviceList.at(deviceList.size() - 1).set(true);
			changeConnection(true);
			//startChecked = true;
		}
		deviceList.at(deviceList.size() - 1).addListener(this, &ofApp::deviceSelection);
		guiPanels.at(guiPanelDevice).getGroup(GUI_DEVICE_GROUP_MENU_NAME).getGroup(GUI_DEVICE_GROUP_NAME).add(deviceList.at(deviceList.size() - 1));
	}
	if (ipPosition > -1 && deviceList.at(ipPosition).get()) {
		return true; // the device is selected
	}
	else {
		return false; // the device is not selected
	}
}

void ofApp::deviceSelection(bool & selected) {
	ofScopedLock lock(connectionLock);
	changeConnection(selected);
}

void ofApp::changeConnection(bool selected) {
	if (selected) {
		if (deviceSelected.get().compare(GUI_STRING_NO_EMOTIBIT_SELECTED) != 0) {	// If there is currently a selected IP address
																																							// Unselected it
			for (int j = 0; j < deviceList.size(); j++) {
				if (deviceSelected.get().compare(deviceList.at(j).getName()) == 0) {
					deviceList.at(j).set(false);
					udpConnection.Close();
				}
			}
		}

		// Updated the selectedIp
		for (int j = 0; j < deviceList.size(); j++) {
			if (deviceList.at(j).get()) {
				string ipAddress = deviceList.at(j).getName();
				deviceSelected.set(ipAddress);

				udpConnection.Create();
				udpConnection.SetEnableBroadcast(false);
				cout << ipAddress << endl;
				udpConnection.Connect(ipAddress.c_str(), connectionPort);
				udpConnection.SetNonBlocking(true);
			}
		}
	}
	else {
		deviceSelected.set(GUI_STRING_NO_EMOTIBIT_SELECTED);
		udpConnection.Close();

		udpConnection.Create();
		udpConnection.SetEnableBroadcast(true);
		vector<string> ips = getLocalIPs();
		vector<string> ipSplit = ofSplitString(ips.at(0), ".");
		string ipAddress = ipSplit.at(0) + "." + ipSplit.at(1) + "." + ipSplit.at(2) + ".255";
		cout << ipAddress << endl;
		udpConnection.Connect(ipAddress.c_str(), connectionPort);
		udpConnection.SetNonBlocking(true);
		sendEmotiBitPacket(udpConnection, EmotiBitPacket::TypeTag::HELLO_EMOTIBIT, ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT));
	}
}

void ofApp::sendBroadcast(string ipAddress) {
	//Net::IPAddress publicIp;
	//publicIp = ofxNet::NetworkUtils::getPublicIPAddress();

	ofxUDPManager udpBcastConnection;

	udpBcastConnection.Create();

	vector<string> ipSplit = ofSplitString(ipAddress, ".");
	for (int i = 0; i < 256; i++) {
		// This is a hack to get around WiFi UDP broadcast issues
		// ToDo: figure out a better solution to WiFi UDP broadcast issues
		ipAddress = ipSplit.at(0) + "." + ipSplit.at(1) + "." + ipSplit.at(2) + "." + ofToString(i);
		cout << ipAddress << endl;
		udpBcastConnection.SetEnableBroadcast(true);
		udpBcastConnection.Connect(ipAddress.c_str(), connectionPort);
		udpBcastConnection.SetNonBlocking(true);
		sendEmotiBitPacket(udpBcastConnection, EmotiBitPacket::TypeTag::HELLO_EMOTIBIT, ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT));
		//udpBcastConnection.Send(message.c_str(), message.length());
		udpBcastConnection.Close();
	}
}

string ofApp::ofGetTimestampString(const string& timestampFormat) {
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
	ofStringReplace(tmpTimestampFormat, "%i", ofToString(us/1000, 3, '0'));
	ofStringReplace(tmpTimestampFormat, "%f", ofToString(us, 6, '0'));

	if (strftime(buf, bufsize, tmpTimestampFormat.c_str(), &tm) != 0) {
		str << buf;
	}
	auto ret = str.str();


	return ret;
}