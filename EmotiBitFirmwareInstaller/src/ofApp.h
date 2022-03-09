#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "unordered_map"

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
		
		void setupInstructionList();

		void setupErrorMessageList();

		void resetStateTimer();

		bool detectFeatherPlugin();
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
		
		bool runWincUpdater();

		bool uploadWincUpdaterSketch();

		/*!
			@brief Function to update the FW on the WINC1500 on device
		*/
		//void updateWiFiFw();
		
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
		
		enum State {
			TIMEOUT = -1,
			WAIT_FOR_FEATHER = 0,
			UPLOAD_WINC_FW_UPDATER_SKETCH,
			RUN_WINC_UPDATER,
			UPLOAD_EMOTIBIT_FW,
			COMPLETED,
			EXIT,
			LENGTH
		}_state;

		ofTrueTypeFont	instructionFont;
		const int STATE_TIMEOUT = 20;
		bool globalTimerReset = false;
		std::vector<std::string> comListOnStartup;
		std::vector<std::string> comListWithProgrammingPort;
		std::string userInfo1;
		std::string featherPort;
		const int MAX_NUM_TRIES_PING = 3;
		const std::string DELIMITER = ",";
		const std::string COM_PORT_NONE = "COMX";
		unordered_map<int, std::string> errorMessageList;
		unordered_map<int, std::string> onScreenInstructionList;
		std::string errorMessage;
		std::string currentInstruction;
		struct BossaCommand {
			std::string windows = "data\\bossac.exe -i -d -U true -e -w -v -R -b -p ";
			std::string unix = "bossac -i -d -U true -e -w -v -R -b -p ";
		}bossaCommand;
};
