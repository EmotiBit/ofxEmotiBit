#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofLogToConsole();
	ofSetLogLevel(OF_LOG_NOTICE);
	_state = State::START;
	setupGuiElementPositions();
	setupErrorMessageList();
	setupInstructionList();
	titleImage.load("EmotiBit.png");
	titleImage.resize(300, 266);
	// get initial list of available com ports
	comListOnStartup = getComPortList(true);
	ofBackground(255, 255, 255, 255);
#ifdef TARGET_OSX
    ofSetDataPathRoot("../Resources/");
#endif
	//old OF default is 96 - but this results in fonts looking larger than in other programs.
	ofTrueTypeFont::setGlobalDpi(72);

	if(instructionFont.load(ofToDataPath("verdana.ttf"), 20, true, true))
    {
        ofLogNotice() << "Instruction Font loaded correctly";
    }
	if (progressFont.load(ofToDataPath("verdanab.ttf"), 20, true, true))
	{
		ofLogNotice() << "Instruction Font loaded correctly";
	}
	if(titleFont.load(ofToDataPath("verdanab.ttf"), 40, true, true))
    {
        ofLogNotice() << "Title Font loaded correctly";
    }
	//instructionFont.setLineHeight(18.0f);
	//instructionFont.setLetterSpacing(1.037);
}

//--------------------------------------------------------------
void ofApp::update(){
	// Used for updating "progress string" in the GUI
	static uint32_t timeSinceLastProgressIndicatorUpdate = ofGetElapsedTimeMillis();

	if (_state == State::START)
	{
		// starting state machine
		progressToNextState();
	}
	else if (_state == State::WAIT_FOR_FEATHER)
	{
		if (ofGetElapsedTimef() > STATE_TIMEOUT)
		{
			// Feather not detected before TIMEOUT
			raiseError();
		}
		else
		{
			int numDevicesDetected = detectFeatherPlugin();
			if (numDevicesDetected == 1)
			{
				// progress to next state;
				progressToNextState();
			}
			else if (numDevicesDetected > 1)
			{
				// multiple devices detected
				raiseError("Multiple Devices Detected");
			}
		}
	}
	else if (_state == State::UPLOAD_WINC_FW_UPDATER_SKETCH)
	{

        // check if the upload was completed successfully
        if (uploadWincUpdaterSketch())
        {
            // progress to next state;
            progressToNextState();
        }
		if (!systemCommandExecuted && tryCount == MAX_NUM_TRIES_PING_1200)
		{
			raiseError();
		}
	}
	else if (_state == State::RUN_WINC_UPDATER)
	{
		// ToDo: How to catch a fail for the Winc Updater? 
        if (runWincUpdater())
        {
            // progress to next state;
            progressToNextState();
        }
	}
	else if (_state == State::UPLOAD_EMOTIBIT_FW)
	{
        if (uploadEmotiBitFw())
        {
            // progress to next state;
            progressToNextState();
        }

        if (!systemCommandExecuted && tryCount == MAX_NUM_TRIES_PING_1200)
		{
			raiseError();
		}
	}
	else if (_state == State::COMPLETED)
	{
		// print some success message
		ofLog(OF_LOG_NOTICE, onScreenInstructionList[State::COMPLETED]);
		//resetStateTimer();
		progressToNextState();
	}
	else if (_state == State::DONE)
	{
		// do nothing
	}
	else if (_state == State::INSTALLER_ERROR)
	{
		// do nothing
	}

	// update progressIndicatorString
	if (ofGetElapsedTimeMillis() - timeSinceLastProgressIndicatorUpdate > 500)
	{
		if (_state > State::WAIT_FOR_FEATHER && _state <= State::COMPLETED)
		{
			progressString += ".";
		}
		timeSinceLastProgressIndicatorUpdate = ofGetElapsedTimeMillis();
	}
}

void ofApp::raiseError(std::string additionalMessage)
{
	progressString = "";
	onScreenInstruction = onScreenInstructionList[State::INSTALLER_ERROR];
	// set the Error string according to the current state
	displayedErrorMessage = additionalMessage + errorMessageList[_state];
	_state = State::INSTALLER_ERROR;
	tryCount = 0;
}

