#include "EmotiBitLsl.h"

static const string MARKER_INFO_NAME_LABEL = "name";
static const string MARKER_INFO_SOURCE_ID_KEY_LABEL = "sourceId";

EmotiBitLsl::ReturnCode EmotiBitLsl::addMarkerInput(string jsonStr)
{

	Json::Reader reader;
	Json::Value settings;
	reader.parse(jsonStr, settings);

	MarkerStreamInfo markerStreamInfo;

	if (!settings.isMember("lsl"))
	{
		return ReturnCode::ERROR_LSL_TAG_NOT_FOUND;
	}
	if (!settings["lsl"].isMember("marker"))
	{
		return ReturnCode::ERROR_MARKER_TAG_NOT_FOUND;
	}
	if (!settings["lsl"]["marker"].isMember(MARKER_INFO_NAME_LABEL))
	{
		return ReturnCode::ERROR_NAME_NOT_FOUND;
	}
	markerStreamInfo.name = settings["lsl"]["marker"][MARKER_INFO_NAME_LABEL].asString();
	if (settings["lsl"]["marker"].isMember(MARKER_INFO_SOURCE_ID_LABEL))
	{
		markerStreamInfo.sourceId = settings["lsl"]["marker"][MARKER_INFO_SOURCE_ID_LABEL].asString();
	}
	
	if (!markerStreamInfo.sourceId.empty())
	{
		markerStreamInfo.receiver = std::make_shared<ofxLSL::Receiver<string>>(markerStreamInfo.name, markerStreamInfo.sourceId);
	}
	else
	{
		markerStreamInfo.receiver = std::make_shared<ofxLSL::Receiver<string>>(markerStreamInfo.name);
	}

	markerInputs.push_back(markerStreamInfo);

	return ReturnCode::SUCCESS;
}

size_t EmotiBitLsl::getNumDataOutputs()
{
	return patchboard.getNumPatches();
}

size_t EmotiBitLsl::getNumMarkerInputs()
{
	return markerInputs.size();
}

vector<string> EmotiBitLsl::createMarkerInputPackets(uint16_t &packetCounter)
{
	vector<string> packets;
	for (int j = 0; j < markerInputs.size(); j++)
	{
		if (markerInputs.at(j).receiver->isConnected())
		{
			// ToDo: consider moving marker receiver->flush() to a separate thread to tighten up timing
			auto markerSamples = markerInputs.at(j).receiver->flush();
			// for all samples received
			for (int i = 0; i < markerSamples.size(); i++)
			{
				markerInputs.at(j).rxCount;
				auto markerSample = markerSamples.at(i);
				std::stringstream payload;
				payload.precision(7);
				uint16_t payloadLen = 0;

				addToPayload(EmotiBitPacket::PayloadLabel::LSL_MARKER_RX_TIMESTAMP, payload, payloadLen);
				addToPayload(markerSample->timeStamp + markerSample->timeCorrection, payload, payloadLen);
				addToPayload(EmotiBitPacket::PayloadLabel::LSL_MARKER_SRC_TIMESTAMP, payload, payloadLen);
				addToPayload(markerSample->timeStamp, payload, payloadLen);
				addToPayload(EmotiBitPacket::PayloadLabel::LSL_LOCAL_CLOCK_TIMESTAMP, payload, payloadLen);
				addToPayload(lsl::local_clock(), payload, payloadLen);
				addToPayload(EmotiBitPacket::PayloadLabel::LSL_MARKER_DATA, payload, payloadLen);
				for (auto channel : markerSample->sample) 
				{
					addToPayload(channel, payload, payloadLen);
				}
				packets.push_back(EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::LSL_MARKER, packetCounter++, payload.str(), payloadLen));

				// ToDo: consider if we even need a marker sample to get cross domain points.
				//		Can we just not do: LC = lsl::local_clock, and LM = lsl::local_clock() - timeCorretion()?
				// send TX packet for LCxLM
				payload.clear();
				payloadLen = 0;
				addToPayload(EmotiBitPacket::PayloadLabel::LSL_LOCAL_CLOCK_TIMESTAMP, payload, payloadLen);
				addToPayload(markerSample->timeStamp + markerSample->timeCorrection, payload, payloadLen);
				addToPayload(EmotiBitPacket::PayloadLabel::LSL_MARKER_SRC_TIMESTAMP, payload, payloadLen);
				addToPayload(markerSample->timeStamp, payload, payloadLen);
				packets.push_back(EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::TIMESTAMP_CROSS_TIME, packetCounter++, payload.str(), payloadLen));


				// ToDo: Consider if TIMESTAMP_CROSS_TIME packet sending needs to be in a different spot
				double lsltime = lsl::local_clock();
				string timestampLocal = ofGetTimestampString(EmotiBitPacket::TIMESTAMP_STRING_FORMAT);
				payload.clear();
				payloadLen = 0;
				addToPayload(EmotiBitPacket::TypeTag::TIMESTAMP_LOCAL, payload, payloadLen);
				addToPayload(timestampLocal, payload, payloadLen);
				addToPayload(EmotiBitPacket::PayloadLabel::LSL_LOCAL_CLOCK_TIMESTAMP, payload, payloadLen);
				addToPayload(lsltime, payload, payloadLen);
				packets.push_back(EmotiBitPacket::createPacket(EmotiBitPacket::TypeTag::TIMESTAMP_CROSS_TIME, packetCounter++, payload.str(), payloadLen));
			}
		}
	}
}

template <class T>
void EmotiBitLsl::addToPayload(const T &element, std::stringstream &payload, uint16_t &payloadLen)
{
	payload << element << EmotiBitPacket::PAYLOAD_DELIMITER;
	payloadLen++;
}

