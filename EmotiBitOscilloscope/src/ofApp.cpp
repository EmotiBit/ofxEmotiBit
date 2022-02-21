#include "ofApp.h"
#include "ofxBiquadFilter.h"
#include <algorithm>

//--------------------------------------------------------------
void ofApp::setup() {
	ofLogToConsole();
	ofSetFrameRate(30);
	ofBackground(255, 255, 255);
	ofSetLogLevel(OF_LOG_NOTICE);
	writeOfxEmotiBitVersionFile();
	setTypeTagPlotAttributes();
	emotiBitWiFi.begin();	// Startup WiFi connectivity
	timeWindowOnSetup = 10;  // set timeWindow for setup (in seconds)
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
			dataLogger.push(packet + '\n');
		}
	}

	updateMenuButtons();
}

void ofApp::setTypeTagPlotAttributes()
{
	// ToDo: Add attributes for all streams for refactor
	{
		// Add plot attributes for THERMOPILE data
		typeTagPlotAttr attr;
		attr.plotName = "THERM";
		attr.plotColor = ofColor(239, 97, 82);
		// ToDo: someday, we can even consider a standard layout considering even plot number p -> {w,s,p}
		std::vector<int> sIdx = { 1, 3 };  // {window, scope}
		attr.scopeIdx = sIdx;
		typeTagPlotAttributes.emplace(EmotiBitPacket::TypeTag::THERMOPILE, attr);
	}

	{
		// Plot Attributes for TEMP1 data
		typeTagPlotAttr attr;
		attr.plotName = "TEMP1";
		attr.plotColor = ofColor(234, 174, 68);
		std::vector<int> sIdx = { 1, 3 };  // {window, scope}
		attr.scopeIdx = sIdx;
		typeTagPlotAttributes.emplace(EmotiBitPacket::TypeTag::TEMPERATURE_1, attr);
	}
}

void ofApp::resetScopePlot(int w, int s)
{
	scopeWins.at(w).scopes.at(s).clearData();
	scopeWins.at(w).scopes.at(s).setup(timeWindowOnSetup, samplingFreqs.at(w).at(s), plotNames.at(w).at(s), plotColors.at(w).at(s),
		0, 1);
}

void ofApp::initMetaDataBuffers()
{
	bufferSizes = initBuffer(bufferSizes);
	dataCounts = initBuffer(dataCounts);
	dataFreqs = initBuffer(dataFreqs);
}

void ofApp::resetIndexMapping()
{
	for (int w = 0; w < typeTags.size(); w++) {
		for (int s = 0; s < typeTags.at(w).size(); s++) {
			for (int p = 0; p < typeTags.at(w).at(s).size(); p++) {
				vector<int> indexes{ w, s, p };
				typeTagIndexes.emplace(typeTags.at(w).at(s).at(p), indexes);
			}
		}
	}
}



void ofApp::addDataStream(std::string typetag)
{
	if (typeTagIndexes.find(typetag) == typeTagIndexes.end())
	{
		// ToDo: Find a more elegant way to solve default plot state.
		// If adding THERM, remove default TEMP1. 
		if (typetag == EmotiBitPacket::TypeTag::THERMOPILE)
		{
			// TEMP 1 was added at setup wihout confirmation that data is present. Therefore it is removed when
			// another data stream on the same plot is detected(to resolve autoscaling issues)
			// It will be added by separate add() call if Temp1 stream is found
			removeDataStream(EmotiBitPacket::TypeTag::TEMPERATURE_1);
		}
		auto scopeIdx = typeTagPlotAttributes[typetag].scopeIdx;
		int w = scopeIdx.at(0);
		int s = scopeIdx.at(1);
		// add plot attributes to class vairables
		plotNames.at(w).at(s).emplace_back(typeTagPlotAttributes[typetag].plotName);
		plotColors.at(w).at(s).emplace_back(typeTagPlotAttributes[typetag].plotColor);
		typeTags.at(w).at(s).emplace_back(typetag);
		// new plotIdx
		int p = plotNames.at(w).at(s).size() - 1; //  Size - 1 to make sure there is no out of bounds access.
		std::vector<int> plotIdx = { w, s, p };
		// update typetag Indexing
		typeTagIndexes.emplace(typetag, plotIdx);
		// re-init metadata buffers
		initMetaDataBuffers();
		// reset the scope
		resetScopePlot(w, s);
	}
}


void ofApp::removeDataStream(std::string typetag)
{
	if (typeTagIndexes.find(typetag) != typeTagIndexes.end())
	{
		auto plotIdx = typeTagIndexes[typetag];
		// find the window, scope, plot for the stream
		int w = plotIdx.at(0);
		int s = plotIdx.at(1);
		int p = plotIdx.at(2);
		// erase the attributes from class variables
		plotNames.at(w).at(s).erase(plotNames.at(w).at(s).begin() + p);
		plotColors.at(w).at(s).erase(plotColors.at(w).at(s).begin() + p);
		typeTags.at(w).at(s).erase(typeTags.at(w).at(s).begin() + p);
		
		// recreate index mapping
		typeTagIndexes.clear();
		resetIndexMapping();

		// re-init metadata buffers
		initMetaDataBuffers();
		// reset the scope
		resetScopePlot(w, s);

		// ToDo: Find a more elegant way to handle "empty" plot buffer
		// if removing THERM, make sure TEMP1 is still in the plot buffer
		if (typetag == EmotiBitPacket::TypeTag::THERMOPILE)
		{
			addDataStream(EmotiBitPacket::TypeTag::TEMPERATURE_1);
		}
	}
}
Periodizer::Periodizer()
{

}

Periodizer::Periodizer(std::string inputAperiodicSignalIdentifier, std::string inputPeriodicSignalIdentifier, std::string outputSignalIdentifier, float defaultOutputValue)
{
	inputAperiodicSignal = inputAperiodicSignalIdentifier;
	inputPeriodicSignal = inputPeriodicSignalIdentifier;
	outputSignal = outputSignalIdentifier;
	defaultValue = defaultOutputValue;
	lastSampledValue = NAN;
}

