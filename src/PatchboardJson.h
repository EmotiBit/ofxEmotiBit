#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "json/json.h"

class PatchboardJson
{
public:
	//static unordered_map<string, vector<string>> extractPatchcordMap(string jsonStr);
	//static string extractInputType(string jsonStr);
	//static string extractOutputType(string jsonStr);

	enum ReturnCode {
		SUCCESS = 0,
		ERROR_NO_PATHBOARD_FOUND = -1,
		ERROR_NO_SETTINGS_FOUND = -2,
		ERROR_NO_INPUT_TYPE_FOUND = -3,
		ERROR_NO_OUTPUT_TYPE_FOUND = -4,
		ERROR_NO_PATCHCORDS_FOUND = -5,
		ERROR_PATCHBOARD_FORMAT = -6
	};

	std::string inputType = "";
	std::string outputType = "";
	std::unordered_map<std::string, std::vector<std::string>> patchcords;

	Json::Value patchboard;

	ReturnCode parse(std::string jsonStr);
	size_t getNumPatches();
};