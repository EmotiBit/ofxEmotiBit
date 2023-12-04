#include "ofMain.h"
#include "ofxEmotiBitVersion.h"
#include "ofxJSON.h"

namespace SoftwareVersionChecker
{
	/*!
	@brief Check if a later version of EmotiBit software is available to download
	*/
	void checkLatestVersion();

	/*!
	@brief Test if the response ot ping contains keywords
	@param response received when pinging an IP
	@return True, is network is available, else false
	*/
	bool testPingResponse(std::string pingResponse);
}