int Periodizer::update(std::string identifier, std::vector<float> data, std::vector<float> &periodizedData)
{
	periodizedData.clear();
	// if updating the aperiodic signal value
	if (identifier.compare(inputAperiodicSignal) == 0)
	{
		if (data.size() > 0)
		{
			lastSampledValue = data.back();
			return 0;
		}
	}
	// if updating the periodic output
	else
	{
		if (identifier.compare(inputPeriodicSignal) == 0)
		{
			if (isnan(defaultValue)) // HR type data. update with the lastSampledValue
			{
				periodizedData.assign(data.size(), lastSampledValue);
				return periodizedData.size();
			}
			else   // EDR type data
			{
				if (isnan(lastSampledValue))// no new EDR amplitude to plot
				{
					periodizedData.assign(data.size(), defaultValue);
					return periodizedData.size();
				}
				else // new EDR amplitude to plot
				{
					periodizedData.assign(data.size() - 1, defaultValue);
					periodizedData.insert(periodizedData.begin(), lastSampledValue);
					lastSampledValue = NAN;
					return periodizedData.size();
				}
			}
		}
	}
}
//--------------------------------------------------------------
void ofApp::draw() {
	drawOscilloscopes();
	drawConsole();
}

//--------------------------------------------------------------
void ofApp::exit() {
	printf("exit()");
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	//string note = userNote.getParameter().toString();
	//if (note.compare(GUI_STRING_EMPTY_USER_NOTE) != 0) {
	//	// Don't process keystrokes if we're typing a note
	//	// ToDo: come up with a better check
	//	return;
	//}
	ofMouseEventArgs temp;
	if (!userNote.mouseReleased(temp))
	{
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
		if (DEBUGGING)
		{
			if (key == OF_KEY_UP) {
				drawYTranslate--;
				drawYScale = (drawYScale * 900.f + 1.f) / 900.f;
			}
			if (key == OF_KEY_DOWN) {
				drawYTranslate++;
				drawYScale = (drawYScale * 900.f - 1.f) / 900.f;
			}
		}
		if (ofGetElapsedTimef() < 5) {
			// Enter special modes if keys pressed in first few seconds
			if (key == 'T')
			{
				if (!_testingHelper.testingOn)
				{
					cout << "Entering Testing Mode" << endl;
					_testingHelper.setLogFilename("testingResults.txt");
					_testingHelper.testingOn = true;

					// Remove minYspans for testing
					for (int w = 0; w < plotNames.size(); w++) {
						for (int s = 0; s < plotNames.at(w).size(); s++) {
							if (yLims.at(w).at(s).at(0) == yLims.at(w).at(s).at(1)) {
								scopeWins.at(w).scopes.at(s).autoscaleY(true);
							}
						}
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
	//cout << "Key Released: " << (char)key << endl;

	//string note = userNote.getParameter().toString();
	//if (note.compare(GUI_STRING_EMPTY_USER_NOTE) != 0 ) {
	//	// Don't process keystrokes if we're typing a note
	//	// ToDo: come up with a better check
	//	return;
	//}
	ofMouseEventArgs temp;
	if (!userNote.mouseReleased(temp))
	{
		if (key == ' ') {
			//userNote.
			isPaused = !isPaused;
		}
		if (key == 'i') {
			drawDataInfo = !drawDataInfo;
		}
		if (key == OF_KEY_BACKSPACE || key == OF_KEY_DEL) {
			clearOscilloscopes(false);
		}
		if (key == ':')
		{
			logData = !logData;
			logConsole = !logConsole;
			cout << "Data logging: " << logData << endl;
			if (logData)
			{
				dataLogger.startThread();
			}
			else
			{
				dataLogger.stopThread();
			}
			if (logConsole)
			{
				consoleLogger.startThread();
			}
			else
			{
				consoleLogger.stopThread();
			}
		}
		if (key == 'D')
		{
			DEBUGGING = !DEBUGGING;
		}
		if (DEBUGGING) {
			if (key == 'l')
			{
				int w = 0;
				int s = 3;
				int p = 0;
				vector<int> indexes{ w, s, p };
				typeTagIndexes.erase(typeTags.at(w).at(s).at(p));
				typeTags.at(w).at(s).at(p) = EmotiBitPacket::TypeTag::EDL;
				typeTagIndexes.emplace(typeTags.at(w).at(s).at(p), indexes);
				plotNames.at(w).at(s).at(p) = "EDL";
				scopeWins.at(w).scopes.at(s).setVariableNames(plotNames.at(w).at(s));
				scopeWins.at(w).scopes.at(s).autoscaleY(true, 0.f);
			}
			if (key == 'r')
			{
				int w = 0;
				int s = 3;
				int p = 0;
				vector<int> indexes{ w, s, p };
				typeTagIndexes.erase(typeTags.at(w).at(s).at(p));
				typeTags.at(w).at(s).at(p) = EmotiBitPacket::TypeTag::EDR;
				typeTagIndexes.emplace(typeTags.at(w).at(s).at(p), indexes);
				plotNames.at(w).at(s).at(p) = "EDR";
				scopeWins.at(w).scopes.at(s).setVariableNames(plotNames.at(w).at(s));
				scopeWins.at(w).scopes.at(s).autoscaleY(true, 0.f);
			}
			if (key == 'a')
			{
				int w = 0;
				int s = 3;
				int p = 0;
				vector<int> indexes{ w, s, p };
				typeTagIndexes.erase(typeTags.at(w).at(s).at(p));
				typeTags.at(w).at(s).at(p) = EmotiBitPacket::TypeTag::EDA;
				typeTagIndexes.emplace(typeTags.at(w).at(s).at(p), indexes);
				plotNames.at(w).at(s).at(p) = "EDA";
				scopeWins.at(w).scopes.at(s).setVariableNames(plotNames.at(w).at(s));
				scopeWins.at(w).scopes.at(s).autoscaleY(true, 0.f);
			}
		}
		else if (_testingHelper.testingOn)
		{
			if (key == 'p')
			{
				_testingHelper.recordPpgResult();
			}
			if (key == 'l')
			{
				_testingHelper.pushEdlEdrResult();
			}
			if (key == 'L')
			{
				_testingHelper.popEdlEdrResult();
			}
			if (key == 'r')
			{
				_testingHelper.pushEdrP2pResult();
			}
			if (key == 'R')
			{
				_testingHelper.popEdrP2pResult();
			}
			if (key == 't')
			{
				_testingHelper.pushThermopileResult();
			}
			if (key == 'T')
			{
				_testingHelper.popThermopileResult();
			}
			if (key == 'c')
			{
				_testingHelper.clearAllResults();
			}
		}
		else
		{
			if (key == 'S')
			{
				ofxMultiScope::saveScopeSettings(scopeWins);
			}
			else if (key == 'L')
			{
				scopeWins = ofxMultiScope::loadScopeSettings();
				plotIdIndexes = ofxMultiScope::getPlotIdIndexes();
			}
			else if (key == 'P')
			{
				string patchboardFile = "oscOutputSettings.xml";
				oscPatchboard.loadFile(ofToDataPath(patchboardFile));
				oscSender.clear();
				try
				{
					cout << "Starting OSC: " << oscPatchboard.settings.output["ipAddress"] 
						<< "," << ofToInt(oscPatchboard.settings.output["port"]) << endl;
					oscSender.setup(oscPatchboard.settings.output["ipAddress"], ofToInt(oscPatchboard.settings.output["port"]));
					sendOsc = true;
				}
				catch (exception e) {}
			}
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

void ofApp::sendExperimenterNoteButton() {
	string note = userNote.getParameter().toString();
	if (note.compare(GUI_STRING_EMPTY_USER_NOTE) != 0 && emotiBitWiFi.isConnected()) {
		vector<string> payload;
		payload.push_back(ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT));
		payload.push_back(note);
		emotiBitWiFi.sendControl(EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::USER_NOTE, emotiBitWiFi.controlPacketCounter++, payload));
		userNote.getParameter().fromString(GUI_STRING_EMPTY_USER_NOTE);
	}

	if (_testingHelper.testingOn)
	{
		_testingHelper.updateSerialNumber(note);
		//_testingHelper.updateTestStatus(note);
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
			guiPanels.at(guiPanelDevice).getGroup(GUI_DEVICE_GROUP_NAME).add(deviceList.at(deviceList.size() - 1));
			if (emotibitIps.size() == 1 && deviceList.size() == 1)  // This is the first device in the list
			{
				// There is one device on the network and it's the first device in the list
				// connect
				deviceList.at(deviceList.size() - 1).set(true);
			}
		}
	}

	// Update selected device
	if (emotiBitWiFi.isConnected())
	{
		deviceSelected.setup(GUI_STRING_EMOTIBIT_SELECTED, emotiBitWiFi.connectedEmotibitIp);
	}
	else
	{
		deviceSelected.setup(GUI_STRING_EMOTIBIT_SELECTED, GUI_STRING_NO_EMOTIBIT_SELECTED);
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
			textColor = notAvailableColor;
		}
		guiPanels.at(guiPanelDevice).getGroup(GUI_DEVICE_GROUP_NAME).getControl(ip)->setTextColor(textColor);

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
			clearOscilloscopes(true);
		}
	}
}

void ofApp::powerModeSelection(ofAbstractParameter& mode)
{
	// Remove listener during list management
	ofRemoveListener(powerModeGroup.parameterChangedE(), this, &ofApp::powerModeSelection);

	bool selected = mode.cast<bool>().get();
	if (selected)
	{
		// Box checked

		// Unselect other options
		for (auto option = powerModeList.begin(); option != powerModeList.end(); option++)
		{
			if (option->getName().compare(mode.getName()) != 0)
			{
				option->set(false);
			}
		}

		string packet;
		string localTime = ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT);
		if (mode.getName().compare(GUI_STRING_NORMAL_POWER) == 0)
		{
			_powerMode = PowerMode::NORMAL_POWER;
			packet = EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::MODE_NORMAL_POWER, 
				emotiBitWiFi.controlPacketCounter++, localTime, 1);
		}
		else if (mode.getName().compare(GUI_STRING_LOW_POWER) == 0)
		{
			_powerMode = PowerMode::LOW_POWER;
			packet = EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::MODE_LOW_POWER, 
				emotiBitWiFi.controlPacketCounter++, localTime, 1);
		}
		else if (mode.getName().compare(GUI_STRING_WIRELESS_OFF) == 0)
		{
			_powerMode = PowerMode::WIRELESS_OFF;
			packet = EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::MODE_WIRELESS_OFF, 
				emotiBitWiFi.controlPacketCounter++, localTime, 1);
		}
		else if (mode.getName().compare(GUI_STRING_HIBERNATE) == 0)
		{
			_powerMode = PowerMode::HIBERNATE;
			packet = EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::MODE_HIBERNATE, 
				emotiBitWiFi.controlPacketCounter++, localTime, 1);
		}
		if (packet.length() > 0)
		{
			emotiBitWiFi.sendControl(packet);
		}
	}
	else
	{
		// Box unchecking not permitted on this list. Re-check box.
		mode.cast<bool>().set(true);
	}

	// Re-add the listener
	ofAddListener(powerModeGroup.parameterChangedE(), this, &ofApp::powerModeSelection);
}

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
				if (emotiBitWiFi.isConnected())
				{
					// If we're already connected, first disconnect
					emotiBitWiFi.disconnect();
					// ToDo: verify this is thread-safe
					vector<string> dataPackets;
					emotiBitWiFi.readData(dataPackets);
					emotiBitWiFi.readData(dataPackets);
					clearOscilloscopes(true);
				}
				// ToDo: consider if we need a delay here
				emotiBitWiFi.connect(ip);
				_powerMode = PowerMode::LOW_POWER;
				clearOscilloscopes(true);
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
			// ToDo: verify this is thread-safe
			vector<string> dataPackets;
			emotiBitWiFi.readData(dataPackets);
			emotiBitWiFi.readData(dataPackets);
			clearOscilloscopes(true);
		}
	}
}

