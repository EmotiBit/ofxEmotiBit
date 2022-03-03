#pragma once

#include "ofMain.h"

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
		bool initProgrammerMode(std::string &programmerPort, std::vector<std::string> &programmerPortComList);
		
		/*!
			@brief Function to update the FW on the WINC1500 on device
		*/
		void updateWiFiFw();
		
		/*!
			@brief function to update the EmotiBit FW on device
		*/
		void updateEmotiBitFw();
		
		/*!
			@brief find if a new COM port has been added to the system
			@param oldList initial list of COM ports
			@param newList updated list of COM ports
			@return New COM port name is detected, else return COM_PORT_NONE
		*/
		std::string findNewComPort(std::vector<std::string> oldList, std::vector<std::string> newList);
		
		
		const int MAX_NUM_TRIES_PING = 3;
		const std::string DELIMITER = ",";
		const std::string COM_PORT_NONE = "COMX";
		struct BossaCommand {
			std::string windows = "data\\bossac.exe -i -d -U true -e -w -v -R -b -p ";
			std::string unix = "bossac -i -d -U true -e -w -v -R -b -p ";
		}bossaCommand;
};
