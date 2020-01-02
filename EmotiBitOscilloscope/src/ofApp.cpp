#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetFrameRate(30);
	ofBackground(255, 255, 255);

	writeOfxEmotiBitVersionFile();

	emotiBitWiFi.begin();	// Startup WiFi connectivity

	setupGui();
	setupOscilloscopes();
	
	logData = false;
	logConsole = false;
	dataLogger.setFilename("dataLog.txt");
	if (logData)
	{
		dataLogger.startThread();
	}
	consoleLogger.setFilename("consoleLog.txt");
	if (logConsole)
	{
		consoleLogger.startThread();
	}

	lsl.start(); //Start up lsl connection on a seperate thread
}

//--------------------------------------------------------------
void ofApp::update() {
	vector<string> infoPackets;
	emotiBitWiFi.processAdvertising(infoPackets);
	// ToDo: Handle info packets with mode change information

	updateLsl();

	vector<string> dataPackets;
	emotiBitWiFi.readData(dataPackets);
	for (string packet : dataPackets)
	{
		processSlowResponseMessage(packet);
		if (logData)
		{
			dataLogger.push(packet);
		}
	}

	updateDeviceList();
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
		ofPushStyle();
		ofSetLineWidth(5);
		guiPanels.at(i).draw();
		ofPopStyle();
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

					ofSetColor(plotColors.at(w).at(s).at(p));
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
	if (key == 'i') {
		drawDataInfo = !drawDataInfo;
	}
	if (key == OF_KEY_BACKSPACE || key == OF_KEY_DEL) {
		clearOscilloscopes();
	}
	if (key == ':')
	{
		logData = !logData;
		logConsole = !logConsole;
		if (logData)
		{
			dataLogger.startThread();
		}
		if (logConsole)
		{
			consoleLogger.startThread();
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

void ofApp::recordButtonPressed(bool & recording) {
	if (recording) {
		string localTime = ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT);
		emotiBitWiFi.sendControl(EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::RECORD_BEGIN, emotiBitWiFi.controlPacketCounter++, localTime, 1));
	}
	else {
		string localTime = ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT);
		emotiBitWiFi.sendControl(EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::RECORD_END, emotiBitWiFi.controlPacketCounter++, localTime, 1));
	}
}

void ofApp::hibernateButtonPressed(bool & hibernate) {
	if (hibernate) {
		string localTime = ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT);
		emotiBitWiFi.sendControl(EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::MODE_HIBERNATE, emotiBitWiFi.controlPacketCounter++, localTime, 1));
	}
}

void ofApp::sendExperimenterNoteButton() {
	string note = userNote.getParameter().toString();
	if (note.compare("[Add a note]") != 0) {
		vector<string> payload;
		payload.push_back(ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT));
		payload.push_back(note);
		emotiBitWiFi.sendControl(EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::USER_NOTE, emotiBitWiFi.controlPacketCounter++, payload));
	}
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

float ofApp::smoother(float smoothData, float newData, float newDataWeight) 
{
	smoothData = smoothData * (1 - newDataWeight) + newData * newDataWeight;
	return smoothData;
}

