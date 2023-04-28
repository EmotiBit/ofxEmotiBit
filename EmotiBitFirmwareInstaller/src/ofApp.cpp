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

RE Feather M0:
1. The FirmwareUpdater.ino.feather_m0.bin was created using Arduino by modifying the arduino Example as directed here: https://learn.adafruit.com/adafruit-atwinc1500-wifi-module-breakout/updating-firmware
2. The WINC FW m2m_aio_3a0.bin was obtained by following the guide here: http://ww1.microchip.com/downloads/en/DeviceDoc/ATWINC15x0%20Software%20Release%20Notes_9%20Aug%202018.pdf
  1. The firmware binary was extracted from the ASF package using atme studio 7.
3. The EmotiBit FW EmotiBit_stock_firmware.ino.feather_m0.bin was generated using arduino IDE by compiling the example here: https://github.com/EmotiBit/EmotiBit_FeatherWing/tree/master/EmotiBit_stock_firmware
4. Details about BOSSA can be found here: http://manpages.ubuntu.com/manpages/bionic/man1/bossac.1.html
  1. The executables were obtained from the release page
5. The WINC uploader can be found here: https://github.com/arduino/FirmwareUploader/releases

RE Feather ESP32:
1. The Feather ESP32 requires esptool executable/binary to flash firmware. We are using esptool v3.3, which can found on espressif's release page: https://github.com/espressif/esptool/releases/tag/v3.3
2. esptool requries additional bin files to run, "boot_app0.bin", "EmotiBit_stock_firmware.ino.bootloader.bin" and "EmotiBit_stock_firmware.partitions.bin" were all acquired from visual micro's build+upload steps
*/

#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
#ifdef TARGET_OSX
	ofSetDataPathRoot("../Resources/");
#endif
	ofSetWindowTitle("EmotiBit Firmware Installer (v" + ofxEmotiBitVersion + ")");
	ofLogToConsole();
#ifdef TARGET_LINUX
	ofLogNotice() << "To install firmware on linux, please follow instructions at https://github.com/EmotiBit/EmotiBit_Docs/blob/master/Getting_Started.md#for-linux-and-advanced-users";
	ofSleepMillis(5000);
	ofExit(0);
	return;
#endif
	ofSetLogLevel(OF_LOG_NOTICE);
	//old OF default is 96 - but this results in fonts looking larger than in other programs.
	ofTrueTypeFont::setGlobalDpi(72);
	// ToDo: Handle the case if a font file is missing
	if (instructionFont.load(ofToDataPath("verdana.ttf"), 20, true, true))
	{
		ofLogNotice() << "Instruction Font loaded correctly";
	}
	if (titleFont.load(ofToDataPath("verdanab.ttf"), 40, true, true))
	{
		ofLogNotice() << "Title Font loaded correctly";
	}
	if (footnoteFont.load(ofToDataPath("verdana.ttf"), 13, true, true))
	{
		ofLogNotice() << "Footnote Font loaded correctly";
	}
	if (warningFont.load(ofToDataPath("verdanab.ttf"), 21, true, true))
	{
		ofLogNotice() << "Warning Font loaded correctly";
	}

	_state = State::START;
	setupErrorMessageList();
	setupInstructionList();
	setupGuiElementPositions();
	titleImage.load("EmotiBit.png");
	titleImage.resize(300, 266); // width, height
	ofBackground(255, 255, 255, 255);

	boardComList[Board::FEATHER_M0];
	boardComList[Board::FEATHER_ESP_32];
}

