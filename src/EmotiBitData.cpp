
#include "EmotiBitData.h"

EmotiBitData::EmotiBitData()
{
}
//
//EmotiBitData::EmotiBitData(const EmotiBitData::EmotiBitData& oldObj)
//{
//	_data = oldObj._data;
//}

std::vector<float> EmotiBitData::readData(EmotiBitPacket::TypeTag typeTag)
{
	return readData(typeTag);
}

std::vector<float> EmotiBitData::readData(std::string typeTag)
{
	auto it = _data.find(typeTag);
	if (it == _data.end())
	{
		vector<float> empty;
		return empty;
	}
	return it->second;
}

void EmotiBitData::push_back(std::string typeTag, float data)
{
	std::vector<float> dataVec;
	dataVec.push_back(data);
	auto result = _data.emplace(typeTag, dataVec);
	if (!result.second)
	{
		// existing typeTag found, push data onto existing buffer
		result.first->second.push_back(data);
	}
}

void EmotiBitData::clear()
{
	_data.clear();
}