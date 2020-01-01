#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
	//ofSetVerticalSync(true);
	//ofSetFrameRate(60);

	emotiBitWiFi.begin();

	logger.setFilename("EmotiBitWiFiLog" + ofToString(emotiBitWiFi.dataPort) + ".txt");
	logger.setDirPath(ofToDataPath(""));

	logger.startThread();

}

//--------------------------------------------------------------
void ofApp::update()
{
	emotiBitWiFi.processAdvertising();
}

//--------------------------------------------------------------
void ofApp::draw(){
	vector<string> dataPackets;
	emotiBitWiFi.readData(dataPackets);
	string data;
	static uint32_t missedPackets = 0;
	static uint16_t lastPacketNumber = 0;
	EmotiBitPacket::Header header;
	for (string packet : dataPackets)
	{
		logger.push(packet);
		data = packet;

		EmotiBitPacket::getHeader(packet, header);
		if (header.packetNumber - lastPacketNumber > 1)
		{
			missedPackets += header.packetNumber - (lastPacketNumber + 1);
			lastPacketNumber = header.packetNumber;
			cout << "MISSED PACKETS -- TOTAL: " << missedPackets << endl;
		}
	}

	ofSetHexColor(0x000000);
	ofDrawBitmapString("Data: \n" + data, 10, 20);
	ofDrawBitmapString("Connected: " + emotiBitWiFi.connectedEmotibitIp, 10, 60);
	ofDrawBitmapString("EmotiBits:\n", 10, 100);

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
		emotiBitWiFi.connect(ofToInt(ofToString(key)) - 1);
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
