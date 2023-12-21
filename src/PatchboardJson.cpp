
#include "PatchboardJson.h"

PatchboardJson::ReturnCode PatchboardJson::parse(std::string jsonStr)
{
	Json::Reader reader;
	reader.parse(jsonStr, patchboard);

	inputType = "";
	outputType = "";
	patchcords.clear();

	if (!patchboard.isMember("patchboard"))
	{
		_lastErrMsg = "[PatchboardJson::parse] JSON tag not found: patchboard";
		return ReturnCode::ERR_TAG_NOT_FOUND;
	}
	if (!patchboard["patchboard"].isMember("settings"))
	{
		_lastErrMsg = "[PatchboardJson::parse] JSON tag not found: settings";
		return ReturnCode::ERR_TAG_NOT_FOUND;
	}

	// parse the input type
	if (!patchboard["patchboard"]["settings"].isMember("input"))
	{
		_lastErrMsg = "[PatchboardJson::parse] JSON tag not found: settings>input";
		return ReturnCode::ERR_TAG_NOT_FOUND;
	}
	if (!patchboard["patchboard"]["settings"]["input"].isMember("type"))
	{
		_lastErrMsg = "[PatchboardJson::parse] JSON tag not found: type";
		return ReturnCode::ERR_TAG_NOT_FOUND;
	}
	inputType = patchboard["patchboard"]["settings"]["input"]["type"].asString();

	// parse the output type
	if (!patchboard["patchboard"]["settings"].isMember("output"))
	{
		_lastErrMsg = "[PatchboardJson::parse] JSON tag not found: settings>output";
		return ReturnCode::ERR_TAG_NOT_FOUND;
	}
	if (!patchboard["patchboard"]["settings"]["output"].isMember("type"))
	{
		_lastErrMsg = "[PatchboardJson::parse] JSON tag not found: type";
		return ReturnCode::ERR_TAG_NOT_FOUND;
	}
	outputType = patchboard["patchboard"]["settings"]["output"]["type"].asString();

	// parse the patchcords
	if (!patchboard["patchboard"].isMember("patchcords"))
	{
		_lastErrMsg = "[PatchboardJson::parse] JSON tag not found: patchcords";
		return ReturnCode::ERR_TAG_NOT_FOUND;
	}
	if (!patchboard["patchboard"]["patchcords"].isArray())
	{
		_lastErrMsg = "[PatchboardJson::parse] Incorrect JSON structure: patchcords must be an array";
		return ReturnCode::ERR_FORMAT_INCORRECT;
	}
	int nPatches = patchboard["patchboard"]["patchcords"].size();
	if (nPatches == 0)
	{
		_lastErrMsg = "[PatchboardJson::parse] Incorrect JSON structure: patchcords must be an array ";
		return ReturnCode::ERR_FORMAT_INCORRECT;
	}
	for (int p = 0; p < nPatches; p++)
	{
		if (!patchboard["patchboard"]["patchcords"][p].isMember("input"))
		{
			_lastErrMsg = "[PatchboardJson::parse] JSON tag not found: patchcords>input";
			return ERR_TAG_NOT_FOUND;
		}
		std::string key = patchboard["patchboard"]["patchcords"][p]["input"].asString();

		if (!patchboard["patchboard"]["patchcords"][p].isMember("output"))
		{
			_lastErrMsg = "[PatchboardJson::parse] JSON tag not found: patchcords>output";
			return ERR_TAG_NOT_FOUND;
		}
		std::string value = patchboard["patchboard"]["patchcords"][p]["output"].asString();

		std::vector<std::string> values = patchcords[key];
		values.push_back(value);
		//std::cout << indent << key << ":" << value << std::endl;
		patchcords[key] = values;
	}

	return ReturnCode::SUCCESS;
};

size_t PatchboardJson::getNumPatches()
{
	return patchcords.size();
}

std::string PatchboardJson::getLastErrMsg()
{
	return _lastErrMsg;
}