//--------------------------------------------------------------
void ofApp::update(){
	// Used for updating "progress string" in the GUI
	static uint32_t timeSinceLastProgressIndicatorUpdate = ofGetElapsedTimeMillis();

	if (guiTestMode)
	{
		if ((int)_state < guiTestState)
		{
			if (guiTestState == (int)State::INSTALLER_ERROR)
			{
				raiseError();
			}
			else
			{
				progressToNextState();
			}
		}
		else
		{
			// we are at the state we want to test. Check GUI output.
		}
	}
	else
	{
		if (_state == State::START)
		{
			clearGuiElements();
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
					if (getBoard() == Board::FEATHER_M0)
					{
						// progress to next state;
						progressToNextState();
					}
					else if (getBoard() == Board::FEATHER_ESP_32)
					{
						// progress to flashing FW;
						progressToNextState((int)State::UPLOAD_EMOTIBIT_FW);
					}
				}
				else if (numDevicesDetected > 1)
				{
					// multiple devices detected
					raiseError("Multiple Devices Detected\n");
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
			ofLog(OF_LOG_NOTICE, instructionList[State::COMPLETED]);
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
	}
	// update progressIndicatorString
	if (ofGetElapsedTimeMillis() - timeSinceLastProgressIndicatorUpdate > 500)
	{
		if (_state >= State::WAIT_FOR_FEATHER && _state < State::COMPLETED)
		{
			// stop the progress string from going out of bounds
			if (progressString.size() > 40)
			{
				progressString = "";
			}
			else
			{
				progressString += ".";
			}
		}
		timeSinceLastProgressIndicatorUpdate = ofGetElapsedTimeMillis();
	}

	// Update any changes to the loaded fw file path
	fwPathGuiString = _fwFilePath;
	if (fwPathGuiString.length() > 200)  // we display a max of 200 characters on the screen for file path
	{
		// If path is longer than 200 characters, 
		// keep the last 197 characters of the path (to fit on screen)
		fwPathGuiString = fwPathGuiString.substr(fwPathGuiString.length() - 197);
		fwPathGuiString = "..." + fwPathGuiString;
	}
	// fold long path into multi-line string to fit on screen without overflow
	const int charPerLine = 50;
	std::string foldedFilePath = "";
	int subStrEnd = 0;
	while (subStrEnd <= fwPathGuiString.length())
	{
		foldedFilePath += fwPathGuiString.substr(subStrEnd, charPerLine);
		foldedFilePath += "\n";
		subStrEnd += charPerLine;
	}
	fwPathGuiString = foldedFilePath;
}

void ofApp::raiseError(std::string additionalMessage)
{
	progressString = "";
	// clear existing GUI element lists
	textElementlist.clear();
	imageElementList.clear();
	
	// load text elements to be displayed
	GuiElement::Location startLoc = guiElementPositions["Instructions"];
	textElementlist.push_back(GuiTextElement(startLoc, ofColor(255,0,0),instructionList[State::INSTALLER_ERROR]));
	int cursorXLoc = startLoc.x;
	int cursorYLoc = startLoc.y + instructionFont.stringHeight(instructionList[State::INSTALLER_ERROR]) + 5;
	// set the Error string according to the current state
	std::string errorMessage = additionalMessage + errorMessageList[_state];
	textElementlist.push_back(GuiTextElement(GuiElement::Location(cursorXLoc, cursorYLoc), ofColor(255, 0, 0), errorMessage));
	cursorYLoc += instructionFont.stringHeight(errorMessage);

	// Load image elements to be displayed
	std::vector<std::string> images = errorImages[_state];
	for (int i = 0; i < images.size(); i++)
	{
		ofImage img;
		img.load(ofToDataPath(ofFilePath::join("instructions", images.at(i))));
		img.resize(resizedImgDim, resizedImgDim);
		//disaplyedErrorImage.push_back(img);
		imageElementList.push_back(GuiImageElement(GuiElement::Location(cursorXLoc, cursorYLoc), ofColor(255), img));
		cursorXLoc += (resizedImgDim + 10); // aditional 10 pixel to add distance between images displayed
	}
	_state = State::INSTALLER_ERROR;
	pingProgTryCount = 0;
}

void ofApp::progressToNextState(int state)
{
	stateStartTime = ofGetElapsedTimeMillis();
	// ToDo: verify behavior if states dont have a continuous emnumeration
	if (state == -1)
	{
		_state = State((int)_state + 1);
	}
	else
	{
		_state = (State)state;
	}
	ofLog(OF_LOG_NOTICE, "State: " + ofToString(_state));
	if (_state == State::DONE)
	{
		return;
	}
	
	// clear text and image GUi element lists
	textElementlist.clear();
	imageElementList.clear();

	// move already displayed messages to "old" stack
	if (newMessage != "")
	{
		// there exists a message on the screen
		oldMessage += newMessage;
		oldMessage += "\n";
	}
	newMessage = instructionList[_state];

	// Load text GUI elements
	GuiElement::Location instructionStartLoc = guiElementPositions["Instructions"];
	// Add warning if flashing firmware
	if ((int)_state > (int)State::WAIT_FOR_FEATHER && (int)_state < (int)State::COMPLETED)
	{
		textElementlist.push_back(GuiTextElement(instructionStartLoc, ofColor(255, 128, 0), S_WARNING));
	}
	int cursorYLocation = instructionStartLoc.y;
	cursorYLocation += warningFont.stringHeight(S_WARNING) + 5; // update cursor location
	
	// add old message in gray
	textElementlist.push_back(GuiTextElement(GuiElement::Location(instructionStartLoc.x,cursorYLocation),
		ofColor(160), oldMessage ));
	cursorYLocation += instructionFont.stringHeight(oldMessage);  // update cursor location
	if (_state == State::COMPLETED)
	{
		// Add "success" statement in GREEN
		textElementlist.push_back(GuiTextElement(GuiElement::Location(instructionStartLoc.x, cursorYLocation),
			ofColor(0,255,0), newMessage));
	}
	else
	{
		// add new message in black
		textElementlist.push_back(GuiTextElement(GuiElement::Location(instructionStartLoc.x, cursorYLocation),
			ofColor(0), newMessage));
	}
	cursorYLocation += instructionFont.stringHeight(newMessage);

	// Find where the progress "....." string will be added
	std::size_t lastnewLine = newMessage.find_last_of("\n");
	progressStringLocation.x = instructionStartLoc.x + instructionFont.stringWidth(newMessage.substr(lastnewLine + 1)) + 8;
	progressStringLocation.y = cursorYLocation - instructionFont.stringHeight("sample_text");

	// Load image GUI elements
	std::vector<std::string> imageList = instructionImages[_state];
	int cursorXLocation = instructionStartLoc.x;
	for (int i = 0; i < imageList.size(); i++)
	{
		ofImage img;
		img.load(ofToDataPath(ofFilePath::join("instructions", imageList.at(i))));
		img.resize(resizedImgDim, resizedImgDim);
		GuiImageElement imgEle(GuiElement::Location(cursorXLocation, cursorYLocation), ofColor(255), img);
		imageElementList.push_back(imgEle);
		cursorXLocation += (resizedImgDim + 10);
	}

	progressString = "";
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
	
	// draw footnote
	if (_state == State::DISPLAY_INSTRUCTION)
	{
		GuiElement::Location loc = guiElementPositions["FootnoteString"];
		ofSetColor(0);
		footnoteFont.drawString(footnoteString, loc.x, loc.y);
		int cursorYLoc = loc.y + footnoteFont.stringHeight(footnoteString);
		ofSetColor(128);
		footnoteFont.drawString(fwPathGuiString, loc.x, cursorYLoc);
	}
		
	// draw text
		for (int i = 0; i < textElementlist.size(); i++)
	{
		ofSetColor(textElementlist.at(i).color);
		auto loc = textElementlist.at(i).location;
		instructionFont.drawString(textElementlist.at(i).text, loc.x, loc.y);
	}

	// draw progress string
	ofSetColor(0);
	instructionFont.drawString(progressString, progressStringLocation.x, progressStringLocation.y);

	// draw image
	for (int i = 0; i < imageElementList.size(); i++)
	{
		ofSetColor(imageElementList.at(i).color);
		auto loc = imageElementList.at(i).location;
		imageElementList.at(i).image.draw(loc.x, loc.y);
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
#ifdef EMOTIBIT_FW_INST_DEBUG
	/*
	 * key	ascii	State
	 * 0	48		START
	 * 1 	49		DISPLAY_INSTRUCTION,
	 * 2	50		WAIT_FOR_FEATHER,
	 * 3	51		UPLOAD_WINC_FW_UPDATER_SKETCH,
	 * 4	52		RUN_WINC_UPDATER,
	 * 5 	53		UPLOAD_EMOTIBIT_FW,
	 * 6	54		COMPLETED,
	 */
	if (key >= 48 && key <= 54)
	{
		guiTestMode = true;
		guiTestState = key - 48;
		_state = State::START;
		clearGuiElements();
	}

	// press "f" oreceeded by a state aboe to jump to the failure screen for the state
	if (key == 'f')
	{
		if (guiTestMode)
		{
			guiTestState = (int)State::INSTALLER_ERROR;
		}
	}
#endif
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	// only accept key press in the first stage of display instruction
	if (_state == State::DISPLAY_INSTRUCTION)
	{
		if (((char)key) == 'L')
		{
			// Load FW file
			ofFileDialogResult fileLoadResult = ofSystemLoadDialog("Select a firmware .bin file (be sure it's compatible with your Feather)");
			if (fileLoadResult.bSuccess) {
				_fwFilePath = "\"" + fileLoadResult.filePath + "\"";
				//ofStringReplace(tempFilePath, "\\", "/"); // Handle Windows paths
				ofLogNotice() << "Firmware file loaded: " << _fwFilePath << endl;
			}
		}
	}
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
	guiElementPositions["TitleString"] = GuiElement::Location{ 10, 150 + int(titleFont.getLineHeight() / 2) };
	guiElementPositions["TitleImage"] = GuiElement::Location{ 724, 30 };
	guiElementPositions["Instructions"] = GuiElement::Location{ 30, 270 };  //!< State update messages are drawn relative to this location
	int footnoteStringHeight = footnoteFont.stringHeight(footnoteString);
	int yLoc = 768 - (6 * footnoteStringHeight);
	guiElementPositions["FootnoteString"] = GuiElement::Location{ 625, yLoc };
	
}

void ofApp::setupInstructionList()
{
	// Step based user instructions
	instructionList[State::START] = "";
	instructionList[State::DISPLAY_INSTRUCTION] = "1. Make sure the EmotiBit Hibernate switch is NOT set to HIB"
														"\n   !!CAUTION: Excessive force can break the HIB switch. Handle with care!!!"
													    "\n2. Make sure EmotiBit is stacked with Feather with Battery and SD-Card inserted"
													    "\n\t More information about stacking EmotiBit available at docs.emotibit.com"
														"\n3. Make sure EmotiBit is NOT PLUGGED to the computer."
														"\n4. Press space-bar to continue";
	
	instructionList[State::WAIT_FOR_FEATHER] = "5. Press Reset Button on the Feather"
		"\n6. Plug in the Feather using using a data-capable USB cable (as provided in the EmotiBit Kit)"
		"\n>>> Detecting Feather plug-in";
	instructionList[State::UPLOAD_WINC_FW_UPDATER_SKETCH] = ">>> Uploading WINC firmware updater sketch";
	instructionList[State::RUN_WINC_UPDATER] = ">>> Updating WINC FW";
	instructionList[State::UPLOAD_EMOTIBIT_FW] = ">>> Updating EmotiBit firmware";
	instructionList[State::COMPLETED] = "FIRMWARE UPDATE COMPLETED SUCCESSFULLY!";
	instructionList[State::INSTALLER_ERROR] = "FAILED";

	// Images to be displayed for each instruction
	// If you want to add any image to be displayed, just add the image name to the list
	//ToDo: There is currently no bounds on images goinging outside the window, if too many images have been added to the list
	instructionImages[State::START];
	instructionImages[State::DISPLAY_INSTRUCTION] = std::vector<std::string>{ "correctHibernateSwitch.jpg", "un-plugInEmotiBit.jpg" };
	instructionImages[State::WAIT_FOR_FEATHER] = std::vector<std::string>{ "pressResetButton.jpg", "plugInEmotiBit.jpg" };
	instructionImages[State::UPLOAD_WINC_FW_UPDATER_SKETCH];
	instructionImages[State::RUN_WINC_UPDATER];
	instructionImages[State::UPLOAD_EMOTIBIT_FW];
	instructionImages[State::COMPLETED];
	instructionImages[State::INSTALLER_ERROR];

	// WARNING message
	S_WARNING = "DO NOT UNPLUG OR RESET EMOTIBIT WHILE UPDATE IN PROGRESS";
	// footnote string
	footnoteString = "If you wish to load a custom firmware bin file, type \"L\"";
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
	errorImages[State::UPLOAD_WINC_FW_UPDATER_SKETCH];
	errorImages[State::RUN_WINC_UPDATER];
	errorImages[State::UPLOAD_EMOTIBIT_FW];
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
        if (currentComList.size() - comListOnStartup.size() > 2)
        {
            // found multiple new com ports
            ofLog(OF_LOG_NOTICE, "Multiple Ports detected ");
        }
        else if (currentComList.size() - comListOnStartup.size() == 2)
		{
			// Exactly 2 ports detected. Might be artifact of SiLabs driver for ESP32. Check if a COM port was lebelled as ESP32 port
            if(boardComList[Board::FEATHER_ESP_32].size())
            {
                ofLogNotice() << "Feather ESP32 detected on port: " + boardComList[Board::FEATHER_ESP_32].at(0);
                // A port was detected as ESP32!
                setBoard(Board::FEATHER_ESP_32);
                featherPort = boardComList[Board::FEATHER_ESP_32].at(0); // return the first element of the COM list assigned to the  board
                return 1; // return  1 to indicate Feather  found!
            }
            else
            {
                ofLog(OF_LOG_NOTICE, "2 ports detected, no board found!");
            }
		}
		else
		{
			// found 1 new COM port. The new COM port is taken as the Feather Port
			std::string newComPort = findNewComPort(comListOnStartup, currentComList);

			if (newComPort.compare(COM_PORT_NONE) != 0)
			{
                if (std::find(boardComList[Board::FEATHER_M0].begin(), boardComList[Board::FEATHER_M0].end(), newComPort) != boardComList[Board::FEATHER_M0].end())
                {
                    setBoard(Board::FEATHER_M0);
                }
                else if (std::find(boardComList[Board::FEATHER_ESP_32].begin(), boardComList[Board::FEATHER_ESP_32].end(), newComPort) != boardComList[Board::FEATHER_ESP_32].end())
                {
                    setBoard(Board::FEATHER_ESP_32);
                }
                if(getBoard() != Board::NONE)
                {
                    featherPort = newComPort;
                }
                else
                {
                    comListOnStartup = currentComList;
                }
				return currentComList.size() - comListOnStartup.size();
			}
		}
	}
	return currentComList.size() - comListOnStartup.size();
}

ofApp::Board ofApp::getBoard()
{
	return _board;
}

void ofApp::setBoard(ofApp::Board board)
{
	_board = board;
}

void ofApp::assignComPortToBoard(ofApp::Board board, std::string comPort)
{
	// add com port to board list if it does not already exist
	if (std::find(boardComList[board].begin(), boardComList[board].end(), comPort) == boardComList[board].end())
	{
		boardComList[board].push_back(comPort);
	}
}

std::vector<std::string> ofApp::getComListFromDeviceList(ofxIO::SerialDeviceInfo::DeviceList deviceList)
{
	std::vector<std::string> comPortList;
	for (int i = 0; i < deviceList.size(); i++)
	{
        // ToDo: needs a filter for /dev/cu.SLAB_USBtoUART. Looks like an artifact of SLABS drivers
        comPortList.push_back(deviceList[i].port());
        Board b;
        b = getBoardFromDeviceInfo(deviceList[i]);
        assignComPortToBoard(b, deviceList[i].port());
	}
	// sort the list
	std::sort(comPortList.begin(), comPortList.end());
	return comPortList;
}

ofxIO::SerialDeviceInfo::DeviceList ofApp::getDeviceList()
{
	return ofxIO::SerialDeviceUtils::listDevices();
}

ofApp::DeviceInfo ofApp::parseDeviceInfo(ofx::IO::SerialDeviceInfo deviceInfo)
{
	ofApp::DeviceInfo info;
	info.desc = deviceInfo.description();
	info.port = deviceInfo.port();
	// get hw info
	std::string temp = deviceInfo.hardwareId();
#ifdef TARGET_WIN32
	// get required substring
	if (temp.find("VID") != std::string::npos)
	{
		std::string hwinfo = temp.substr(temp.find("VID"));
		// split into parameter pairs
		std::vector<std::string> hwIdSplit = ofSplitString(hwinfo, "&");
		// logic to parse hwId
		// Ex: USB\VID_239A&PID_800B&REV_0100&MI_00
		for (int i = 0; i < hwIdSplit.size(); i++)
		{
			std::vector<std::string> idSegment = ofSplitString(hwIdSplit.at(i), "_");  // https://docs.microsoft.com/en-us/windows-hardware/drivers/hid/hidclass-hardware-ids-for-top-level-collections#special-purpose-hardware-id
			if (idSegment.at(0).compare("VID") == 0)
			{
				std::string vid = idSegment.at(1);
				transform(vid.begin(), vid.end(), vid.begin(), ::toupper);
				info.vid = vid;
			}
			else if (idSegment.at(0).compare("PID") == 0)
			{
				std::string pid = idSegment.at(1);
				transform(pid.begin(), pid.end(), pid.begin(), ::toupper);
				info.pid = pid;
			}
			else
			{
				// ToDo: extract other information if necessary
			}
		}
	}
#elif defined TARGET_OSX
    // example hardware id: /dev/cu.usbmodem14301, Adafruit Feather M0, USB VID:PID=239a:800b SNR=6A04F4CF50533336372E3120FF09280F
    std::vector<std::string> splitHwinfo = ofSplitString(temp, " ");
    for(int i=0;i<splitHwinfo.size();i++)
    {
        std::string s = splitHwinfo.at(i);
        if(s.find("VID") != std::string::npos)
        {
            std::string ids = ofSplitString(s, "=").back();
            std::string vid = ofSplitString(ids, ":").front();
            std::string pid = ofSplitString(ids,":").back();
            transform(vid.begin(), vid.end(), vid.begin(), ::toupper);
            transform(pid.begin(), pid.end(), pid.begin(), ::toupper);
            info.vid = vid;
            info.pid = pid;
        }
    }
#else
    // for linux
#endif
	return info;
}

ofApp::Board ofApp::getBoardFromDeviceInfo(ofx::IO::SerialDeviceInfo deviceInfo)
{
	ofApp::DeviceInfo info;
	info = parseDeviceInfo(deviceInfo);
	bool isFeatherM0 = false;
	
	if (std::find(ADARUIT_VID_LIST.begin(), ADARUIT_VID_LIST.end(), info.vid) != ADARUIT_VID_LIST.end())
	{
		// Device vendor detected as Adafruit
		if (std::find(ADARUIT_PID_LIST.begin(), ADARUIT_PID_LIST.end(), info.pid) != ADARUIT_PID_LIST.end())
		{
			// Device detected as Feather M0
			return Board::FEATHER_M0;
		}
	}
	else
	{
		// perform a check for ESP using device description
        // ToDo: Find a better way to detect ESP32. This approach is not "robust"
		std::string espDescIdentifier = "Silicon";
        std::string espSlabIdentifier = "SLAB";
#ifdef TARGET_WIN32
		if (info.desc.find(espDescIdentifier) != std::string::npos)
		{
			// It is ESP!
			return Board::FEATHER_ESP_32;
		}
#elif defined TARGET_OSX
        // if descriptions says silicon labs and port is SLAB_USBtoUART
        if (info.desc.find(espDescIdentifier) != std::string::npos && info.port.find(espSlabIdentifier) != std::string::npos)
		{
			// It is ESP!
			return Board::FEATHER_ESP_32;
		}
#endif
	}
	return Board::NONE;
}

std::vector<std::string> ofApp::getComPortList(bool printOnConsole)
{
	ofSerial serial;
	std::vector<std::string> comPortList;
	// get device list
	ofxIO::SerialDeviceInfo::DeviceList deviceList = getDeviceList();
	
	// get list of COM ports
	comPortList = getComListFromDeviceList(deviceList);	
	
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
		if (systemOutput.compare(threadedSystemCall.PIPE_OPEN_FAILED) == 0)
		{
			raiseError(threadedSystemCall.PIPE_OPEN_FAILED + "\n");
		}
		// thread execution complete
		systemCommandExecuted = false;
		// return the result of execution
		return threadedSystemCall.cmdResult;
	}
}

