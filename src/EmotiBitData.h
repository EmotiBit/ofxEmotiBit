
#pragma once

#include "EmotiBitPacket.h"
#include <unordered_map>

class EmotiBitData
{
public:
	std::unordered_map <std::string, std::vector<float>> _data; // Stores latest available data.

	EmotiBitData();
	//EmotiBitData(const EmotiBitData& oldObj);

	std::vector<float> readData(EmotiBitPacket::TypeTag typeTag); // Reads latest available data.
	std::vector<float> readData(std::string typeTag); // Reads latest available data.
	void push_back(std::string typeTag, float data);	// Adds data
	void clear();
};