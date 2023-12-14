/**************************************************************************/
/*!
		@file     EmotiBitLsl.h

		This is a library to LSL communications in the EmotiBit ecosystem.

		EmotiBit invests time and resources providing this open source code,
		please support EmotiBit and open-source hardware by purchasing
		products from EmotiBit!

		Written by Sean Montgomery for EmotiBit.

		BSD license, all text here must be included in any redistribution
*/
/**************************************************************************/

#pragma once

#include <string>
#include "json/json.h"
#include "ofxLSL.h"
#include "PatchboardJson.h"
#include "EmotiBitPacket.h"

using namespace EmotiBit;

class EmotiBitLsl
{
public:
	enum ReturnCode {
		SUCCESS = 0,
		ERROR_LSL_TAG_NOT_FOUND = -1,
		ERROR_MARKER_TAG_NOT_FOUND = -2,
		ERROR_NAME_NOT_FOUND = -3,
		ERROR_SRCID_NOT_FOUND = -4,
	};

	struct MarkerStreamInfo {
		// For more info: https://github.com/sccn/liblsl/blob/5eded5c1d381a1a5fbbcce105edfaa53f009176a/include/lsl_cpp.h#L161
		std::string name = "";  //!< marker stream inlet name for LSL.
		std::string sourceId = "";  //!< marker stream inlet sourceId for LSL.		
		size_t rxCount = 0;
		std::shared_ptr<ofxLSL::Receiver<string>> receiver;
	};
	vector<MarkerStreamInfo> markerInputs;

	static const string MARKER_INFO_NAME_LABEL;
	static const string MARKER_INFO_SOURCE_ID_LABEL;


	ofxLSL::Sender();

	PatchboardJson patchboard;

	void update();
	ReturnCode addMarkerInput(string jsonStr);
	vector<string> createMarkerInputPackets(uint16_t &packetCounter);

	ReturnCode addDataStreamOutputs(string jsonStr);
	vector<MarkerStreamInfo> getMarkerStreamInfo();
	size_t getNumDataOutputs();
	size_t getNumMarkerInputs();

	// ToDo: Move to EmotiBitPacket
	template<typename T>
	void addToPayload(const T &element, std::stringstream &payload, uint16_t &payloadLen);

};
