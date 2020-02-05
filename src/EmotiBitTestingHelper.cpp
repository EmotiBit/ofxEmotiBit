#include "EmotiBitTestingHelper.h"

EmotiBitTestingHelper::EmotiBitTestingHelper()
{

}


void EmotiBitTestingHelper::setLogFilename(const string &filename)
{
	//_testingResultsLog.setDirPath(dirPath);
	_testingResultsLog.setFilename(filename);
	_testingResultsLog.startThread();

}

void EmotiBitTestingHelper::update(const vector<string> &splitPacket, const EmotiBitPacket::Header &packetHeader)
{
	updateEda(splitPacket, packetHeader);
	updatePpg(splitPacket, packetHeader);
	updateThermopile(splitPacket, packetHeader);
	static uint64_t printTimer = ofGetElapsedTimeMillis();
	if (ofGetElapsedTimeMillis() - printTimer > 200)
	{
		printTimer = ofGetElapsedTimeMillis();
		cout << "PPG Red: " << ofToString(_ppgRed, 0) << ", PPG IR: " << ofToString(_ppgIR, 0) << ", PPG Green: " << ofToString(_ppgGreen, 0);
		cout << ", EL: " << ofToString(_edl, 6) << ", ER: " << ofToString(_edr, 6) << ", ER P2P: " << ofToString(_edrFiltP2P, 6);
		cout << ", Therm: " << ofToString(_thermopile, 2) << endl;

	}
}
void EmotiBitTestingHelper::updateSerialNumber(const string &userNote)
{
	if (userNote.substr(0,3).compare("SN:") == 0)
	{
		_results.serialNumber = userNote;
		cout << "Serial Number -- " << userNote << endl;
		_results.testStatus = "";
		printResults();
	}
}

//void EmotiBitTestingHelper::updateTestStatus(const string &userNote)
//{
//	if (userNote.compare("PASS") == 0 || userNote.compare("FAIL") == 0)
//	{
//		_results.testStatus = userNote;
//		printResults();
//	}
//}

void EmotiBitTestingHelper::updateSdCardFilename(const string &filename)
{
	_results.sdCardFilename = filename;
}

void EmotiBitTestingHelper::updateEda(const vector<string> &splitPacket, const EmotiBitPacket::Header &packetHeader)
{
	// Code to handle EDR RMS estimation for testing
	// Plan
	// LP filter at 1Hz
	// Store last 5 seconds
	// Find max - min for last 5 seconds

	// ToDo: Make this code more versatile

	float edaFs = 15;
	float edaRmsWinLen = 5.f; // seconds
	float lpFreq = 1.f;
	static deque<float> edaRmsQueue;
	static ofxBiquadFilter1f edaRmsFilter = ofxBiquadFilter1f(OFX_BIQUAD_TYPE_LOWPASS, lpFreq / edaFs, 0.7071);
	float elLpFreq = 0.16f;
	static ofxBiquadFilter1f elFilter = ofxBiquadFilter1f(OFX_BIQUAD_TYPE_LOWPASS, elLpFreq / edaFs, 0.7071);
	float minEdaRms;
	float maxEdaRms;
	if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::EDL) == 0)
	{
		for (size_t i = 0; i < packetHeader.dataLength; i++)
		{
			_edl = elFilter.update(ofToFloat(splitPacket.at(EmotiBitPacket::headerLength + i)));
		}
	}

	if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::EDR) == 0)
	{
		for (size_t i = 0; i < packetHeader.dataLength; i++)
		{
			edaRmsQueue.push_back(edaRmsFilter.update(ofToFloat(splitPacket.at(EmotiBitPacket::headerLength + i))));
		}
		while (edaRmsQueue.size() > edaFs * edaRmsWinLen)
		{
			edaRmsQueue.pop_front();
		}

		minEdaRms = 1000;
		maxEdaRms = -1000;
		for (size_t i = 0; i < edaRmsQueue.size(); i++)
		{
			minEdaRms = std::min(minEdaRms, edaRmsQueue.at(i));
			maxEdaRms = std::max(maxEdaRms, edaRmsQueue.at(i));
		}
		_edr = edaRmsQueue.back();
		_edrFiltP2P = maxEdaRms - minEdaRms;
	}
}

void EmotiBitTestingHelper::updatePpg(const vector<string> &splitPacket, const EmotiBitPacket::Header &packetHeader)
{
	// ToDo: Make this code more versatile

	float ppgFs = 25;
	float lpFreq = 0.2f;
	static ofxBiquadFilter1f ppgRedFilter = ofxBiquadFilter1f(OFX_BIQUAD_TYPE_LOWPASS, lpFreq / ppgFs, 0.7071);
	static ofxBiquadFilter1f ppgIRFilter = ofxBiquadFilter1f(OFX_BIQUAD_TYPE_LOWPASS, lpFreq / ppgFs, 0.7071);
	static ofxBiquadFilter1f ppgGreenFilter = ofxBiquadFilter1f(OFX_BIQUAD_TYPE_LOWPASS, lpFreq / ppgFs, 0.7071);

	if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::PPG_RED) == 0)
	{
		for (size_t i = 0; i < packetHeader.dataLength; i++)
		{
			_ppgRed = ppgRedFilter.update(ofToFloat(splitPacket.at(EmotiBitPacket::headerLength + i)));
		}
	}

	if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::PPG_INFRARED) == 0)
	{
		for (size_t i = 0; i < packetHeader.dataLength; i++)
		{
			_ppgIR = ppgIRFilter.update(ofToFloat(splitPacket.at(EmotiBitPacket::headerLength + i)));
		}
	}

	if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::PPG_GREEN) == 0)
	{
		for (size_t i = 0; i < packetHeader.dataLength; i++)
		{
			_ppgGreen = ppgGreenFilter.update(ofToFloat(splitPacket.at(EmotiBitPacket::headerLength + i)));
		}
	}
}