void ofApp::updateDeviceList()
{
	// Update add any missing EmotiBits on network to the device list
	// ToDo: consider subtraction of EmotiBits that are stale
	auto emotibitIps = emotiBitWiFi.getEmotiBitIPs();
	for (auto it = emotibitIps.begin(); it != emotibitIps.end(); it++)
	{
		string ip = it->first;
		bool available = it->second.isAvailable;
		bool found = false;
		// Search the GUI list to see if we're missing any EmotiBits
		for (auto device = deviceList.begin(); device != deviceList.end(); device++)
		{
			if (ip.compare(device->getName()) == 0)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			deviceList.emplace_back(ip, false);	// Add a new device (unchecked)
			//deviceList.at(deviceList.size() - 1).addListener(this, &ofApp::deviceSelection);	// Attach a listener
			guiPanels.at(guiPanelDevice).getGroup(GUI_DEVICE_GROUP_MENU_NAME).getGroup(GUI_DEVICE_GROUP_NAME).add(deviceList.at(deviceList.size() - 1));
			if (emotibitIps.size() == 1 && deviceList.size() == 1)  // This is the first device in the list
			{
				// There is one device on the network and it's the first device in the list
				// connect
				deviceList.at(deviceList.size() - 1).set(true);
			}
		}
	}

	// Update selected device
	if (emotiBitWiFi.isConnected)
	{
		deviceSelected.set(emotiBitWiFi.connectedEmotibitIp);
	}
	else
	{
		deviceSelected.set(GUI_STRING_NO_EMOTIBIT_SELECTED);
	}

	// Update deviceList to reflect availability and connection status
	for (auto device = deviceList.begin(); device != deviceList.end(); device++)
	{
		// Update availability color
		string ip = device->getName();
		bool available = false;
		try { available = emotibitIps.at(ip).isAvailable; }
		catch (const std::out_of_range& oor) { oor; } // ignore exception
		ofColor textColor;
		if (available || ip.compare(emotiBitWiFi.connectedEmotibitIp) == 0)
		{
			textColor = deviceAvailableColor;
		}
		else
		{
			textColor = deviceNotAvailableColor;
		}
		guiPanels.at(guiPanelDevice).getGroup(GUI_DEVICE_GROUP_MENU_NAME).getGroup(GUI_DEVICE_GROUP_NAME).getControl(ip)->setTextColor(textColor);

		// Update device connection status checkbox
		bool selected = device->get();
		if (ip.compare(emotiBitWiFi.connectedEmotibitIp) == 0 && !selected)
		{
			// Connected to device -- checkbox needs to be checked
			ofRemoveListener(deviceGroup.parameterChangedE(), this, &ofApp::deviceGroupSelection);
			device->set(true);
			ofAddListener(deviceGroup.parameterChangedE(), this, &ofApp::deviceGroupSelection);
		}
		else if (ip.compare(emotiBitWiFi.connectedEmotibitIp) != 0 && selected)
		{
			// Not connected to device -- checkbox needs to be unchecked
			ofRemoveListener(deviceGroup.parameterChangedE(), this, &ofApp::deviceGroupSelection);
			device->set(false);
			ofAddListener(deviceGroup.parameterChangedE(), this, &ofApp::deviceGroupSelection);
			clearOscilloscopes();
		}
	}
}

//void ofApp::deviceGroupSelection(ofParameter<bool> &device)
void ofApp::deviceGroupSelection(ofAbstractParameter& device)
{
	string ip = device.getName();
	bool selected = device.cast<bool>().get();
	if (selected)
	{
		// device selected
		auto emotibitIps = emotiBitWiFi.getEmotiBitIPs();
		bool available = false;
		try	{	available = emotibitIps.at(ip).isAvailable;	}
		catch (const std::out_of_range& oor) { oor; } // ignore exception
		if (available)
		{
			// Only respond to available selections
			if (ip.compare(emotiBitWiFi.connectedEmotibitIp) == 0)
			{
				// We're already connected to the selected IP, so enjoy a cold beer
			}
			else
			{
				if (emotiBitWiFi.isConnected)
				{
					// If we're already connected, first disconnect
					emotiBitWiFi.disconnect();
				}
				// ToDo: consider if we need a delay here
				emotiBitWiFi.connect(ip);
				clearOscilloscopes();
			}
		}
	}
	else	
	{
		// device unselected
		if (emotiBitWiFi.connectedEmotibitIp.compare(ip) == 0)
		{
			// The device we're connected to has been unchecked... disconnect
			emotiBitWiFi.disconnect();
		}
	}
}

