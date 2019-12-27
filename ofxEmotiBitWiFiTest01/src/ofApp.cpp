#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
	// we don't want to be running too fast
	ofSetVerticalSync(true);
	ofSetFrameRate(3);

	emotiBitWiFi.begin();
}

//--------------------------------------------------------------
void ofApp::update()
{
	emotiBitWiFi.processAdvertising();
}

//--------------------------------------------------------------
void ofApp::draw(){
	string allIps = "";

	emotibitIps = emotiBitWiFi.getEmotiBitIPs();
	for (auto it = emotibitIps.begin(); it != emotibitIps.end(); it++)
	{
		allIps += it->first + "\n";
	}

	string data;
	emotiBitWiFi.readData(data);
	if (data.length() > 0) {
		cout << "Data: " << data;
	}

	ofSetHexColor(0x000000);
	ofDrawBitmapString("Data: \n" + data, 10, 20);

	ofDrawBitmapString("EmotBits:\n" + allIps, 10, 60);

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	if ((char)key == 'c')
	{
		if (emotibitIps.size() > 0) {
			emotiBitWiFi.connect(emotibitIps.begin()->first);
			//emotiBitWiFi.connect("192.168.0.36");
		}
	}
	else if ((char)key == 'd')
	{
		emotiBitWiFi.disconnect();
	}
	else if ((char)key == 'r')
	{
		string packet = EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::RECORD_BEGIN, emotiBitWiFi.controlPacketCounter++, "", 0);
		cout << packet;
		emotiBitWiFi.sendControl(packet);
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

