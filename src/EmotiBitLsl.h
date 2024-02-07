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
#include <unordered_map>

using namespace EmotiBit;

class EmotiBitLsl
{
public:
	//! ReturnCode for EmotiBitLsl functions
	//! SUCCESS == 0
	enum ReturnCode {
		SUCCESS = 0,
		ERR_TAG_NOT_FOUND = -1,
		ERR_FORMAT_INCORRECT = -2,
		ERR_VALUE_MISMATCH = -3
	};
	
	//! Information about marker streams
	struct MarkerStreamInfo {
		// For more info: https://github.com/sccn/liblsl/blob/5eded5c1d381a1a5fbbcce105edfaa53f009176a/include/lsl_cpp.h#L161
		std::string name = "";  //!< marker stream inlet name for LSL.
		std::string sourceId = "";  //!< marker stream inlet sourceId for LSL.		
		size_t rxCount = 0;
		std::shared_ptr<ofxLSL::Receiver<string>> receiver;
	};
	vector<MarkerStreamInfo> _markerInputs;	// Markers that are being captured

	static const string MARKER_INFO_NAME_LABEL;
	static const string MARKER_INFO_SOURCE_ID_LABEL;

	//! @brief Adds an LSL marker to capture list
	//! @param jsonStr with fields lsl:marker:name and lsl:marker:sourceId
	//! @return ReturnCode SUCCESS==0
	ReturnCode addMarkerInput(string jsonStr);

	//! @brief Creates a vector of EmotiBitPacket strings from inbound LSL markers
	//! @param packetCounter reference to increment with each packet
	//! @return EmotiBitPacket strings with LSL markers for sending to device
	vector<string> createMarkerInputPackets(uint16_t &packetCounter);

	//! @brief Gets LSL MarkerStreamInfo of streams being captured
	//! @return MarkerStreamInfo list
	vector<MarkerStreamInfo> getMarkerStreamInfo();

	//! @brief Gets number of LSL marker streams
	//! @return number of LSL marker streams
	size_t getNumMarkerInputs();

	//! @brief Adds LSL data stream outputs
	//! @param jsonStr to create a patchboard mapping EmotiBit data to LSL outputs
	//! @param sourceId EmotiBit ID
	//! @return ReturnCode SUCCESS==0
	ReturnCode addDataStreamOutputs(string jsonStr, string sourceId);

	//! @brief Returns whether sourceId is in the list of LSL outputs
	//! @param sourceId EmotiBit ID
	//! @return ReturnCode SUCCESS==0
	bool isDataStreamOutputSource(string sourceId);

	//! @brief Gets the present number of data outputs
	//! @param sourceId EmotiBit ID
	//! @return number of present data outputs
	size_t getNumDataOutputs(string sourceId);

	//! @brief clears the list data output streams
	//! @return void
	void clearDataStreamOutputs();

	//! @brief Gets the last captured error message 
	//! @return last captured error message 
	string getlastErrMsg();

	// ToDo: Move to EmotiBitPacket
	//! @brief Adds an element to the passed payload reference
	//! @param element EmotiBit TypeTag or PayloadLabel to add to the payload
	//! @param payload stringstream referemce to add payload to
	//! @param payloadLen reference to increment payload size
	template<typename T>
	void addToPayload(const T &element, std::stringstream &payload, uint16_t &payloadLen);

	//! @brief Adds samples to the LSL output
	//! @param _values to add. Must be a single point in time with a value for each channel of stream. 
	//! @param typeTag EmotiBit TypeTag of associated values
	//! @param sourceId EmotiBit ID
	//! @return true on success
	template <typename T>
	bool addSample(const vector<T> &_values, const std::string &typeTag, const std::string &sourceId);

	ofxLSL::Sender _lslSender;
	string _lastErrMsg = "";
	unordered_map<string, PatchboardJson> _patchboards; // <sourceId, patchboard>
	unordered_map<string, string> _outChanTypeMap; // LSL channel <<sourceId, name>, type>

};
