
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
		return ReturnCode::ERROR_NO_PATHBOARD_FOUND;
	}
	if (!patchboard["patchboard"].isMember("settings"))
	{
		return ReturnCode::ERROR_NO_SETTINGS_FOUND;
	}

	// parse the input type
	if (!patchboard["patchboard"]["settings"].isMember("input"))
	{
		return ReturnCode::ERROR_NO_INPUT_TYPE_FOUND;
	}
	if (!patchboard["patchboard"]["settings"]["input"].isMember("type"))
	{
		return ReturnCode::ERROR_NO_INPUT_TYPE_FOUND;
	}
	inputType = patchboard["patchboard"]["settings"]["input"]["type"].asString();

	// parse the output type
	if (!patchboard["patchboard"]["settings"].isMember("output"))
	{
		return ReturnCode::ERROR_NO_OUTPUT_TYPE_FOUND;
	}
	if (!patchboard["patchboard"]["settings"]["output"].isMember("type"))
	{
		return ReturnCode::ERROR_NO_OUTPUT_TYPE_FOUND;
	}
	outputType = patchboard["patchboard"]["settings"]["output"]["type"].asString();

	// parse the patchcords
	if (!patchboard["patchboard"]["settings"].isMember("patchcords"))
	{
		return ReturnCode::ERROR_NO_PATCHCORDS_FOUND;
	}
	if (!patchboard["patchboard"]["settings"]["patchcords"].isArray())
	{
		return ReturnCode::ERROR_NO_PATCHCORDS_FOUND;
	}
	int nPatches = patchboard["patchboard"]["settings"]["patchcords"].size();
	if (nPatches == 0)
	{
		return ReturnCode::ERROR_NO_PATCHCORDS_FOUND;
	}
	for (int p = 0; p < nPatches; p++)
	{
		if (!patchboard["patchboard"]["settings"]["patchcords"][p].isMember("input"))
		{
			return ERROR_PATCHBOARD_FORMAT;
		}
		std::string key = patchboard["patchboard"]["settings"]["patchcords"][p]["input"].asString();

		if (!patchboard["patchboard"]["settings"]["patchcords"][p].isMember("output"))
		{
			return ERROR_PATCHBOARD_FORMAT;
		}
		std::string value = patchboard["patchboard"]["settings"]["patchcords"][p]["output"].asString();

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