void ofApp::sendDataSelection(bool & selected) {
	if (selected) {
		if (sendOptionSelected.get().compare(GUI_STRING_SEND_DATA_NONE) != 0) {	// If there is currently a selected IP address
			// Unselected it
			for (int j = 0; j < sendDataList.size(); j++) {
				if (sendOptionSelected.get().compare(sendDataList.at(j).getName()) == 0) {
					sendDataList.at(j).set(false);
				}
			}
		}

		// Updated the selected output
		for (int j = 0; j < sendDataList.size(); j++) {
			if (sendDataList.at(j).get()) {
				string output = sendDataList.at(j).getName();
				sendOptionSelected.set(output);
			}
		}
	}
	else {
		sendOptionSelected.set(GUI_STRING_SEND_DATA_NONE);
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
			for (int n = EmotiBitPacket::headerLength; n < splitPacket.size(); n++) {
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
				for (int n = EmotiBitPacket::headerLength; n < splitPacket.size(); n++) {
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
				for (int n = EmotiBitPacket::headerLength; n < splitPacket.size(); n++) {
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
				if (guiPanels.at(guiPanelMode).getControl(GUI_STRING_CONTROL_HIBERNATE) != NULL) {
					hibernateButton.set(GUI_STRING_CONTROL_HIBERNATE, false);
					guiPanels.at(guiPanelMode).getControl(GUI_STRING_CONTROL_HIBERNATE)->setBackgroundColor(ofColor(0, 0, 0));
					hibernateStatus.setBackgroundColor(ofColor(0, 0, 0));
					hibernateStatus.getParameter().fromString(GUI_STRING_MODE_ACTIVE);
				}
				if (guiPanels.at(guiPanelRecord).getControl(GUI_STRING_CONTROL_RECORD) != NULL) {
					recordingButton.set(GUI_STRING_CONTROL_RECORD, false);
					guiPanels.at(guiPanelRecord).getControl(GUI_STRING_CONTROL_RECORD)->setBackgroundColor(ofColor(0, 0, 0));
					recordingStatus.setBackgroundColor(ofColor(0, 0, 0));
					recordingStatus.getParameter().fromString(GUI_STRING_NOT_RECORDING);
				}
			}
		}
	}
}

void ofApp::setupGui()
{

	ofSetWindowTitle("EmotiBit Oscilloscope (v" + ofxEmotiBitVersion + ")");

	legendFont.load(ofToDataPath("verdanab.ttf"), 11, true, true);
	axesFont.load(ofToDataPath("verdana.ttf"), 10, true, true);
	subLegendFont.load(ofToDataPath("verdana.ttf"), 7, true, true);


	recordingButton.addListener(this, &ofApp::recordButtonPressed);
	hibernateButton.addListener(this, &ofApp::hibernateButtonPressed);
	sendUserNote.addListener(this, &ofApp::sendExperimenterNoteButton);

	int sendDataWidth = 180;

	int guiXPos = 0;
	int guiYPos = 25;
	int guiWidth = 220;
	int guiPosInc = guiWidth + 1;
	guiPanels.resize(7);
	int p = 0;
	guiPanelDevice = p;
	guiPanels.at(guiPanelDevice).setDefaultWidth(guiWidth);
	guiPanels.at(guiPanelDevice).setDefaultHeight(guiYPos);
	guiPanels.at(guiPanelDevice).loadFont(ofToDataPath("verdanab.ttf"), 10, true, true);
	guiPanels.at(guiPanelDevice).setup("selectDevice", "junk.xml", guiXPos, -guiYPos * 2.2);
	deviceMenuGroup.setName(GUI_DEVICE_GROUP_MENU_NAME);
	deviceMenuGroup.add(deviceSelected.set("EmotiBit", GUI_STRING_NO_EMOTIBIT_SELECTED));
	deviceGroup.setName(GUI_DEVICE_GROUP_NAME);
	//deviceList.emplace_back("Message All Emotibits", true);
	//deviceGroup.add(deviceList.at(deviceList.size() - 1));
	deviceMenuGroup.add(deviceGroup);
	guiPanels.at(guiPanelDevice).add(deviceMenuGroup);
	//guiPanels.at(guiPanelDevice).getGroup(GUI_DEVICE_GROUP_MENU_NAME).getGroup(GUI_DEVICE_GROUP_NAME)
	ofAddListener(deviceGroup.parameterChangedE(), this, &ofApp::deviceGroupSelection);
	p++;
	guiXPos += guiWidth + 1;
	guiWidth = 210;
	guiPanelRecord = p;
	guiPanels.at(guiPanelRecord).setDefaultWidth(guiWidth);
	guiPanels.at(guiPanelRecord).setup("startRecording", "junk.xml", guiXPos, -guiYPos);
	guiPanels.at(guiPanelRecord).add(recordingButton.set(GUI_STRING_CONTROL_RECORD, false));
	guiPanels.at(guiPanelRecord).getControl(GUI_STRING_CONTROL_RECORD)->setTextColor(recordControlColor); // color of label and x
	guiPanels.at(guiPanelRecord).getControl(GUI_STRING_CONTROL_RECORD)->setFillColor(recordControlColor); // fill color of checkbox
	//guiPanels.at(guiPanelRecord).getControl(GUI_STRING_CONTROL_RECORD)->loadFont(ofToDataPath("verdanab.ttf"), 11, true, true); // Seems to affect all guiPanels
	//guiPanels.at(guiPanelRecord).getControl(GUI_STRING_CONTROL_RECORD)->setUseTTF(true);
	//guiPanels.at(guiPanelRecord).getControl(GUI_STRING_CONTROL_RECORD)->setBackgroundColor(ofColor(0,0,255)); // background of whole control
	//guiPanels.at(guiPanelRecord).getControl(GUI_STRING_CONTROL_RECORD)->setHeaderBackgroundColor(ofColor(255,255,0)); // not cear what this does
	//guiPanels.at(guiPanelRecord).getControl(GUI_STRING_CONTROL_RECORD)->setBorderColor(ofColor(0,255,0)); // not clear what this does
	//guiPanels.at(guiPanelRecord).getControl(GUI_STRING_CONTROL_RECORD)->setSize(5, 10); // size of whole field
	guiPanels.at(guiPanelRecord).add(recordingStatus.setup("Status", GUI_STRING_NOT_RECORDING));
	//guiPanels.at(0).getControl(GUI_STRING_CONTROL_RECORD)->setSize(guiWidth, guiYPos * 2);

	p++;
	guiXPos += guiWidth + 1;
	guiWidth = 170;
	guiPanelMode = p;
	guiPanels.at(guiPanelMode).setDefaultWidth(guiWidth);
	guiPanels.at(guiPanelMode).setup(GUI_STRING_CONTROL_HIBERNATE, "junk.xml", guiXPos, -guiYPos);
	guiPanels.at(guiPanelMode).add(hibernateButton.set(GUI_STRING_CONTROL_HIBERNATE, false));
	guiPanels.at(guiPanelMode).getControl(GUI_STRING_CONTROL_HIBERNATE)->setTextColor(hibernateControlColor); // color of label and x
	guiPanels.at(guiPanelMode).getControl(GUI_STRING_CONTROL_HIBERNATE)->setFillColor(hibernateControlColor); // fill color of checkbox
	guiPanels.at(guiPanelMode).add(hibernateStatus.setup("Mode", GUI_STRING_MODE_ACTIVE));
	p++;
	guiXPos += guiWidth + 1;
	guiWidth = 210;
	guiPanelLevels = p;
	guiPanels.at(guiPanelLevels).setDefaultWidth(guiWidth);
	guiPanels.at(guiPanelLevels).setup("batteryStatus", "junk.xml", guiXPos, -guiYPos);
	guiPanels.at(guiPanelLevels).add(batteryStatus.setup("Battery Level", "?"));
	guiPanels.at(guiPanelLevels).add(sdCardStatus.setup("SD Card Remaining", "93%"));
	p++;
	guiXPos += guiWidth + 1;
	guiWidth = 200;
	guiPanelErrors = p;
	guiPanels.at(guiPanelErrors).setDefaultWidth(guiWidth);
	guiPanels.at(guiPanelErrors).setup("errorStatus", "junk.xml", guiXPos, -guiYPos);
	guiPanels.at(guiPanelErrors).add(dataClippingCount.set(GUI_STRING_CLIPPING_EVENTS, 0, 0, 0));
	guiPanels.at(guiPanelErrors).add(dataOverflowCount.set(GUI_STRING_OVERFLOW_EVENTS, 0, 0, 0));
	p++;
	guiXPos += guiWidth + 1;
	guiWidth = 200;
	guiPanelUserNote = p;
	guiPanels.at(guiPanelUserNote).setDefaultWidth(ofGetWindowWidth() - guiXPos - sendDataWidth);
	guiPanels.at(guiPanelUserNote).setup("userNote", "junk.xml", guiXPos, -guiYPos);
	guiPanels.at(guiPanelUserNote).add(userNote.setup("Note:", "[Add a note]"));
	guiPanels.at(guiPanelUserNote).add(sendUserNote.setup(GUI_STRING_NOTE_BUTTON));
	guiPanels.at(guiPanelUserNote).getControl(GUI_STRING_NOTE_BUTTON)->setTextColor(noteControlColor); // color of label and x
	guiPanels.at(guiPanelUserNote).getControl(GUI_STRING_NOTE_BUTTON)->setFillColor(noteControlColor); // fill color of checkbox

	p++;
	guiPanelSendData = p;
	guiWidth = sendDataWidth;
	guiXPos = ofGetWindowWidth() - guiWidth - 1;
	guiPanels.at(guiPanelSendData).setDefaultWidth(guiWidth);
	guiPanels.at(guiPanelSendData).setup("sendData", "junk.xml", guiXPos, -guiYPos * 2.2);
	sendDataMenuGroup.setName(GUI_SEND_DATA_MENU_NAME);
	sendDataMenuGroup.add(sendOptionSelected.set(GUI_STRING_SEND_DATA_VIA, GUI_STRING_SEND_DATA_NONE));
	sendDataGroup.setName(GUI_OUTPUT_GROUP_NAME);
	sendDataMenuGroup.add(sendDataGroup);
	guiPanels.at(guiPanelSendData).add(sendDataMenuGroup);
	sendDataOptions = {
		GUI_STRING_SEND_DATA_OSC,
		GUI_STRING_SEND_DATA_LSL,
		GUI_STRING_SEND_DATA_TCP,
		GUI_STRING_SEND_DATA_UDP,
		GUI_STRING_SEND_DATA_MQTT
	};
	for (int j = 0; j < sendDataOptions.size(); j++) {
		sendDataList.emplace_back(sendDataOptions.at(j), false);
		sendDataList.at(sendDataList.size() - 1).addListener(this, &ofApp::sendDataSelection);
		//sendDataGroup.add(sendDataList.at(sendDataList.size() - 1));
		guiPanels.at(guiPanelSendData).getGroup(GUI_SEND_DATA_MENU_NAME).getGroup(GUI_OUTPUT_GROUP_NAME).add(sendDataList.at(sendDataList.size() - 1));
	}
	guiPanels.at(guiPanelSendData).getGroup(GUI_SEND_DATA_MENU_NAME).getGroup(GUI_OUTPUT_GROUP_NAME).minimize();
	//guiPanels.at(p).minimize();
	//guiPanels.at(p).minimizeAll();

}
void ofApp::setupOscilloscopes() 
{

	typeTags = vector<vector<vector<string>>>
	{
		{ // scope panel 1
			{ EmotiBitPacket::TypeTag::PPG_RED },
			{ EmotiBitPacket::TypeTag::PPG_INFRARED },
			{ EmotiBitPacket::TypeTag::PPG_GREEN },
			{ EmotiBitPacket::TypeTag::EDA },
			//{ EmotiBitPacket::TypeTag::EDL, EmotiBitPacket::TypeTag::EDR },
			{ EmotiBitPacket::TypeTag::HUMIDITY_0}
		},
		{ // scope panel 2
			{ EmotiBitPacket::TypeTag::ACCELEROMETER_X, EmotiBitPacket::TypeTag::ACCELEROMETER_Y, EmotiBitPacket::TypeTag::ACCELEROMETER_Z },
			{ EmotiBitPacket::TypeTag::GYROSCOPE_X, EmotiBitPacket::TypeTag::GYROSCOPE_Y, EmotiBitPacket::TypeTag::GYROSCOPE_Z },
			{ EmotiBitPacket::TypeTag::MAGNETOMETER_X, EmotiBitPacket::TypeTag::MAGNETOMETER_Y, EmotiBitPacket::TypeTag::MAGNETOMETER_Z },
			{ EmotiBitPacket::TypeTag::TEMPERATURE_0 },
			{ EmotiBitPacket::TypeTag::THERMOPILE}
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
			{ 25.f },
			{ 25.f },
			{ 25.f },
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
			{ "PPG:RED" },
			{ "PPG:IR" },
			{ "PPG:GRN" },
			{ "EDA" },
			//{ "EDL", "EDR" },
			{ "HUMIDITY" }
		},
		{ // scope panel 2
			{ "ACC:X", "ACC:Y", "ACC:Z" },
			{ "GYRO:X", "GYRO:Y", "GYRO:Z" },
			{ "MAG:X", "MAG:Y", "MAG:Z" },
			{ "TEMP" },
			{ "THERM" }
		}
	};
	yLims = vector<vector<vector<float>>>
	{
		{ // scope panel 1
			{  0.f,0.f  },
			{  0.f,0.f  },
			{  0.f,0.f  },
			{ 0.f,0.f },
			//{ -0.01f, 3.31f },
			{ 0.f, 0.f  }
		},
		{ // scope panel 2
			{  -8.f, 8.f  },
			{  -1000.f, 1000.f  },
			{  0.f,0.f  },
			{  0.f,0.f  },
			{  0.f,0.f  }
		}
	};

	vector<vector<float>> minYSpans = vector<vector<float>>
	{
		{ // scope panel 1
			{  0.f },
			{  0.f },
			{  0.f },
			{ 0.01f },
			{ 1.f  }
		},
		{ // scope panel 2
			{ 0.f },
			{ 0.f },
			{ 50.f },
			{ 0.5f },
			{ 2.f }
		}
	};

	plotColors = vector<vector<vector<ofColor>>>
	{
		{ // scope panel 1
			{ofColor(255, 69, 78)},
			{ofColor(128, 75, 181)},
			{ofColor(120, 209, 192)},
			{ofColor(21, 73, 130)},
			{ofColor(125, 184, 234)}
		},
		{ // scope panel 2
			{ofColor(255, 115, 0), ofColor(1, 204, 115), ofColor(4, 107, 183)},
			{ofColor(255, 115, 0), ofColor(1, 204, 115), ofColor(4, 107, 183)},
			{ofColor(255, 115, 0), ofColor(1, 204, 115), ofColor(4, 107, 183)},
			{ofColor(234, 174, 68)},
			{ofColor(239, 97, 82)}
		}
	};

	//plotColors = { ofColor(0,0,0), ofColor(255,0,0) , ofColor(0,191,0), ofColor(0,0,255) };
	float timeWindow = 20.; // seconds

	int guiHeight = guiPanels.at(guiPanels.size() - 1).getPosition().y + guiPanels.at(guiPanels.size() - 1).getHeight();
	ofRectangle scopeArea = ofRectangle(ofPoint(0, guiHeight), ofPoint(ofGetWidth() / 2, ofGetHeight()));
	ofRectangle scopeArea2 = ofRectangle(ofPoint(ofGetWidth() / 2, guiHeight), ofPoint(ofGetWidth(), ofGetHeight()));


	scopeWins.emplace_back(plotNames.at(0).size(), scopeArea, legendFont); // Setup the multiScope panel
	scopeWins.emplace_back(plotNames.at(1).size(), scopeArea2, legendFont); // Setup the multiScope panel

	for (int w = 0; w < plotNames.size(); w++) {
		for (int s = 0; s < plotNames.at(w).size(); s++) {
			scopeWins.at(w).scopes.at(s).setup(timeWindow, samplingFreqs.at(w).at(s), plotNames.at(w).at(s), plotColors.at(w).at(s),
				0, 1); // Setup each oscilloscope panel
			if (yLims.at(w).at(s).at(0) == yLims.at(w).at(s).at(1)) {
				scopeWins.at(w).scopes.at(s).autoscaleY(true, minYSpans.at(w).at(s));
			}
			else {
				scopeWins.at(w).scopes.at(s).setYLims(pair<float, float>(yLims.at(w).at(s).at(0), yLims.at(w).at(s).at(1)));
			}
		}
		scopeWins.at(w).setPlotLineWidth(3);
		scopeWins.at(w).setAxesFont(axesFont);

		selectedScope = 0; // Select all scopes for increment/decrement

		isPaused = false;

	}
}

void ofApp::updateLsl()
{
	if (lsl.isConnected()) {
		auto buffer = lsl.flush();

		if (buffer.size()) {
			auto sampleToUse = buffer.back();
			std::stringstream ss;
			for (auto channel : sampleToUse.sample) {
				ss << "," << ofToString(channel);
			}

			vector<string> payload;

			payload.clear();
			payload.push_back("TSC");
			payload.push_back(ofToString(sampleToUse.timestampLocal, 7));
			payload.push_back("TS");
			payload.push_back(ofToString(sampleToUse.timestamp, 7));
			payload.push_back("LC");
			payload.push_back(ofToString(sampleToUse.localClock, 7));
			payload.push_back("LD");
			payload.push_back(ss.str());
			string packet = EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::LSL_MARKER, emotiBitWiFi.controlPacketCounter++, payload);
			emotiBitWiFi.sendControl(packet);
			//cout << packet;

			// ToDo: Consider if TIMESTAMP_CROSS_TIME packet sending needs to be in a different spot
			double lsltime = lsl::local_clock();
			payload.clear();
			payload.push_back(ofToString(EmotiBitPacket::TypeTag::TIMESTAMP_LOCAL));
			payload.push_back(ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT));
			payload.push_back("LC");
			payload.push_back(ofToString(lsltime, 7));
			emotiBitWiFi.sendControl(EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::TIMESTAMP_CROSS_TIME, emotiBitWiFi.controlPacketCounter++, payload));
		}
	}
}

void ofApp::clearOscilloscopes()
{
	for (int w = 0; w < scopeWins.size(); w++) {
		scopeWins.at(w).clearData();
	}
}