bool ofApp::updateUsingEspTool(std::string fwFilePath)
{
	std::string command;
	std::string filepath = "exec";
	if (!systemCommandExecuted)
	{
		ofLog(OF_LOG_NOTICE, "uploading FW on ESP");
#if defined(TARGET_OSX)
		ofSleepMillis(1000);
		command = ofFilePath::join(filepath, "mac");
		command = ofFilePath::join(command, "esptool");
		command = ofToDataPath(command);
#elif defined(TARGET_LINUX)
		// for linux
		ofSleepMillis(1000);
		command = ofFilePath::join(filepath, "linux");
		command = ofFilePath::join(command, "esptool");
		command = ofToDataPath(command);
#else
		command = ofFilePath::join(filepath, "win");
		command = ofFilePath::join(command, "esptool.exe");
		command = ofToDataPath(command);
#endif
		command = command + " " + "--chip esp32 --port " + featherPort +
			" --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB" +
			" 0x1000" + " " + ofToDataPath(ofFilePath::join("esp32","EmotiBit_stock_firmware.ino.bootloader.bin")) +
			" 0x8000" + " " + ofToDataPath(ofFilePath::join("esp32", "EmotiBit_stock_firmware.partitions.bin")) +
			" 0xe000" + " " + ofToDataPath(ofFilePath::join("esp32", "boot_app0.bin")) +
			" 0x10000" + " " + fwFilePath;
		ofLogNotice("Running: ") << command;
		//system(command.c_str());
		threadedSystemCall.setup(command, "Hash of data verified"); // the target response string is captured from observed output
		threadedSystemCall.startThread();
		systemCommandExecuted = true;
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

bool ofApp::updateUsingBossa(std::string fwFilePath)
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
            command = ofToDataPath(command);
            command = command + " " + "-i -d -U true -e -w -v -R -b -p " + programmerPort + " " + fwFilePath;
#else
			command = "bossac.exe";
            command = ofToDataPath(command);
            command = command + " " + "-i -d " + "--port=" + programmerPort + " -U true -i -e -w -v" + " " + fwFilePath + " -R";
#endif
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
	if (getBoard() == Board::FEATHER_M0)
	{
		if (_fwFilePath.empty())
		{
			return updateUsingBossa(ofToDataPath("EmotiBit_stock_firmware.ino.feather_m0.bin"));
		}
		else
		{
			return updateUsingBossa(_fwFilePath);
		}
	}
	else
	{
		if (_fwFilePath.empty())
		{
			return updateUsingEspTool(ofToDataPath("EmotiBit_stock_firmware.ino.feather_esp32.bin"));
		}
		{
			return updateUsingEspTool(_fwFilePath);
		}
	}
}

void ofApp::clearGuiElements()
{
	oldMessage = "";
	newMessage = "";
	textElementlist.clear();
	imageElementList.clear();
}