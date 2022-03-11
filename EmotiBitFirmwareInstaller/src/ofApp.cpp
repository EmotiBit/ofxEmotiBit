#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofLogToConsole();
	ofSetLogLevel(OF_LOG_NOTICE);
	_state = State::WAIT_FOR_FEATHER;
	setupGuiElementPositions();
	setupErrorMessageList();
	setupInstructionList();
	titleImage.load("EmotiBit.png");
	titleImage.resize(300, 266);
	// get initial list of available com ports
	comListOnStartup = getComPortList(true);
	ofBackground(255, 255, 255, 255);

	//old OF default is 96 - but this results in fonts looking larger than in other programs.
	ofTrueTypeFont::setGlobalDpi(72);

	if(instructionFont.load("verdana.ttf", 20, true, true))
    {
        ofLogNotice() << "Instruction Font loaded correctly";
    }
	if (progressFont.load("verdanab.ttf", 20, true, true))
	{
		ofLogNotice() << "Instruction Font loaded correctly";
	}
	if(titleFont.load("verdanab.ttf", 40, true, true))
    {
        ofLogNotice() << "Title Font loaded correctly";
    }
	//instructionFont.setLineHeight(18.0f);
	//instructionFont.setLetterSpacing(1.037);
}

//--------------------------------------------------------------
void ofApp::update(){
	bool progressToNextState = false;
	static uint32_t timeSinceLastProgressUpdate = ofGetElapsedTimeMillis();
	if (!globalTimerReset)
	{
		resetStateTimer();
		ofLog(OF_LOG_NOTICE, "State: " + ofToString(_state));
		currentInstruction = currentInstruction + "\n" + onScreenInstructionList[_state];
	}

	if (_state == State::WAIT_FOR_FEATHER)
	{
		if (detectFeatherPlugin())
		{
			// progress to next state;
			progressToNextState = true;
		}
	}
	else if (_state == State::UPLOAD_WINC_FW_UPDATER_SKETCH)
	{
		if (uploadWincUpdaterSketch())
		{
			// progress to next state;
			progressToNextState = true;
		}
	}
	else if (_state == State::RUN_WINC_UPDATER)
	{
		if (runWincUpdater())
		{
			// progress to next state;
			progressToNextState = true;
		}
	}
	else if (_state == State::UPLOAD_EMOTIBIT_FW)
	{
		if (uploadEmotiBitFw())
		{
			// progress to next state;
			progressToNextState = true;
		}
	}
	else if (_state == State::TIMEOUT)
	{
		// print error on the console
		ofLog(OF_LOG_ERROR, errorMessage);
		// print the error message on GUI
		resetStateTimer();
		while (ofGetElapsedTimef() < 5);
		ofExit();
	}
	else if (_state == State::COMPLETED)
	{
		// print some success message
		ofLog(OF_LOG_NOTICE, onScreenInstructionList[State::COMPLETED]);
		resetStateTimer();
		progressToNextState = true;
	}
	else if (_state == State::EXIT)
	{
		while (ofGetElapsedTimef() < 5);
		ofExit();
	}

	// raise timeout if no system command is running and state has not progressed in STATE_TIMEOUT
	if (!progressToNextState  && !systemCommandExecuted && ofGetElapsedTimef() > STATE_TIMEOUT)
	{
		// timeout
		currentInstruction = "[FAILED]: " + onScreenInstructionList[_state];
		progressString = "";
		errorMessage = errorMessageList[_state];
		_state = State::TIMEOUT;
		globalTimerReset = false;
	}
	if (progressToNextState)
	{
		_state = State((int)_state + 1);
		if (_state <= State::COMPLETED)
		{
			progressString = "UPDATING";
		}
		else
		{
			progressString = "";
		}
		globalTimerReset = false;
	}
	if (ofGetElapsedTimeMillis() - timeSinceLastProgressUpdate > 500)
	{
		if (_state > State::WAIT_FOR_FEATHER && _state <= State::COMPLETED)
		{
			progressString += ".";
		}
		timeSinceLastProgressUpdate = ofGetElapsedTimeMillis();
	}
}

//--------------------------------------------------------------
void ofApp::draw(){

	ofSetColor(0);
	titleFont.drawString("EmotiBit Firmware Installer", guiElementPositions["TitleString"].x, guiElementPositions["TitleString"].y);
	ofSetColor(255);
	titleImage.draw(guiElementPositions["TitleImage"].x, guiElementPositions["TitleImage"].y);
	
	// color of instructions
	ofSetColor(0);
	instructionFont.drawString(currentInstruction + "\n" + errorMessage, guiElementPositions["Instructions"].x, guiElementPositions["Instructions"].y);
	
	// color of progress string
	ofSetColor(0, 255, 150);
	progressFont.drawString(progressString, guiElementPositions["Progress"].x, guiElementPositions["Progress"].y);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}


