#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofLogToConsole();
	ofSetLogLevel(OF_LOG_VERBOSE);
	// update the FW on the WINC1500
	updateWiFiFw();
	ofSleepMillis(1000);
	// upload EmotiBit FW on device
	updateEmotiBitFw();
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

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
		ofLog(OF_LOG_VERBOSE, "Available COM ports: " + comPorts);
	}
	return comPortList;
}

bool ofApp::initProgrammerMode(std::string &programmerPort, std::vector<std::string> &programmerPortComList)
{
	ofSerial serial;
	// get initial list of COM ports
	std::vector<std::string> initialComPortList = getComPortList(true);
	std::vector<std::string> updatedComPortList;
	// ping every COM port to try and set it to programmer mode
	for (int j = 0; j < MAX_NUM_TRIES_PING; j++)
	{
		ofLog(OF_LOG_NOTICE, "########## Try: " + ofToString(j + 1));
		for (int i = 0; i < initialComPortList.size(); i++)
		{
			ofLog(OF_LOG_VERBOSE,"##### Pinging " + initialComPortList.at(i));
			// connecting to port at 1200 baud sets it to programmer/bootloader mode
			serial.setup(initialComPortList.at(i), 1200);
			serial.close();
			// wait before reading available COM ports
			ofSleepMillis(1000);
			// read updated COM list.
			// if device was set to programmer mode, the COM port will change
			updatedComPortList = getComPortList(true);
			// check if a new OCM port has been detected
			std::string newPort = findNewComPort(initialComPortList, updatedComPortList);
			if (newPort.compare(COM_PORT_NONE) != 0)
			{
				// return the new COm port and the list of COM ports with the feather in programmer mode
				programmerPort = newPort;
				programmerPortComList = updatedComPortList;
				return true;
			}
		}
	}
	return false;
}
std::string ofApp::findNewComPort(std::vector<std::string> oldList, std::vector<std::string> newList)
{
	// for every COm port in the new list
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

void ofApp::updateWiFiFw()
{
	std::string programmerPort;
	std::vector<std::string> programmerPortComList;
	// try to set feather in programmer mode
	if(initProgrammerMode(programmerPort, programmerPortComList))
	{
		// run command to upload WiFi Updater sketch
		ofLog(OF_LOG_NOTICE, "uploading WiFi updater sketch");
		std::string command = "data\\bossac.exe -i -d -U true -e -w -v -R -b -p " + programmerPort + " data\\WINC\\FirmwareUpdater.ino.feather_m0.bin";
		system(command.c_str());
		ofLog(OF_LOG_NOTICE, "DONE!");
		ofLog(OF_LOG_NOTICE, "WiFi updater sketch uploaded");
		ofSleepMillis(2000);

		// get updated COM list. the feather returns back to feather port, after the bossa flash is completed.
		std::vector<std::string> newComPortList = getComPortList(true);
		// find the fether port 
		std::string featherPort = findNewComPort(programmerPortComList, newComPortList); // old list, new list
		if (featherPort.compare(COM_PORT_NONE) != 0)
		{
			// found feather port
			ofLog(OF_LOG_NOTICE, "Feather found at: " + featherPort);
			ofLog(OF_LOG_NOTICE, "UPDATING WINC FW");
			std::string command = "data\\WINC\\FirmwareUploader.exe -port " + featherPort + " -firmware " + "data\\WINC\\m2m_aio_3a0.bin";
			system(command.c_str());
			ofLog(OF_LOG_NOTICE, "WINC FW updated!");
		}
		else
		{
			ofLog(OF_LOG_ERROR, "Feather port Not Found. WINC FW UPDATE FAILED!");
		}
	}
	else
	{
		ofLog(OF_LOG_ERROR, "Failed to enter programmer mode");
	}
}

void ofApp::updateEmotiBitFw()
{
	std::string programmerPort;
	std::vector<std::string> programmerPortComList;
	// try to set device in programmer mode
	if (initProgrammerMode(programmerPort, programmerPortComList))
	{
		// device successfully set to programmer mode!
		// run command to upload WiFi Updater sketch
		ofLog(OF_LOG_NOTICE, "Uploading EmotiBit FW");
		std::string command = "data\\bossac.exe -i -d -U true -e -w -v -R -b -p " + programmerPort + " data\\EmotiBit_stock_firmware.ino.feather_m0.bin";
		system(command.c_str());
		ofLog(OF_LOG_NOTICE, "DONE!");
		ofLog(OF_LOG_NOTICE, "EmotiBit FW uploaded!");
		ofSleepMillis(2000);
	}
	else
	{
		ofLog(OF_LOG_ERROR, "Failed to enter programmer mode");
		while (1);
	}
}