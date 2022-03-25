/**************************************************************************/
/*!
	@file     ofApp.cpp
	@author   Nitin Nair (EmotiBit)

	@mainpage Firmware Installer for EmotiBit.

	@section intro_sec Introduction

	This is an application designed to update the WINC firmware for the Adafruit feather M0 WiFi and program it with EmotiBit FW.
	The Firmware can be found at https://github.com/EmotiBit/EmotiBit_FeatherWing

		EmotiBit invests time and resources providing this open source code,
	please support EmotiBit and open-source hardware by purchasing
	products from EmotiBit!
 

	@section author Author

	Written by Nitin Nair for EmotiBit.

	@section license License

	BSD license, all text here must be included in any redistribution
*/
/**************************************************************************/

/*
Additional Notes:
1. The FirmwareUpdater.ino.feather_m0.bin was created using Arduino by modifying the arduino Example as directed here: https://learn.adafruit.com/adafruit-atwinc1500-wifi-module-breakout/updating-firmware
2. The WINC FW m2m_aio_3a0.bin was obtained by following the guide here: http://ww1.microchip.com/downloads/en/DeviceDoc/ATWINC15x0%20Software%20Release%20Notes_9%20Aug%202018.pdf
  1. The firmware binary was extracted from the ASF package using atme studio 7.
3. The EmotiBit FW EmotiBit_stock_firmware.ino.feather_m0.bin was generated using arduino IDE by compiling the example here: https://github.com/EmotiBit/EmotiBit_FeatherWing/tree/master/EmotiBit_stock_firmware
4. Details about BOSSA can be found here: http://manpages.ubuntu.com/manpages/bionic/man1/bossac.1.html
  1. The executables were obtained from the release page
5. The WINC uploader can be found here: https://github.com/arduino/FirmwareUploader/releases
*/

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
	titleImage.resize(300, 266); // width, height
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
	if (progressFont.load(ofToDataPath("verdanab.ttf"), 18, true, true))
	{
		ofLogNotice() << "Instruction Font loaded correctly";
	}
	if(titleFont.load(ofToDataPath("verdanab.ttf"), 40, true, true))
    {
        ofLogNotice() << "Title Font loaded correctly";
    }
}

//--------------------------------------------------------------
void ofApp::update(){
	// Used for updating "progress string" in the GUI
	static uint32_t timeSinceLastProgressIndicatorUpdate = ofGetElapsedTimeMillis();

	if (_state == State::START)
	{
		progressToNextState();
	}
	else if (_state == State::DISPLAY_INSTRUCTION)
	{
		// wait for sapce-bar key press
		// progress to next state by pressing space-bar

	}
	else if (_state == State::WAIT_FOR_FEATHER)
	{
		// If no feather detected within timeout, raise Error
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
				// Feather Detected!
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

		if (!systemCommandExecuted && pingProgTryCount == MAX_NUM_TRIES_PING_1200)
		{
			// If BOSSA was unsuccesfull and we tried max times
			raiseError();
		}

		// if bossac has been executed and it has been 90 secs(way longer than any expected delay)
		if (systemCommandExecuted && ofGetElapsedTimeMillis() - stateStartTime > 90000)
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

        if (!systemCommandExecuted && pingProgTryCount == MAX_NUM_TRIES_PING_1200)
		{
			// If BOSSA was unsuccesfull and we tried max times
			raiseError();
		}

		// if bossac has been executed and it has been 90 secs(way longer than any expected delay)
		if (systemCommandExecuted && ofGetElapsedTimeMillis() - stateStartTime > 90000)
		{
			raiseError();
		}
	}
	else if (_state == State::COMPLETED)
	{
		// Update message on the GUI
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
		// do nothing. The GUI Messages have already been updated in raiseError()
	}

	// update progressIndicatorString
	if (ofGetElapsedTimeMillis() - timeSinceLastProgressIndicatorUpdate > 500)
	{
		if (_state > State::WAIT_FOR_FEATHER && _state <= State::COMPLETED)
		{
			// stop the progress string from going out of bounds
			if (progressString.size() > 40)
			{
				progressString = "UPDATING";
			}
			else
			{
				progressString += ".";
			}
		}
		timeSinceLastProgressIndicatorUpdate = ofGetElapsedTimeMillis();
	}
}

void ofApp::raiseError(std::string additionalMessage)
{
	progressString = "";
	onScreenInstruction = onScreenInstructionList[State::INSTALLER_ERROR];
	onScreenInstructionImage.clear();
	// set the Error string according to the current state
	displayedErrorMessage = additionalMessage + errorMessageList[_state];
	std::vector<std::string> disaplyedErrorImageList = errorImages[_state];
	for (int i = 0; i < disaplyedErrorImageList.size(); i++)
	{
		ofImage temp;
		temp.load(ofToDataPath(ofFilePath::join("instructions", disaplyedErrorImageList.at(i))));
		temp.resize(resizedImgDim, resizedImgDim);
		disaplyedErrorImage.push_back(temp);
	}
	_state = State::INSTALLER_ERROR;
	pingProgTryCount = 0;
}