void ofApp::setupGuiElementPositions()
{
	guiElementPositions["TitleString"] = GuiElementPos{ 10, 150 + int(titleFont.getLineHeight() / 2) };
	guiElementPositions["TitleImage"] = GuiElementPos{ 724, 50 };
	guiElementPositions["Instructions"] = GuiElementPos{ 30, 410 };
	guiElementPositions["Progress"] = GuiElementPos{ 30, 400 }; 
}

void ofApp::setupInstructionList()
{
	onScreenInstructionList[State::WAIT_FOR_FEATHER] = "Plug in the feather using the provided USB cable";
	onScreenInstructionList[State::UPLOAD_WINC_FW_UPDATER_SKETCH] = "Step1: Uploading WINC Firmware updater Sketch";
	onScreenInstructionList[State::RUN_WINC_UPDATER] = "Step2: Updating WINC FW";
	onScreenInstructionList[State::UPLOAD_EMOTIBIT_FW] = "Step3: Updating EmotiBit firmware";
	onScreenInstructionList[State::COMPLETED] = "FIRMWARE UPDATE COMPLETED SUCCESSFULLY!";
	onScreenInstructionList[State::TIMEOUT] = "";
}

void ofApp::setupErrorMessageList()
{
	errorMessageList[State::WAIT_FOR_FEATHER] = "Feather not detected. Check USB cable. \nMake sure the Feather was not connected before installer was started!";
	errorMessageList[State::UPLOAD_WINC_FW_UPDATER_SKETCH] = "Could not set feather into Bootloader mode. WINC FW UPDATER sketch upload failed.";
	errorMessageList[State::RUN_WINC_UPDATER] = "WINC UPDATER executable failed to run.";
	errorMessageList[State::UPLOAD_EMOTIBIT_FW] = "Could not set feather into Bootloader mode. EmotiBit stock FW update failed.";
}

void ofApp::resetStateTimer()
{
	ofResetElapsedTimeCounter();
	globalTimerReset = true;
}

bool ofApp::detectFeatherPlugin()
{
	std::vector<std::string> currentComList = getComPortList(true);
	std::string newComPort = findNewComPort(comListOnStartup, currentComList);
	if (newComPort.compare(COM_PORT_NONE) != 0)
	{
		// found new COM port
		featherPort = newComPort;
		return true;
	}
	else
	{
		ofLog(OF_LOG_NOTICE, "No new COM port detected");
		return false;
	}
}

std::vector<std::string> ofApp::getComPortList(bool printOnConsole)
{
	ofSerial serial;
	std::vector<std::string> comPortList;
	// get list of com ports
	vector <ofSerialDeviceInfo> initDeviceList = serial.getDeviceList();
	// convert device list into COM ports
	for (int i = 0; i < initDeviceList.size(); i++)
	{
		comPortList.push_back(initDeviceList.at(i).getDevicePath());
	}
	// sort the list
	std::sort(comPortList.begin(), comPortList.end());
	
	// print available COM ports on console
	if (printOnConsole)
	{
		std::string comPorts;
		for (int i = 0; i < comPortList.size() - 1; i++)
		{
			comPorts += comPortList.at(i) + DELIMITER;
		}
		comPorts += comPortList.back();
		ofLog(OF_LOG_NOTICE, "Available COM ports: " + comPorts);
	}
	return comPortList;
}

bool ofApp::initProgrammerMode(std::string &programmerPort)
{
#if defined(TARGET_LINUX) || defined(TARGET_OSX)
    // set to bootloader mode
    // sonnect to serial port
    ofLog(OF_LOG_NOTICE, "connecting using screen");
    std::string command = "screen -d -m " + featherPort + " 1200";
    system(command.c_str());
    // dicsonnect serial port
    ofSleepMillis(1000);
    ofLog(OF_LOG_NOTICE,"disconneting");
    system("screen -ls | grep Detached | cut -d. -f1 | awk '{print $1}' | xargs kill");
    command.clear();
    return true;
#else
	ofSerial serial;
	// get initial list of COM ports
	std::vector<std::string> initialComPortList = getComPortList(true);
	std::vector<std::string> updatedComPortList;
	ofLog(OF_LOG_NOTICE, "Pinging Port: " + featherPort);
	serial.setup(featherPort, 1200);
	ofSleepMillis(200);
	serial.close();
	ofSleepMillis(1000);
	updatedComPortList = getComPortList(true);
	// check if a new OCM port has been detected
	std::string newPort = findNewComPort(initialComPortList, updatedComPortList);
	if (newPort.compare(COM_PORT_NONE) != 0)
	{
		// return the new COM port and the list of COM ports with the feather in programmer mode
		programmerPort = newPort;
		comListWithProgrammingPort = updatedComPortList;
		return true;
	}
	return false;
#endif
}

std::string ofApp::findNewComPort(std::vector<std::string> oldList, std::vector<std::string> newList)
{
	// for every COM port in the new list
	for (int i = 0; i < newList.size(); i++)
	{
		// check if the COM port existed in the old list
		if (std::find(oldList.begin(), oldList.end(), newList.at(i)) == oldList.end())
		{
			// COM port not in oldList 
			// NEW PORT!
			ofLog(OF_LOG_NOTICE, "New COM port detected: " + newList.at(i));
			return newList.at(i);
		}
		else
		{
			// COM port exists in oldList
			continue;
		}
	}
	return COM_PORT_NONE;
}