void EmotiBitTestingHelper::updateThermopile(const vector<string> &splitPacket, const EmotiBitPacket::Header &packetHeader)
{
	// ToDo: Make this code more versatile

	float thermopileFs = 7.5;
	float lpFreq = 1.f;
	static ofxBiquadFilter1f thermopileFilter = ofxBiquadFilter1f(OFX_BIQUAD_TYPE_LOWPASS, lpFreq / thermopileFs, 0.7071);

	if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::THERMOPILE) == 0)
	{
		for (size_t i = 0; i < packetHeader.dataLength; i++)
		{
			_thermopile = thermopileFilter.update(ofToFloat(splitPacket.at(EmotiBitPacket::headerLength + i)));
		}
	}
}


void EmotiBitTestingHelper::printResults()
{
	_testingResultsLog.push("SN: " + _results.serialNumber + "\n");
	_testingResultsLog.push("Filename: " + _results.sdCardFilename + "\n");
	_testingResultsLog.push("PPG: " + ofToString(_results.ppgRed, 0) + ", " + ofToString(_results.ppgIR, 0) + ", " + ofToString(_results.ppgGreen, 0) + "\n");
	_testingResultsLog.push("EDL: ");
	for (auto result : _results.edl)
	{
		_testingResultsLog.push(ofToString(result, 10) + ", ");
	}
	_testingResultsLog.push("\n");
	_testingResultsLog.push("EDR: ");
	for (auto result : _results.edr)
	{
		_testingResultsLog.push(ofToString(result, 10) + ", ");
	}
	_testingResultsLog.push("\n");
	_testingResultsLog.push("EDR P2P: ");
	for (auto result : _results.edrP2P)
	{
		_testingResultsLog.push(ofToString(result, 10) + ", ");
	}
	_testingResultsLog.push("\n");
	_testingResultsLog.push("Therm: ");
	for (auto result : _results.thermopile)
	{
		_testingResultsLog.push(ofToString(result, 2) + ", ");
	}
	//_testingResultsLog.push("\n");
	//_testingResultsLog.push("Status: ");
	//_testingResultsLog.push(_results.testStatus);

	// Serialize the print
	_testingResultsLog.push("\n--\n");
	_testingResultsLog.push(_results.serialNumber + ", ");
	_testingResultsLog.push("(Filename)," + _results.sdCardFilename + ", ");
	_testingResultsLog.push("(PPG), " + ofToString(_results.ppgRed, 0) + ", " + ofToString(_results.ppgIR, 0) + ", " + ofToString(_results.ppgGreen, 0) + ", ");
	_testingResultsLog.push("(EDL), ");
	for (auto result : _results.edl)
	{
		_testingResultsLog.push(ofToString(result, 10) + ", ");
	}
	_testingResultsLog.push("(EDR), ");
	for (auto result : _results.edr)
	{
		_testingResultsLog.push(ofToString(result, 10) + ", ");
	}
	_testingResultsLog.push("(EDR P2P), ");
	for (auto result : _results.edrP2P)
	{
		_testingResultsLog.push(ofToString(result, 10) + ", ");
	}
	_testingResultsLog.push("(Therm), ");
	for (auto result : _results.thermopile)
	{
		_testingResultsLog.push(ofToString(result, 2) + ", ");
	}
	//_testingResultsLog.push("(Status), ");
	//_testingResultsLog.push(_results.testStatus);
	_testingResultsLog.push("\n********\n");
}

void EmotiBitTestingHelper::pushEdlEdrResult()
{
	_results.edl.push_back(_edl);
	_results.edr.push_back(_edr);
	printResults();
}

void EmotiBitTestingHelper::pushEdrP2pResult()
{
	_results.edrP2P.push_back(_edrFiltP2P);
	printResults();
}

void EmotiBitTestingHelper::clearEdaResults()
{
	_results.edl.clear();
	_results.edr.clear();
	_results.edrP2P.clear();
	printResults();
}

void EmotiBitTestingHelper::recordPpgResult()
{
	_results.ppgRed = _ppgRed;
	_results.ppgIR = _ppgIR;
	_results.ppgGreen = _ppgGreen;
	printResults();
}

void EmotiBitTestingHelper::clearPpgResults()
{
	_results.ppgRed = -1;
	_results.ppgIR = -1;
	_results.ppgGreen = -1;
	printResults();
}

void EmotiBitTestingHelper::popEdlEdrResult()
{
	if (_results.edl.size() > 0)
	{
		_results.edl.pop_back();
	}
	if (_results.edr.size() > 0)
	{
		_results.edr.pop_back();
	}
	printResults();
}

void EmotiBitTestingHelper::popEdrP2pResult()
{
	if (_results.edrP2P.size() > 0)
	{
		_results.edrP2P.pop_back();
	}
	printResults();
}

void EmotiBitTestingHelper::pushThermopileResult()
{
	_results.thermopile.push_back(_thermopile);
	printResults();
}

void EmotiBitTestingHelper::popThermopileResult()
{
	if (_results.thermopile.size() > 0)
	{
		_results.thermopile.pop_back();
	}
	printResults();
}

void EmotiBitTestingHelper::clearAllResults()
{
	_results.serialNumber = "";
	_results.sdCardFilename = "";
	_results.testStatus = "";
	_results.edl.clear();
	_results.edr.clear();
	_results.edrP2P.clear();
	_results.thermopile.clear();
	clearPpgResults();
}