void ofApp::progressToNextState()
{
	// ToDo: verify behavior if states dont have a continuous emnumeration
	_state = State((int)_state + 1);
	ofLog(OF_LOG_NOTICE, "State: " + ofToString(_state));
	onScreenInstruction = onScreenInstruction + "\n" + onScreenInstructionList[_state];
	if (_state > State::WAIT_FOR_FEATHER && _state < State::COMPLETED)
	{
		progressString = "UPDATING";
	}
	else
	{
		progressString = "";
	}
	tryCount = 0;
}

//--------------------------------------------------------------
void ofApp::draw(){

	ofSetColor(0);
	titleFont.drawString("EmotiBit Firmware Installer", guiElementPositions["TitleString"].x, guiElementPositions["TitleString"].y);
	ofSetColor(255);
	titleImage.draw(guiElementPositions["TitleImage"].x, guiElementPositions["TitleImage"].y);
	
	// color of instructions
	if (_state == State::DONE)
	{
		ofSetColor(37, 190, 80);
	}
	else if (_state == State::INSTALLER_ERROR)
	{
		ofSetColor(234, 42, 11);
	}
	else
	{
		ofSetColor(0);
	}
	instructionFont.drawString(onScreenInstruction + "\n" + displayedErrorMessage, guiElementPositions["Instructions"].x, guiElementPositions["Instructions"].y);
	
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
	onScreenInstructionList[State::WAIT_FOR_FEATHER] = "Plug in the feather using the provided USB cable. If already plugged in, press Reset";
	onScreenInstructionList[State::UPLOAD_WINC_FW_UPDATER_SKETCH] = "Step1: Uploading WINC Firmware updater Sketch";
	onScreenInstructionList[State::RUN_WINC_UPDATER] = "Step2: Updating WINC FW";
	onScreenInstructionList[State::UPLOAD_EMOTIBIT_FW] = "Step3: Updating EmotiBit firmware";
	onScreenInstructionList[State::COMPLETED] = "FIRMWARE UPDATE COMPLETED SUCCESSFULLY!";
	onScreenInstructionList[State::INSTALLER_ERROR] = "FAILED";
}

void ofApp::setupErrorMessageList()
{
	errorMessageList[State::START] = "";
	errorMessageList[State::WAIT_FOR_FEATHER] = "Feather not detected. Things to check: \n1. Check USB cable.\n2. Make sure EmotiBit Hibernate switch is not on HIB";
	errorMessageList[State::UPLOAD_WINC_FW_UPDATER_SKETCH] = "Failed to Upload WINC Updater Sketch.";
	errorMessageList[State::RUN_WINC_UPDATER] = "WINC updater executable failed to run.";
	errorMessageList[State::UPLOAD_EMOTIBIT_FW] = "EmotiBit stock FW update failed.";
}

int ofApp::detectFeatherPlugin()
{
	std::vector<std::string> currentComList = getComPortList(true);
	if (currentComList.size() < comListOnStartup.size())
	{
		comListOnStartup = currentComList;
		ofLogNotice("COM LIST SIZE REDUCED") << "Reset pressed or feather unplugged";
	}
	else if (currentComList.size() > comListOnStartup.size())
	{
		if (currentComList.size() - comListOnStartup.size() > 1)
		{
			// found multiple new com ports
			ofLog(OF_LOG_NOTICE, "More than one Port ");
		}
		else
		{
			// found 1 new COM port
			std::string newComPort = findNewComPort(comListOnStartup, currentComList);

			if (newComPort.compare(COM_PORT_NONE) != 0)
			{
				featherPort = newComPort;
				return currentComList.size() - comListOnStartup.size();
			}
		}
	}
	return currentComList.size() - comListOnStartup.size();
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
#ifdef TARGET_OSX
        if(initDeviceList.at(i).getDevicePath().find("/dev/tty") != std::string::npos)
        {
            comPortList.push_back(initDeviceList.at(i).getDevicePath());
        }
#else
        comPortList.push_back(initDeviceList.at(i).getDevicePath());
#endif
	}
	// sort the list
	std::sort(comPortList.begin(), comPortList.end());
	
	// print available COM ports on console
	if (printOnConsole)
	{
		std::string comPorts;
		for (int i = 0; i < comPortList.size(); i++)
		{
			comPorts += comPortList.at(i) + DELIMITER;
		}
		ofLog(OF_LOG_NOTICE, "Available COM ports: " + comPorts);
	}
	return comPortList;
}