bool ofApp::checkSystemCallResponse()
{
	if (threadedSystemCall.isThreadRunning())
	{
		// thread is still running
		threadedSystemCall.lock();
		std::string systemOutput = threadedSystemCall.systemOutput;
		threadedSystemCall.systemOutput = "";
		threadedSystemCall.unlock();
		if (systemOutput != "")
		{
			ofLog(OF_LOG_NOTICE, systemOutput);
		}
		return false;
	}
	else
	{
		// print out any system outputs we did not pick up before thread stopped
		// No need to lock as thread is stopped
		std::string systemOutput = threadedSystemCall.systemOutput;
		ofLog(OF_LOG_NOTICE, systemOutput);
		// thread execution complete
		systemCommandExecuted = false;
		return threadedSystemCall.cmdResult;
	}
}

bool ofApp::updateUsingBossa(std::string filePath)
{
	std::string programmerPort;
	// try to set feather in programmer mode
	if (!systemCommandExecuted)
	{
		if (initProgrammerMode(programmerPort))
		{
			// run command to upload WiFi Updater sketch
			ofLog(OF_LOG_NOTICE, "uploading WiFi updater sketch");
			std::string command = "data\\bossac.exe -i -d -U true -e -w -v -R -b -p " + programmerPort + " " + filePath;
			ofLogNotice("Running: ") << command;
			//system(command.c_str());
			threadedSystemCall.setup(command, "Verify successful"); // the target response string is captured from observed output
			threadedSystemCall.startThread();
			systemCommandExecuted = true;
		}
		return false;
	}
	else
	{
		return checkSystemCallResponse();
	}
}

bool ofApp::uploadWincUpdaterSketch()
{
#if defined(TARGET_LINUX) || defined(TARGET_OSX)
    std::string dontCarePort;
    // set feather in programmer mode
    initProgrammerMode(dontCarePort);
    ofLog(OF_LOG_NOTICE,"waiting to flash with bossa");
    ofSleepMillis(5000);
    ofLog(OF_LOG_NOTICE, "uploading firmware updater sketch");
    // upload the firmware updater sketch
    std::string command;
    //system("pwd");
    //system("ls");
    command = "../MacOS/bossac -i -d -U true -e -w -v -R -b -p " + featherPort + " WINC/FirmwareUpdater.ino.feather_m0.bin";
    ofLogNotice("BOSSA Command") << command;
    system(command.c_str());
    return true;
#else
	return updateUsingBossa("data\\WINC\\FirmwareUpdater.ino.feather_m0.bin");
#endif
}

bool ofApp::runWincUpdater()
{
#if defined(TARGET_LINUX) || defined(TARGET_OSX)
    // run the WINC updater
    ofLog(OF_LOG_NOTICE, "Updating Winc FW using WiFi101 updater");
    ofSleepMillis(3000);
    std::string command = std::string("../MacOS/FirmwareUploader -firmware WINC/m2m_aio_3a0.bin ") + std::string("-port ") + featherPort;
    ofLogNotice("BOSSA Command") << command;
    system(command.c_str());
    return true;
#else
	if (!systemCommandExecuted)
	{
		// get updated COM list. the feather returns back to feather port, after the bossa flash is completed.
		std::vector<std::string> newComPortList = getComPortList(true);
		// find the feather port 
		featherPort = findNewComPort(comListWithProgrammingPort, newComPortList); // old list, new list

		if (featherPort.compare(COM_PORT_NONE) != 0)
		{
			ofLog(OF_LOG_NOTICE, "Feather found at: " + featherPort);
			std::string command = "data\\WINC\\FirmwareUploader.exe -port " + featherPort + " -firmware " + "data\\WINC\\m2m_aio_3a0.bin";
			ofLogNotice("UPDATING WINC FW: COMMAND: ") << command;
			threadedSystemCall.setup(command);
			threadedSystemCall.startThread();
			systemCommandExecuted = true;
		}
	}
	else
	{
		return checkSystemCallResponse();
	}
	return false;
#endif
}

bool ofApp::uploadEmotiBitFw()
{
#if defined(TARGET_LINUX) || defined(TARGET_OSX)
    std::string dontCarePort;
    initProgrammerMode(dontCarePort);
    ofLog(OF_LOG_NOTICE,"waiting to flash with bossa");
    ofSleepMillis(5000);
    // upload the firmware updater sketch
    std::string command = "../MacOS/bossac -i -d -U true -e -w -v -R -b -p " + featherPort + " EmotiBit_stock_firmware.ino.feather_m0.bin";
    ofLogNotice("BOSSA Command") << command;
    system(command.c_str());
    return true;
#else
	return updateUsingBossa("data\\EmotiBit_stock_firmware.ino.feather_m0.bin");
#endif
}