void ofApp::progressToNextState()
{
	stateStartTime = ofGetElapsedTimeMillis();
	// ToDo: verify behavior if states dont have a continuous emnumeration
	_state = State((int)_state + 1);
	ofLog(OF_LOG_NOTICE, "State: " + ofToString(_state));
	onScreenInstruction = onScreenInstruction + "\n" + onScreenInstructionList[_state];
	onScreenInstructionImage.clear();
	std::vector<std::string> onScreenInstructionImageList = instructionImages[_state];
	for (int i = 0; i < onScreenInstructionImageList.size(); i++)
	{
		ofImage temp;
		temp.load(ofToDataPath(ofFilePath::join("instructions", onScreenInstructionImageList.at(i))));
		temp.resize(resizedImgDim, resizedImgDim);
		onScreenInstructionImage.push_back(temp);
	}
	if (_state > State::WAIT_FOR_FEATHER && _state < State::COMPLETED)
	{
		progressString = "UPDATING";
	}
	else
	{
		progressString = "";
	}
	pingProgTryCount = 0;
}

//--------------------------------------------------------------
void ofApp::draw(){
	// draw Title
	ofSetColor(0);
	titleFont.drawString("EmotiBit Firmware Installer", guiElementPositions["TitleString"].x, guiElementPositions["TitleString"].y);
	// draw title image
	ofSetColor(255);
	titleImage.draw(guiElementPositions["TitleImage"].x, guiElementPositions["TitleImage"].y);
	
	// set color of instruction
	if (_state == State::DONE)
	{
		// Make text green if Installer was successful
		ofSetColor(37, 190, 80);
	}
	else if (_state == State::INSTALLER_ERROR)
	{
		// Make text red if installer Failed
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

	// draw instruction image
	ofSetColor(255);
	if (onScreenInstructionImage.size() > 0)
	{
		for (int i = 0, offset_x = 0; i < onScreenInstructionImage.size(); i++)
		{
			onScreenInstructionImage.at(i).draw(guiElementPositions["InstructionImage"].x + offset_x, guiElementPositions["InstructionImage"].y);
			offset_x = offset_x + 20 + resizedImgDim;
		}
	}
	// draw error image
	if (disaplyedErrorImage.size() > 0)
	{
		for (int i = 0, offset_x = 0; i < disaplyedErrorImage.size(); i++)
		{
			disaplyedErrorImage.at(i).draw(guiElementPositions["ErrorImage"].x + offset_x, guiElementPositions["ErrorImage"].y);
			offset_x = offset_x + 20 + resizedImgDim;
		}
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	// if pressed spacebar
	if (key == ' ')
	{
		if (_state == State::DISPLAY_INSTRUCTION)
		{
			// start timer to detect feather
			ofResetElapsedTimeCounter();
			progressToNextState();
		}
	}
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
	// The Gui element locations were chosen based on subjective aesthetics.
	guiElementPositions["TitleString"] = GuiElementPos{ 10, 150 + int(titleFont.getLineHeight() / 2) };
	guiElementPositions["TitleImage"] = GuiElementPos{ 724, 30 };
	guiElementPositions["Instructions"] = GuiElementPos{ 30, 300 };
	guiElementPositions["Progress"] = GuiElementPos{ 30, 290 };
	//guiElementPositions["InstructionImage"] = GuiElementPos{ 724, 316 };
	guiElementPositions["InstructionImage"] = GuiElementPos{ 30, 460 };
	guiElementPositions["ErrorImage"] = GuiElementPos{ 30, 460 };
}

void ofApp::setupInstructionList()
{
	// Step based user instructions
	onScreenInstructionList[State::START] = "";
	onScreenInstructionList[State::DISPLAY_INSTRUCTION] = "1. Make sure EmotiBit is stacked with Feather with Battery and SD-Card inserted"
													   "\n2. Make sure the EmotiBit Hibernate switch is NOT set to HIB"
													    "\n\t More information about stacking EmotiBit available at docs.emotibit.com"
														"\n3. Plug in the Feather using using a data-capable USB cable (as provided in the EmotiBit Kit)"
														"\n4. Press space-bar to continue";
	
	onScreenInstructionList[State::WAIT_FOR_FEATHER] = "5. Press Reset button on the Feather (as shown below)";
	onScreenInstructionList[State::UPLOAD_WINC_FW_UPDATER_SKETCH] = "\nDO NOT UNPLUG OR RESET EMOTIBIT\n"
																	"\n>>> Uploading WINC firmware updater sketch";
	onScreenInstructionList[State::RUN_WINC_UPDATER] = ">>> Updating WINC FW";
	onScreenInstructionList[State::UPLOAD_EMOTIBIT_FW] = ">>> Updating EmotiBit firmware";
	onScreenInstructionList[State::COMPLETED] = "\nFIRMWARE UPDATE COMPLETED SUCCESSFULLY!";
	onScreenInstructionList[State::INSTALLER_ERROR] = "FAILED";

	// Images to be displayed for each instruction
	// If you want to add any image to be displayed, just add the image name to the list
	//ToDo: There is currently no bounds on images goinging outside the window, if too many images have been added to the list
	instructionImages[State::START];
	instructionImages[State::DISPLAY_INSTRUCTION] = std::vector<std::string>{ "plugInEmotiBit.jpg" };
	instructionImages[State::WAIT_FOR_FEATHER] = std::vector<std::string>{ "pressResetButton.jpg" };
	instructionImages[State::UPLOAD_WINC_FW_UPDATER_SKETCH];
	instructionImages[State::RUN_WINC_UPDATER];
	instructionImages[State::UPLOAD_EMOTIBIT_FW];
	instructionImages[State::COMPLETED];
	instructionImages[State::INSTALLER_ERROR];
}

void ofApp::setupErrorMessageList()
{
	// Step based error list
	errorMessageList[State::START] = "";
	errorMessageList[State::WAIT_FOR_FEATHER] = "Feather not detected\nThings to check:"
                                                "\n1. Make sure the  Feather is connected to your computer using a data-capable USB cable"
                                                "\n2. Make sure the EmotiBit Hibernate switch is not set to HIB";
	errorMessageList[State::UPLOAD_WINC_FW_UPDATER_SKETCH] = "Failed to Upload WINC Updater Sketch\nRe-run EmotiBit Installer";
	errorMessageList[State::RUN_WINC_UPDATER] = "WINC updater executable failed to run";
	errorMessageList[State::UPLOAD_EMOTIBIT_FW] = "EmotiBit FW update failed\nRe-run EmotiBit Installer";
	
	// Error Image
	// If you want to add any image to be displayed, just add the image name to the list
	// ToDo: There is currently no bounds on images goinging outside the window, if too many images have been added to the list
	errorImages[State::WAIT_FOR_FEATHER] = std::vector < std::string>{"correctHibernateSwitch.jpg"};
	errorImages[State::UPLOAD_WINC_FW_UPDATER_SKETCH] = std::vector < std::string>{ "pressResetButton.jpg" };
	errorImages[State::RUN_WINC_UPDATER] = std::vector < std::string>{ "pressResetButton.jpg" };
	errorImages[State::UPLOAD_EMOTIBIT_FW] = std::vector < std::string>{ "pressResetButton.jpg" };
}

int ofApp::detectFeatherPlugin()
{
	if (!captureComListOnStartup)
	{
		// get initial list of available com ports
		comListOnStartup = getComPortList();
		captureComListOnStartup = true;
	}
	std::vector<std::string> currentComList = getComPortList(true);
	if (currentComList.size() < comListOnStartup.size())
	{
		// On reset, the COM list reduces and grows back
		// Update the startup COM list if reset was detected.
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
			// found 1 new COM port. The new COM port is taken as the Feather Port
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
	// increment try count
	pingProgTryCount++;
	if (pingProgTryCount < MAX_NUM_TRIES_PING_1200)
	{
		ofLog(OF_LOG_NOTICE, "Ping try: " + ofToString(pingProgTryCount));
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
		// Connect and disconnect at 1200 to try and set the Feather in bootloader mode
		serial.setup(featherPort, 1200);
		ofSleepMillis(200);
		serial.close();
		ofSleepMillis(1000);

		updatedComPortList = getComPortList(true);
		// if feather was put in programmer mode, BUT we grabbed available COM port list before it showed up as a COM port
		while (updatedComPortList.size() < initialComPortList.size())
		{
			updatedComPortList = getComPortList(true);
			ofSleepMillis(500);
		}
		// check if a new COM port has been detected. Programmer mode appears as new COM port on windows
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
		// return the result of execution
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
		// check for system response
		bool status = checkSystemCallResponse();
		// if command exe is complete but bossac failed
		if (!status && !systemCommandExecuted)
		{
			// try for MAX_NUM times
			bossacTryCount++;
			if (bossacTryCount < MAX_NUM_TRIES_BOSSAC)
			{
				// reset trying to enter prog mode
				pingProgTryCount = 0;
			}
		}
		return status;
	}
}

bool ofApp::uploadWincUpdaterSketch()
{
	// get the path to the wifi updater sketch binary
	std::string filepath = ofFilePath::join("WINC", "FirmwareUpdater.ino.feather_m0.bin");
    return updateUsingBossa(ofToDataPath(filepath));
}

bool ofApp::runWincUpdater()
{
	if (!systemCommandExecuted)
	{
#ifdef TARGET_WIN32
		// get updated COM list. The feather returns back to feather port, after the bossa flash is completed.
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
