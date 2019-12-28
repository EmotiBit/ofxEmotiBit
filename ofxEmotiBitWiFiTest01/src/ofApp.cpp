#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
	// we don't want to be running too fast
	ofSetVerticalSync(true);
	ofSetFrameRate(60);

	logger.setFilename("EmotiBitWiFiLog.txt");
	logger.setDirPath(ofToDataPath(""));

	logger.startThread();

	emotiBitWiFi.begin();
}

//--------------------------------------------------------------
void ofApp::update()
{
	emotiBitWiFi.processAdvertising();
}

//--------------------------------------------------------------
void ofApp::draw(){
	string data;
	emotiBitWiFi.readData(data);
	if (data.length() > 0) {
		logger.push("Data: " + data);
		//cout << "Data: " << data;
	}

	ofSetHexColor(0x000000);
	ofDrawBitmapString("Data: \n" + data, 10, 20);
	ofDrawBitmapString("Connected: " + emotiBitWiFi.connectedEmotibitIp, 10, 60);
	ofDrawBitmapString("EmotBits:\n", 10, 100);

	emotibitIps = emotiBitWiFi.getEmotiBitIPs();
	int y = 100;
	for (auto it = emotibitIps.begin(); it != emotibitIps.end(); it++)
	{
		if (it->second.isAvailable)
		{
			ofSetHexColor(0x000000);
		}
		else
		{
			ofSetHexColor(0x888888);
		}
		ofDrawBitmapString(it->first, 10, y+=20);
	}

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int k){
	char key = (char)k;
	cout << "keyReleased: " << (char)key << endl;
	if ((char)key == 'c')
	{
		if (emotibitIps.size() > 0) {
			emotiBitWiFi.connect(emotibitIps.begin()->first);
			//emotiBitWiFi.connect("192.168.0.36");
		}
	}
	else if (key == 'd')
	{
		emotiBitWiFi.disconnect();
	}
	else if (key == 'r')
	{
		string packet = EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::RECORD_BEGIN, emotiBitWiFi.controlPacketCounter++, "", 0);
		cout << packet;
		emotiBitWiFi.sendControl(packet);
	}
	else if (key == '1' || key == '2' || key == '3' || key == '4' || key == '5'
		|| key == '6' || key == '7' || key == '8' || key == '9')
	{
		connectTo(ofToInt(ofToString(key)));
	}
}

void ofApp::connectTo(int i)
{
	int counter = 0;
	for (auto it = emotibitIps.begin(); it != emotibitIps.end(); it++)
	{
		counter++;
		if (counter == i)
		{
			emotiBitWiFi.connect(it->first);
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