bool ofApp::initProgrammerMode(std::string &programmerPort)
{
	tryCount++;
	if (tryCount < MAX_NUM_TRIES_PING_1200)
	{
		ofLog(OF_LOG_NOTICE, "Ping try: " + ofToString(tryCount));
#if defined(TARGET_LINUX) || defined(TARGET_OSX)
		// set to bootloader mode
		// connect to serial port
		ofLog(OF_LOG_NOTICE, "connecting using screen");
		std::string command = "screen -d -m -S featherM0 " + featherPort + " 1200";
		system(command.c_str());
		// dicsonnect serial port
		ofSleepMillis(1000);
		ofLog(OF_LOG_NOTICE, "disconneting");
        command.clear();
        command = "screen -XS featherM0 quit";
        ofSleepMillis(500);
		//system("screen -ls | grep Detached | cut -d. -f1 | awk '{print $1}' | xargs kill");
        system(command.c_str());
        
		//programmerPort = featherPort;
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
		// check if a new COM port has been detected
		std::string newPort = findNewComPort(initialComPortList, updatedComPortList);
		if (newPort.compare(COM_PORT_NONE) != 0)
		{
			// return the new COM port and the list of COM ports with the feather in programmer mode
			programmerPort = newPort;
			comListWithProgrammingPort = updatedComPortList;
			return true;
		}
#endif
	}
	else
	{
		// Did not enter programmer mode after MAX_TRIES.
		// return feather port
		ofLog(OF_LOG_NOTICE, "Unable to enter programmer mode. returning feather port.");
		programmerPort = featherPort;
		comListWithProgrammingPort = getComPortList();
		return true;
	}
	return false;
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
	// Set error state. It is set to None when Bossac is completed successfully
	std::string programmerPort;
	// try to set feather in programmer mode
	if (!systemCommandExecuted)
	{
		if (initProgrammerMode(programmerPort))
		{
			// run command to upload WiFi Updater sketch
			ofLog(OF_LOG_NOTICE, "uploading WiFi updater sketch");
            std::string command;
#if defined(TARGET_LINUX) || defined(TARGET_OSX)
            ofLog(OF_LOG_NOTICE,"waiting to flash with bossa");
            ofSleepMillis(5000);
			command = "bossac";
#else
			command = "bossac.exe";
#endif
			command = ofToDataPath(command);
            command = command + " " + "-i -d -U true -e -w -v -R -b -p " + programmerPort + " " + filePath;
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
	std::string filepath = ofFilePath::join("WINC", "FirmwareUpdater.ino.feather_m0.bin");
    return updateUsingBossa(ofToDataPath(filepath));
}

bool ofApp::runWincUpdater()
{
	if (!systemCommandExecuted)
	{
#ifdef TARGET_WIN32
		// get updated COM list. the feather returns back to feather port, after the bossa flash is completed.
		std::vector<std::string> newComPortList = getComPortList(true);
		// find the feather port 
		featherPort = findNewComPort(comListWithProgrammingPort, newComPortList); // old list, new list
#endif
		if (featherPort.compare(COM_PORT_NONE) != 0)
		{
			ofLog(OF_LOG_NOTICE, "Feather found at: " + featherPort);
#ifdef TARGET_WIN32
			std::string applicationName = "FirmwareUploader.exe";
#else
            std::string applicationName = "FirmwareUploader";
#endif
            std::string applicationPath = ofFilePath::join("WINC", applicationName);
            std::string filename = "m2m_aio_3a0.bin";
            std::string filepath = ofFilePath::join("WINC", filename);
            std::string command = ofToDataPath(applicationPath) + " -port "  + featherPort + " -firmware " + ofToDataPath(filepath);
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
}

bool ofApp::uploadEmotiBitFw()
{
    return updateUsingBossa(ofToDataPath("EmotiBit_stock_firmware.ino.feather_m0.bin"));
}
