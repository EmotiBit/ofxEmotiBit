#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "json/json.h"

class PatchboardJson
{
public:

	// ToDo: move ReturnCode to Patchboard.h
	enum ReturnCode {
		SUCCESS = 0,
		ERR_TAG_NOT_FOUND = -1,
		ERR_FORMAT_INCORRECT = -2
	};

	std::string inputType = "";
	std::string outputType = "";
	std::unordered_map<std::string, std::vector<std::string>> patchcords;

	Json::Value patchboard;

	ReturnCode parse(std::string jsonStr);
	size_t getNumPatches();
	std::string getLastErrMsg();

	std::string _lastErrMsg = "";
};