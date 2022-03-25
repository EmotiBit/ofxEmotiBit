/**************************************************************************/
/*!
	@file     ofApp.h

	This is an application to handle flashing Adafruit Feather M0 Wifi with the EmotiBit FW.

	EmotiBit invests time and resources providing this open source code,
	please support EmotiBit and open-source hardware by purchasing
	products from EmotiBit!

	Written by Nitin Nair for EmotiBit.

	BSD license, all text here must be included in any redistribution
*/
/**************************************************************************/
#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "unordered_map"
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include "ThreadedSystemCall.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
		/*!
			@brief Setup the locations where the GUI elements are going to be drawn
		*/
		void setupGuiElementPositions();

		/*!
			@brief Setup Instructions for every step
		*/
		void setupInstructionList();

		/*!
			@brief Setup Error messages for every state
		*/
		void setupErrorMessageList();

		/*!
			@brief Function that progress state of the state machine
		*/
		void progressToNextState();

		/*!
			@brief Function to detect new COM port after feather is plugged in
			@return number of new COM ports detected
		*/
		int detectFeatherPlugin();
		
		/*!
			@brief Function to run bossa to upload sketch(bin) to feather
			@param filePath relative path of the bin file of the sketch
			@return true if sketch was uploaded correctly
		*/
		bool updateUsingBossa(std::string filePath);

		/*!
			@brief Function to check progress on the separate thread executing system calls
			@return true if system call was completed successfully, else false
		*/
		bool checkSystemCallResponse();

		/*!
			@brief funcition to get list of COM ports available on the system
			@param printOnconsole set true, if you want to echo the available COM ports on the console
			@return vector of all available COM ports
		*/
		std::vector<std::string> getComPortList(bool printOnConsole = false);
		
		/*!
			@brief Set the connected device into programmer mode by connecting/disconnecting with 1200 baud
			@param programmerPort COM port of device in programmer/bootloader mode
			@param programmerPortComList vector of available COM ports after device is set in programmer mode
			@return true, if device was set to programmer mode successfully, else false
		*/
		bool initProgrammerMode(std::string &programmerPort);
		
		/*!
			@brief Function to call WiFi101 updater and update the WINC firmware
			@return true if WINC FW was updated correctly
		*/
		bool runWincUpdater();

		/*!
			@brief Function to upload WINC firmwareUpdater sketch. required to run Winc updater
			@return true if sketch uploaded successfully
		*/
		bool uploadWincUpdaterSketch();
		
		/*!
			@brief function to update the EmotiBit FW on device
		*/
		bool uploadEmotiBitFw();
		
		/*!
			@brief find if a new COM port has been added to the system
			@param oldList initial list of COM ports
			@param newList updated list of COM ports
			@return New COM port name is detected, else return COM_PORT_NONE
		*/
		std::string findNewComPort(std::vector<std::string> oldList, std::vector<std::string> newList);
		
		void raiseError(std::string additionalMessage = "");

		// This order shold not be changed. The Feather port is updated in WAIT_FOR_FEATHER and RUN_WINC_UPDATER
		// that feather port is then used in the next sequential step
		enum State {
			START = 0,
			DISPLAY_INSTRUCTION,
			WAIT_FOR_FEATHER,
			UPLOAD_WINC_FW_UPDATER_SKETCH,
			RUN_WINC_UPDATER,
			UPLOAD_EMOTIBIT_FW,
			COMPLETED,
			DONE,
			INSTALLER_ERROR,
			LENGTH
		}_state;


		bool systemCommandExecuted = false;
		ThreadedSystemCall threadedSystemCall;
		ofImage titleImage;
		ofTrueTypeFont	instructionFont;
		ofTrueTypeFont progressFont;
		ofTrueTypeFont	titleFont;
		std::string progressString = "";
		const int STATE_TIMEOUT = 10;
		bool captureComListOnStartup = false;
		std::vector<std::string> comListOnStartup;
		std::vector<std::string> comListWithProgrammingPort;
		std::string userInfo1;
		std::string featherPort;
		const int MAX_NUM_TRIES_PING_1200 = 15; // keep it under the TIMEOUT specified
		const int MAX_NUM_TRIES_BOSSAC = 3; // keep it under the TIMEOUT specified
		const std::string DELIMITER = ",";
		const std::string COM_PORT_NONE = "COMX";
		unordered_map<int, std::string> errorMessageList;
		unordered_map<int, std::string> onScreenInstructionList;
		unordered_map<int, std::vector<std::string>> instructionImages;
		unordered_map<int, std::vector<std::string>> errorImages;
		const int resizedImgDim = 250; // w = h
		std::string displayedErrorMessage;
		std::string onScreenInstruction;
		std::vector<ofImage>onScreenInstructionImage;
		std::vector<ofImage> disaplyedErrorImage;
		int pingProgTryCount = 0;
		int bossacTryCount = 0;
		uint32_t stateStartTime;
		struct GuiElementPos {
			int x = 0;
			int y = 0;
			GuiElementPos() {}
			GuiElementPos(int x, int y) : x{ x }, y{ y } {}
		};
		unordered_map<std::string, GuiElementPos> guiElementPositions;
};