void ofApp::sendDataSelection(ofAbstractParameter& output) {
	// Some outputs are disabled until code is written to support output channels

	string outputName = output.getName();
	bool selected = output.cast<bool>().get();

	for (int j = 0; j < sendDataList.size(); j++) {
		if (sendDataDisabled.at(j))
		{
			if (selected)
			{
				// set disabled outputs back to false
				sendDataList.at(j).set(false);
			}
		}
		else
		{
			if (outputName.compare(sendDataOptions.at(j)) == 0)
			{
				if (GUI_STRING_SEND_DATA_OSC.compare(sendDataOptions.at(j)) == 0)
				{
					if (selected)
					{
						string patchboardFile = "oscOutputSettings.xml";
							oscPatchboard.loadFile(ofToDataPath(patchboardFile));
							oscSender.clear();
							try
						{
							cout << "Starting OSC: " << oscPatchboard.settings.output["ipAddress"]
								<< "," << ofToInt(oscPatchboard.settings.output["port"]) << endl;
								oscSender.setup(oscPatchboard.settings.output["ipAddress"], ofToInt(oscPatchboard.settings.output["port"]));
							sendOsc = true;
						}
						catch (exception e)
						{
							cout << "OSC output setup failed " << endl;
						}
					}
					else
					{
						sendOsc = false;
					}
				}
			}
		}
	}

	return;  
#if (0)
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
#endif
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

void ofApp::processAperiodicData(std::string identifier, std::vector<float> &data)
{
	std::vector<float> periodizedData;
	// if return > 0, add the periodized data into the correct plot
	if (periodizerHeartRate.update(identifier, data, periodizedData))
	{
		if (!isPaused) 
		{
			auto indexPtr = typeTagIndexes.find(EmotiBitPacket::TypeTag::HEART_RATE);
			if (indexPtr != typeTagIndexes.end())
			{
				int w = indexPtr->second.at(0); // Scope window
				int s = indexPtr->second.at(1); // Scope
				int p = indexPtr->second.at(2); // Plot
				std::vector<std::vector<float>> plotData;
				plotData.resize(typeTags.at(w).at(s).size());
				plotData.at(p) = periodizedData;
				// Add data to oscilloscope
				scopeWins.at(w).scopes.at(s).updateData(plotData);
			}
		}
	}
}

void ofApp::processSlowResponseMessage(string packet) {
	vector<string> splitPacket = ofSplitString(packet, ",");	// split data into separate value pairs
	processSlowResponseMessage(splitPacket);
}

void ofApp::processSlowResponseMessage(vector<string> splitPacket) 
{

	EmotiBitPacket::Header packetHeader;
	if (EmotiBitPacket::getHeader(splitPacket, packetHeader)) 
	{
		if (packetHeader.dataLength >= MAX_BUFFER_LENGTH) 
		{
			bufferUnderruns++;
			cout << "**** POSSIBLE BUFFER UNDERRUN EVENT " << bufferUnderruns << ", " << packetHeader.dataLength << " ****" << endl;
		}

		if (_testingHelper.testingOn)
		{
			_testingHelper.update(splitPacket, packetHeader);
		}
		// ToDo: the second comparison is redundant with the called func. Added it here to skip a function call. Might want to change the order later.
		if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::THERMOPILE) == 0 && typeTagIndexes.find(EmotiBitPacket::TypeTag::THERMOPILE) == typeTagIndexes.end())
		{
			// Add stream to plot if data detected.
			addDataStream(EmotiBitPacket::TypeTag::THERMOPILE);
		}
		if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::TEMPERATURE_1) == 0 && typeTagIndexes.find(EmotiBitPacket::TypeTag::TEMPERATURE_1) == typeTagIndexes.end())
		{
			// Add stream to plot if data detected.
			addDataStream(EmotiBitPacket::TypeTag::TEMPERATURE_1);
		}
		auto indexPtr = typeTagIndexes.find(packetHeader.typeTag);	// Check whether we're plotting this typeTage
		if (indexPtr != typeTagIndexes.end()) 
		{	// We're plotting this packet's typeTag!
			vector<vector<float>> data;
			int w = indexPtr->second.at(0); // Scope window
			int s = indexPtr->second.at(1); // Scope
			int p = indexPtr->second.at(2); // Plot
			data.resize(typeTags.at(w).at(s).size());

			vector<string> oscAddresses;
			vector<ofxOscMessage> oscMessages;
			if (sendOsc) // Handle sending data to outputs
			{
				// ToDo: Refactor to handle data outputs in one place
				// ToDo: Make it possible to send data types that aren't being plotted (e.g. EL, ER)
				oscAddresses = oscPatchboard.patchcords[packetHeader.typeTag];
				oscMessages.resize(oscAddresses.size());
				for (auto a = 0; a < oscAddresses.size(); a++)
				{
					oscMessages.at(a).setAddress(oscAddresses.at(a));
				}
			}

			for (int n = EmotiBitPacket::headerLength; n < splitPacket.size(); n++) 
			{
				// Data for plotting in the oscilloscope
				data.at(p).emplace_back(ofToFloat(splitPacket.at(n))); 

				if (sendOsc) // Handle sending data to outputs
				{
					for (auto a = 0; a < oscMessages.size(); a++)
					{
						oscMessages.at(a).addFloatArg(data.at(p).back());
					}
				}
			}
			processAperiodicData(packetHeader.typeTag, data.at(p));

			if (sendOsc)
			{
				for (auto a = 0; a < oscMessages.size(); a++)
				{
					// ToDo: Consider using ofxOscBundle
					oscSender.sendMessage(oscMessages.at(a));
				}
			}

			if (!isPaused) {
				// Add data to oscilloscope
				scopeWins.at(w).scopes.at(s).updateData(data);
			}
			bufferSizes.at(w).at(s).at(p) = packetHeader.dataLength;
			dataCounts.at(w).at(s).at(p) += packetHeader.dataLength;

			// Sliding EDA minYspan 
			if (!DEBUGGING && packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::EDA) == 0 && data.at(p).size() > 0)
			{
				minYSpans.at(w).at(s) = 0.1f * pow(data.at(p).at(0), 1.5f);
				if (yLims.at(w).at(s).at(0) == yLims.at(w).at(s).at(1)) {
					scopeWins.at(w).scopes.at(s).autoscaleY(true, minYSpans.at(w).at(s));
				}
			}
		}
		else 
		{
			if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::BATTERY_VOLTAGE) == 0) 
			{
				deviceSelected.setup(GUI_STRING_EMOTIBIT_SELECTED, GUI_STRING_NO_EMOTIBIT_SELECTED);
				batteryStatus.setup(GUI_STRING_BATTERY_LEVEL, splitPacket.at(6) + "V");
			}
			else if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::BATTERY_PERCENT) == 0) 
			{
				batteryStatus.setup(GUI_STRING_BATTERY_LEVEL, splitPacket.at(6) + "%");
			}
			else if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::EMOTIBIT_MODE) == 0) 
			{
				processModePacket(splitPacket);
			}
			else if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::DATA_CLIPPING) == 0) 
			{
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
			else if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::DATA_OVERFLOW) == 0) 
			{
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
			else if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::RESET) == 0) 
			{
				//if (guiPanels.at(guiPanelMode).getControl(GUI_STRING_CONTROL_HIBERNATE) != NULL) {
				//	hibernateButton.set(GUI_STRING_CONTROL_HIBERNATE, false);
				//	guiPanels.at(guiPanelMode).getControl(GUI_STRING_CONTROL_HIBERNATE)->setBackgroundColor(ofColor(0, 0, 0));
				//	hibernateStatus.setBackgroundColor(ofColor(0, 0, 0));
				//	hibernateStatus.getParameter().fromString(GUI_STRING_MODE_ACTIVE);
				//}
				if (guiPanels.at(guiPanelRecord).getControl(GUI_STRING_CONTROL_RECORD) != NULL) {
					recordingButton.removeListener(this, &ofApp::recordButtonPressed);
					recordingButton.set(false);
					recordingButton.addListener(this, &ofApp::recordButtonPressed);
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

	string legendFontFilename = "verdanab.ttf";
#ifdef TARGET_MAC_OS
    ofSetDataPathRoot("../Resources/");
    cout<<"Changed the data pathroot for Release"<<endl;
#endif
	legendFont.load(ofToDataPath(legendFontFilename), 11, true, true);
	axesFont.load(ofToDataPath("verdana.ttf"), 10, true, true);
	subLegendFont.load(ofToDataPath("verdana.ttf"), 7, true, true);

	_consoleHeight = 21;


	recordingButton.addListener(this, &ofApp::recordButtonPressed);
	sendUserNote.addListener(this, &ofApp::sendExperimenterNoteButton);

	int sendDataWidth = 200;

	int guiXPos = 0;
	int guiYPos = 25;
	int guiWidth = 220;
	int guiPosInc = guiWidth + 1;
	
	guiPanels.resize(6);

	// Device Menu
	int p = 0;
	guiPanelDevice = p;
	guiPanels.at(guiPanelDevice).setDefaultWidth(guiWidth);
	guiPanels.at(guiPanelDevice).setDefaultHeight(guiYPos);
	if (legendFont.isLoaded())
	{
		// Check to see if legend font loaded before adding to gui panel to avoid blank text
		guiPanels.at(guiPanelDevice).loadFont(ofToDataPath(legendFontFilename), 10, true, true);
	}
	guiPanels.at(guiPanelDevice).setup("selectDevice", "junk.xml", guiXPos, -guiYPos);
	guiPanels.at(guiPanelDevice).add(deviceSelected.setup(GUI_STRING_EMOTIBIT_SELECTED, ":" + GUI_STRING_NO_EMOTIBIT_SELECTED));
	//deviceMenuGroup.setName(GUI_DEVICE_GROUP_MENU_NAME);
	deviceGroup.setName(GUI_DEVICE_GROUP_NAME);
	//deviceList.emplace_back("Message All Emotibits", true);
	//deviceGroup.add(deviceList.at(deviceList.size() - 1));
	//deviceMenuGroup.add(deviceGroup);
	guiPanels.at(guiPanelDevice).add(deviceGroup);
	//guiPanels.at(guiPanelDevice).getGroup(GUI_DEVICE_GROUP_MENU_NAME).getGroup(GUI_DEVICE_GROUP_NAME)
	ofAddListener(deviceGroup.parameterChangedE(), this, &ofApp::deviceGroupSelection);


	// Power Status Menu
	p++;
	guiXPos += guiWidth + 1;
	guiWidth = 259;
	guiPanelPowerStatus = p;
	guiPanels.at(guiPanelPowerStatus).setDefaultWidth(guiWidth);
	guiPanels.at(guiPanelPowerStatus).setup("powerStatus", "junk.xml", guiXPos, -guiYPos);
	guiPanels.at(guiPanelPowerStatus).add(batteryStatus.setup(GUI_STRING_BATTERY_LEVEL, ":?", guiWidth, guiYPos));
	//powerStatusMenuGroup.setName(GUI_POWER_STATUS_MENU_NAME);
	//powerStatusMenuGroup.add(batteryStatus.set(GUI_STRING_BATTERY_LEVEL, "?"));
	powerModeGroup.setName(GUI_POWER_MODE_GROUP_NAME);
	//powerStatusMenuGroup.add(powerModeGroup);
	//guiPanels.at(guiPanelPowerStatus).add(powerStatusMenuGroup);
	guiPanels.at(guiPanelPowerStatus).add(powerModeGroup);
	powerModeOptions = {
		GUI_STRING_NORMAL_POWER,
		GUI_STRING_LOW_POWER,
		GUI_STRING_WIRELESS_OFF,
		GUI_STRING_HIBERNATE
	};
	for (int j = 0; j < powerModeOptions.size(); j++) {
		powerModeList.emplace_back(powerModeOptions.at(j), false);
		//sendDataList.at(sendDataList.size() - 1).addListener(this, &ofApp::sendDataSelection);
		//sendDataGroup.add(sendDataList.at(sendDataList.size() - 1));
		guiPanels.at(guiPanelPowerStatus).getGroup(GUI_POWER_MODE_GROUP_NAME).add(powerModeList.at(powerModeList.size() - 1));
	}
	guiPanels.at(guiPanelPowerStatus).getGroup(GUI_POWER_MODE_GROUP_NAME).minimize();
	ofAddListener(powerModeGroup.parameterChangedE(), this, &ofApp::powerModeSelection);

	// Recording Status
	p++;
	guiXPos += guiWidth + 1;
	guiWidth = 269;
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
	guiPanels.at(guiPanelRecord).add(recordingStatus.setup("SD File", GUI_STRING_NOT_RECORDING));
	//guiPanels.at(0).getControl(GUI_STRING_CONTROL_RECORD)->setSize(guiWidth, guiYPos * 2);

	// Error Status
	p++;
	guiXPos += guiWidth + 1;
	guiWidth = 200;
	guiPanelErrors = p;
	guiPanels.at(guiPanelErrors).setDefaultWidth(guiWidth);
	guiPanels.at(guiPanelErrors).setup("errorStatus", "junk.xml", guiXPos, -guiYPos);
	guiPanels.at(guiPanelErrors).add(dataClippingCount.set(GUI_STRING_CLIPPING_EVENTS, 0, 0, 0));
	guiPanels.at(guiPanelErrors).add(dataOverflowCount.set(GUI_STRING_OVERFLOW_EVENTS, 0, 0, 0));
	p++;

	// User Note
	guiXPos += guiWidth + 1;
	guiWidth = 200;
	guiPanelUserNote = p;
	guiPanels.at(guiPanelUserNote).setDefaultWidth(ofGetWindowWidth() - guiXPos - sendDataWidth);
	guiPanels.at(guiPanelUserNote).setup("userNote", "junk.xml", guiXPos, -guiYPos);
	guiPanels.at(guiPanelUserNote).add(userNote.setup("Note:", GUI_STRING_EMPTY_USER_NOTE));
	guiPanels.at(guiPanelUserNote).add(sendUserNote.setup(GUI_STRING_NOTE_BUTTON));
	guiPanels.at(guiPanelUserNote).getControl(GUI_STRING_NOTE_BUTTON)->setTextColor(noteControlColor); // color of label and x
	guiPanels.at(guiPanelUserNote).getControl(GUI_STRING_NOTE_BUTTON)->setFillColor(noteControlColor); // fill color of checkbox

	// Send Data Menu
	p++;
	guiPanelSendData = p;
	guiWidth = sendDataWidth;
	guiXPos = ofGetWindowWidth() - guiWidth + 1;
	guiPanels.at(guiPanelSendData).setDefaultWidth(guiWidth);
	guiPanels.at(guiPanelSendData).setup("sendData", "junk.xml", guiXPos, -guiYPos);
	//sendDataMenuGroup.setName(GUI_SEND_DATA_MENU_NAME);
	guiPanels.at(guiPanelSendData).add(sendOptionSelected.setup(GUI_STRING_SEND_DATA_VIA, GUI_STRING_SEND_DATA_NONE));
	sendDataGroup.setName(GUI_OUTPUT_GROUP_NAME);
	//sendDataMenuGroup.add(sendDataGroup);
	guiPanels.at(guiPanelSendData).add(sendDataGroup);
	sendDataOptions = {
		GUI_STRING_SEND_DATA_OSC,
		GUI_STRING_SEND_DATA_LSL,
		GUI_STRING_SEND_DATA_TCP,
		GUI_STRING_SEND_DATA_UDP,
		GUI_STRING_SEND_DATA_MQTT
	};
	for (int j = 0; j < sendDataOptions.size(); j++) {
		sendDataList.emplace_back(sendDataOptions.at(j), false);
		//sendDataList.at(sendDataList.size() - 1).addListener(this, &ofApp::sendDataSelection);
		//sendDataGroup.add(sendDataList.at(sendDataList.size() - 1));
		guiPanels.at(guiPanelSendData).getGroup(GUI_OUTPUT_GROUP_NAME).add(sendDataList.at(sendDataList.size() - 1));
		// Disable outputs until supporting code written
		if (GUI_STRING_SEND_DATA_OSC.compare(sendDataOptions.at(j)) == 0)
		{
			sendDataDisabled.push_back(false);
			guiPanels.at(guiPanelSendData).getGroup(GUI_OUTPUT_GROUP_NAME).getControl(sendDataOptions.at(j))->setTextColor(deviceAvailableColor);
		}
		else
		{
			sendDataDisabled.push_back(true);
			guiPanels.at(guiPanelSendData).getGroup(GUI_OUTPUT_GROUP_NAME).getControl(sendDataOptions.at(j))->setTextColor(notAvailableColor);
		}
	}
	guiPanels.at(guiPanelSendData).getGroup(GUI_OUTPUT_GROUP_NAME).minimize();
	ofAddListener(sendDataGroup.parameterChangedE(), this, &ofApp::sendDataSelection);

	//Set default widths of device
	deviceSelected.setDefaultWidth(220);
	batteryStatus.setDefaultWidth(259);
}

void ofApp::updatePlotAttributeLists(std::string settingsFile)
{
	ofxXmlSettings scopeSettings;
	scopeSettings.loadFile(settingsFile);

	int nMultiScopes = scopeSettings.getNumTags("multiScope");
	samplingFreqs.resize(nMultiScopes);
	minYSpans.resize(nMultiScopes);
	plotNames.resize(nMultiScopes);
	plotColors.resize(nMultiScopes);
	yLims.resize(nMultiScopes);
	for (int m = 0; m < nMultiScopes; m++)
	{
		scopeSettings.pushTag("multiScope", m);
		int nScopes = scopeSettings.getNumTags("scope");
		samplingFreqs.at(m).resize(nScopes);
		minYSpans.at(m).resize(nScopes);
		plotNames.at(m).resize(nScopes);
		plotColors.at(m).resize(nScopes);
		yLims.at(m).resize(nScopes);
		for (int s = 0; s < nScopes; s++) 
		{
			scopeSettings.pushTag("scope", s);
			//float timeWindow = scopeSettings.getValue("timeWindow", 15.f); // maybe we keep this.
			float samplingFrequency = scopeSettings.getValue("samplingFrequency", 15.f);
			float minYSpan = scopeSettings.getValue("minYSpan", 0.f);
			float yMin = scopeSettings.getValue("yMin", 0.f);
			float yMax = scopeSettings.getValue("yMax", 0.f);
			samplingFreqs.at(m).at(s) = samplingFrequency;
			minYSpans.at(m).at(s) = minYSpan;
			vector<float> yLim = { yMin, yMax };
			yLims.at(m).at(s) = yLim;
			//samplingFreqs-2D vector
			//minYSpans-2D vector
			//plotNames-3D vector
			//plotColors-3D vector
			//yLims-3D vector
			int nPlots = scopeSettings.getNumTags("plot");
			for (int p = 0; p < nPlots; p++) {
				scopeSettings.pushTag("plot", p);
				plotNames.at(m).at(s).push_back(scopeSettings.getValue("plotName", "N/A"));
				scopeSettings.pushTag("plotColor");
				plotColors.at(m).at(s).push_back(ofColor(
					scopeSettings.getValue("r", 255),
					scopeSettings.getValue("g", 255),
					scopeSettings.getValue("b", 255)
				));
				scopeSettings.popTag(); // plotColor
				scopeSettings.popTag(); // plot p
			}
			scopeSettings.popTag(); // scope s
		}

		scopeSettings.popTag(); // multiScope m
	}
}

void ofApp::updateTypeTagList()
{
	for (int i = 0; i < plotIds.size(); i++)// for multiscopes
	{
		vector<vector<std::string>> scopeTypeTagList;
		for (int j = 0; j < plotIds.at(i).size(); j++) // for scopes
		{
			vector<std::string> plotTypeTagList;
			for (int k = 0; k < plotIds.at(i).at(j).size(); k++) // for plots
			{
				for (auto key = patchboard.patchcords.begin(); key != patchboard.patchcords.end(); key++)
				{
					// ToDo: there should be a loop here to go through all map values for a key
					if (ofToInt(key->second.back()) == plotIds.at(i).at(j).at(k))
					{
						plotTypeTagList.push_back(key->first);
					}
				}
			}
			scopeTypeTagList.push_back(plotTypeTagList);
		}
		typeTags.push_back(scopeTypeTagList);
	}
}


void ofApp::setupOscilloscopes() 
{
	if (patchboard.loadFile("inputSettings.xml"))
	{
		ofLog(OF_LOG_NOTICE, "PatchBoard succesfully loaded");
	}
	scopeWins = ofxMultiScope::loadScopeSettings();
	plotIds = ofxMultiScope::getPlotIds();
	updatePlotAttributeLists();
	updateTypeTagList();
	// Create an index mapping for each type tag
	for (int w = 0; w < typeTags.size(); w++) {
		for (int s = 0; s < typeTags.at(w).size(); s++) {
			for (int p = 0; p < typeTags.at(w).at(s).size(); p++) {
				vector<int> indexes{ w, s, p };
				typeTagIndexes.emplace(typeTags.at(w).at(s).at(p), indexes);
			}
		}
	}
	initMetaDataBuffers();
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

void ofApp::clearOscilloscopes(bool connectedDeviceUpdated)
{
	for (int w = 0; w < scopeWins.size(); w++) {
		scopeWins.at(w).clearData();
	}

	// update the Scope plots ONLY if there is update to connected device
	if (connectedDeviceUpdated)
	{
		// ToDo: think of an elegant way to set scopes to default value
		// remove only THERM. TEMP1 is default for temperature scope..
		removeDataStream(EmotiBitPacket::TypeTag::THERMOPILE);
	}
}

void ofApp::updateMenuButtons()
{
	if (!emotiBitWiFi.isConnected())
	{
		_recording = false;
		batteryStatus.setup(GUI_STRING_BATTERY_LEVEL,"?");
		_powerMode = PowerMode::length;
		guiPanels.at(guiPanelPowerStatus).getGroup(GUI_POWER_MODE_GROUP_NAME).minimize();
		//if (guiPanels.at(guiPanelDevice).getGroup(GUI_DEVICE_GROUP_MENU_NAME).getGroup(GUI_DEVICE_GROUP_NAME).isMinimized())
		//{
		//	guiPanels.at(guiPanelDevice).getGroup(GUI_DEVICE_GROUP_MENU_NAME).getGroup(GUI_DEVICE_GROUP_NAME).maximize();
		//}
		dataClippingCount = 0;
		dataOverflowCount = 0;
	}

	if (_recording)
	{
		// ToDo: also control button/checkbox
		// ofRemoveListener(deviceGroup.parameterChangedE(), this, &ofApp::deviceGroupSelection);
		recordingButton.removeListener(this, &ofApp::recordButtonPressed);
		recordingButton.set(true);
		recordingButton.addListener(this, &ofApp::recordButtonPressed);
		if (guiPanels.at(guiPanelRecord).getControl(GUI_STRING_CONTROL_RECORD) != NULL) {
			guiPanels.at(guiPanelRecord).getControl(GUI_STRING_CONTROL_RECORD)->setBackgroundColor(ofColor(0, 0, 0));
			recordingStatus.setBackgroundColor(recordControlColor);
			recordingStatus.getParameter().fromString(_recordingFilename);
		}
	}
	else
	{
		recordingButton.removeListener(this, &ofApp::recordButtonPressed);
		recordingButton.set(false);
		recordingButton.addListener(this, &ofApp::recordButtonPressed);
		if (guiPanels.at(guiPanelRecord).getControl(GUI_STRING_CONTROL_RECORD) != NULL) {
			guiPanels.at(guiPanelRecord).getControl(GUI_STRING_CONTROL_RECORD)->setBackgroundColor(ofColor(0, 0, 0));
			recordingStatus.setBackgroundColor(ofColor(0, 0, 0));
			recordingStatus.getParameter().fromString(GUI_STRING_NOT_RECORDING);
		}
	}

	// Set Power Mode Options
	string optionName = GUI_STRING_NORMAL_POWER;
	if (_powerMode == PowerMode::NORMAL_POWER)
	{
		optionName = GUI_STRING_NORMAL_POWER;
	}
	else if (_powerMode == PowerMode::LOW_POWER)
	{
		optionName = GUI_STRING_LOW_POWER;
	}
	else if(_powerMode == PowerMode::WIRELESS_OFF)
	{
		optionName = GUI_STRING_WIRELESS_OFF;
	}
	else if (_powerMode == PowerMode::HIBERNATE)
	{
		optionName = GUI_STRING_HIBERNATE;
	}
	ofRemoveListener(powerModeGroup.parameterChangedE(), this, &ofApp::powerModeSelection);
	for (auto option = powerModeList.begin(); option != powerModeList.end(); option++)
	{
		if (option->getName().compare(optionName) == 0)
		{
			option->set(true);
		}
		else
		{
			option->set(false);
		}
	}
	ofAddListener(powerModeGroup.parameterChangedE(), this, &ofApp::powerModeSelection);

	updateDeviceList();
}

void ofApp::processModePacket(vector<string> &splitPacket)
{
	size_t startIndex = EmotiBitPacket::headerLength;
	string value;

	int pos = EmotiBitPacket::getPacketKeyedValue(splitPacket, EmotiBitPacket::PayloadLabel::RECORDING_STATUS, value);
	if (pos > -1)
	{
		if (value.compare(EmotiBitPacket::TypeTag::RECORD_BEGIN) == 0)
		{
			_recording = true;
			// See if we got a filename for the file we're recording to
			if (pos + 1 < splitPacket.size())
			{
				string filename = splitPacket.at(pos + 1);
				if (filename.size() > 4 && filename.substr(filename.size() - 4, 4).compare(".csv") == 0)
				{
					_testingHelper.updateSdCardFilename(filename);
					_recordingFilename = filename;
				}
				else
				{
					_recordingFilename = GUI_STRING_RECORDING;
				}
			}
		}
		else if (value.compare(EmotiBitPacket::TypeTag::RECORD_END) == 0)
		{
			_recording = false;
		}
	}

	if (EmotiBitPacket::getPacketKeyedValue(splitPacket, EmotiBitPacket::PayloadLabel::POWER_STATUS, value) > -1)
	{
		if (value.compare(EmotiBitPacket::TypeTag::MODE_NORMAL_POWER) == 0)
		{
			_powerMode = PowerMode::NORMAL_POWER;
		}
		else if (value.compare(EmotiBitPacket::TypeTag::MODE_LOW_POWER) == 0)
		{
			_powerMode = PowerMode::LOW_POWER;
		}
		else if (value.compare(EmotiBitPacket::TypeTag::MODE_MAX_LOW_POWER) == 0)
		{
			_powerMode = PowerMode::MAX_LOW_POWER;
		}
		else if (value.compare(EmotiBitPacket::TypeTag::MODE_WIRELESS_OFF) == 0)
		{
			_powerMode = PowerMode::WIRELESS_OFF;
			emotiBitWiFi.disconnect();
		}
		else if (value.compare(EmotiBitPacket::TypeTag::MODE_HIBERNATE) == 0)
		{
			_powerMode = PowerMode::HIBERNATE;
			emotiBitWiFi.disconnect();
		}
	}
}

void ofApp::drawConsole()
{
	// Draw console
	string _consoleString = "Status: ";
	if (_testingHelper.testingOn)
	{
		_consoleString += "TESTING MODE ON -- ";
	}
	if (DEBUGGING)
	{
		_consoleString += "DEBUGGING -- ";
	}
	if (isPaused)
	{
		_consoleString += "Data visualizer paused";
	}
	else
	{
		if (_powerMode == PowerMode::LOW_POWER)
		{
			_consoleString += "Low power mode";
		}
		else if (_powerMode == PowerMode::NORMAL_POWER)
		{
			_consoleString += "Data streaming";
		}
		else if (_powerMode == PowerMode::WIRELESS_OFF)
		{
			_consoleString += "Wireless off";
		}
		else if (_powerMode == PowerMode::HIBERNATE)
		{
			_consoleString += "Hibernating";
		}
	}

	int consoleTextPadding = 3;
	ofPushStyle();
	ofFill();
	ofSetColor(0, 0, 0);
	ofDrawRectangle(0, ofGetWindowHeight() - _consoleHeight, ofGetWindowWidth(), _consoleHeight);
	ofPopStyle();
	ofPushStyle();
	ofSetColor(255, 255, 255);
    if (axesFont.isLoaded())
    {
        axesFont.drawString(_consoleString, 10, ofGetWindowHeight() - _consoleHeight / 2 + consoleTextPadding);
	}
    else
    {
        ofDrawBitmapString(_consoleString, 10, ofGetWindowHeight() - _consoleHeight / 2 + consoleTextPadding);
    }
    ofPopStyle();

}

void ofApp::drawOscilloscopes()
{
	ofPushMatrix();
	ofTranslate(0, drawYTranslate);
	ofScale(1, drawYScale);	// for debugging menus

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

		// Calculated empirical sampling freq
		static uint64_t freqCalcTimer = ofGetElapsedTimeMillis();
		uint32_t elapsedTime = ofGetElapsedTimeMillis() - freqCalcTimer;
		if (elapsedTime > 2000)
		{
			freqCalcTimer = ofGetElapsedTimeMillis();
			for (int w = 0; w < typeTags.size(); w++) {
				for (int s = 0; s < typeTags.at(w).size(); s++) {
					ofPoint bl = scopeWins.at(w).scopes.at(s).getPosition().getBottomLeft();
					for (int p = 0; p < typeTags.at(w).at(s).size(); p++) {
						dataFreqs.at(w).at(s).at(p) = 1000.f * dataCounts.at(w).at(s).at(p) / (elapsedTime);
						dataCounts.at(w).at(s).at(p) = 0;
					}
				}
			}
		}

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
                    if (subLegendFont.isLoaded())
                    {
                        subLegendFont.drawString(ofToString(bufferSizes.at(w).at(s).at(p)) + " (Bffr)", 0, 0);
                    }
                    else
                    {
                        ofDrawBitmapString(ofToString(bufferSizes.at(w).at(s).at(p)) + " (Bffr)", 0, 0);
                    }
					ofTranslate(0, (-fontHeight) * (int)(typeTags.at(w).at(s).size() + 1));
                    if (subLegendFont.isLoaded())
                    {
					    subLegendFont.drawString(ofToString(dataFreqs.at(w).at(s).at(p), 1) + " (Hz)", 0, 0);
                    }
                    else
                    {
                        ofDrawBitmapString(ofToString(dataFreqs.at(w).at(s).at(p), 1) + " (Hz)", 0, 0);
                    }
					ofPopStyle();
					ofPopMatrix();
				}
			}
		}
	}
	ofPopMatrix();


}
