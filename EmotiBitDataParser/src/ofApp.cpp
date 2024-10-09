#include "ofApp.h"
#include <stdio.h>
#include "Poco/Timestamp.h"
#include <algorithm>

//--------------------------------------------------------------
void ofApp::setup() {
	ofLogToConsole();
	ofSetLogLevel(OF_LOG_VERBOSE);
#ifdef TARGET_OSX
    ofSetDataPathRoot("../Resources/");
    cout<<"Changed the data pathroot for Release"<<endl;
#endif
	writeOfxEmotiBitVersionFile();
	ofSetWindowTitle("EmotiBit Data Parser (v" + ofxEmotiBitVersion + ")");

	ofBackground(255, 255, 255);
	legendFont.load(ofToDataPath("verdana.ttf"), 12, true, true);
	subLegendFont.load(ofToDataPath("verdana.ttf"), 7, true, true);
	parsedDataFormat.loadFromFile("parsedDataFormat.json");
	processButton.addListener(this, &ofApp::startProcessing);

	int guiXPos = 0;
	int guiYPos = 20;
	int guiWidth = ofGetWindowWidth();
	int guiPosInc = guiWidth + 1;
	//guiPanels.resize(1);  // This fails in OF v0.11.2 with "attempting to reference a deleted function" error
	guiPanels.at(0).setDefaultWidth(guiWidth);
	guiPanels.at(0).setup("startRecording","junk.xml", guiXPos, -guiYPos);
	guiPanels.at(0).add(processStatus.setup("Status", GUI_STATUS_IDLE));
	guiPanels.at(0).add(processButton.set(GUI_PANEL_LOAD_FILE, false));
	guiXPos += guiPosInc;
	//guiPanels.at(1).setDefaultWidth(ofGetWindowWidth() - guiXPos);
	//guiPanels.at(1).setup("paths", "junk.xml", guiXPos, -guiYPos);
	//guiPanels.at(1).add(inputPath.setup("Input Path:", "[Add path to EmotiBit csv file]"));
	//guiPanels.at(1).add(outputPath.setup("Output Path:", "[Add path to output directory]"));

	dataLine = "Click above to load data file";

	ofSetColor(128, 128, 128);

	linesPerLoop = 1000;

	fileExt = ".csv";

	if (argFileName.size() > 0) {
		ofFile argExists(argFileName);
		if (!argExists.exists()) {
			cout << "ERROR: " << argFileName << " does not exist, try absolute path" << endl;
			ofExit();
		}
		else {
			processButton.set(true);
		}
	}
}

size_t ofApp::getMaxLoggerSize() {
	size_t out = 0;
	for (auto it = loggers.cbegin(); it != loggers.cend(); ++it) {
		std::max(out, it->second->size(LoggerThread::LoggerQueue::PUSH));
		std::max(out, it->second->size(LoggerThread::LoggerQueue::POP));
	}
	return out;
}

//--------------------------------------------------------------
void ofApp::startProcessing(bool & processing) {

	if (processing) {
		bool fileLoadedGUI = false;
		string filePathGUI;
		if (argFileName.size() == 0) {
			ofFileDialogResult fileLoadResult = ofSystemLoadDialog("Open an EmotiBit raw csv data file");
			if (fileLoadResult.bSuccess) {
				fileLoadedGUI = true;
				filePathGUI = fileLoadResult.filePath;
			}
		}
		if (argFileName.size() > 0 || fileLoadedGUI) {
			if (guiPanels.at(0).getControl(GUI_PANEL_LOAD_FILE) != NULL) {
				guiPanels.at(0).getControl(GUI_PANEL_LOAD_FILE)->setBackgroundColor(ofColor(255, 0, 0));
				processStatus.setBackgroundColor(ofColor(255, 0, 0));
				processStatus.getParameter().fromString(GUI_STATUS_PROCESSING);
			}
			
			string inFilePath;
			if (argFileName.size() > 0) { //gets the absolute file path of the command line argument
				inFilePath = argFileName;
			}
			else {
				inFilePath = filePathGUI;
			}
			inFile.open(inFilePath, ios::in);
			string tempFilePath = inFilePath;
			ofStringReplace(tempFilePath, fileExt + "\0", "\0"); // drop the .csv extension


			tempFilePath = inFilePath;
			ofStringReplace(tempFilePath, "\\", "/"); // Handle Windows paths
			vector<string> pathSegments = ofSplitString(tempFilePath, "/");
			string filename = pathSegments.back();
			inFileBase = filename;
			ofStringReplace(inFileBase, fileExt + "\0", "\0");

			// Remove all the files matching [filebase]*[fileExt]
			ofFile file(inFilePath);
			//populate the directory object
			inFileDir = file.getEnclosingDirectory();
			ofDirectory dir(inFileDir);
			dir.listDir();
			for (int i = 0; i < dir.size(); i++) {
				string dirFilename = dir.getName(i);
				if (filename.compare(dirFilename) == 0) {
					cout << "Processing file: " << dirFilename << endl;
					continue;
				}
				try {
					if (inFileBase.compare(0, inFileBase.size(), dirFilename, 0, inFileBase.size()) == 0 &&
						dirFilename.compare(dirFilename.size() - fileExt.size(), fileExt.size(), fileExt) == 0) {
						string dirPath = inFileDir + dirFilename;
						cout << "Removing file: " << dirPath << endl;
						remove(dirPath.c_str());
					}
				}
				catch (exception e) {
				}
			}

			string outFilePath = inFileDir + inFileBase + "_" + timestampFilenameString + fileExt;
			cout << "Creating file: " << outFilePath << endl;
			auto loggerElem = loggers.emplace(timestampFilenameString, new LoggerThread("", outFilePath));
			if (loggerElem.second) { // emplace worked
				loggerElem.first->second->startThread();
				loggerElem.first->second->push("RD,TS_received,TS_sent,AK,RoundTrip,");
			}

			//for (uint8_t i = 0; i < (uint8_t) EmotiBitPacket::PacketType::length; i++) {
			//	string typeTag(EmotiBitPacket::typeTags[i]);
			//	std::remove(filename.c_str());
			//}

			lineCounter = 0;
			currentState = State::PARSING_TIMESTAMPS;
		}
		if (!inFile.is_open()) {
			processButton.set(false);  // Processing halted, reset GUI display, remove files, and clear loggers
		}
	}
	else {
		if (guiPanels.at(0).getControl(GUI_PANEL_LOAD_FILE) != NULL) {
			guiPanels.at(0).getControl(GUI_PANEL_LOAD_FILE)->setBackgroundColor(ofColor(0, 0, 0));
			processStatus.setBackgroundColor(ofColor(0, 0, 0));
			processStatus.getParameter().fromString(GUI_STATUS_IDLE);
		}
		if (inFile.is_open()) {
			inFile.close();
		}
		closeLoggers();
		currentState = State::IDLE;
	}
}


//--------------------------------------------------------------
void ofApp::update() {
	if (currentState != State::WARNING_INSUFFICIENT_TIMESYNCS &&inFile.is_open()) {
		int lines = 0;
		while (lines < linesPerLoop) {
			lines++;
			getline(inFile, dataLine);

			bool keepGoing = true;
			if (inFile.eof()) {
				eofCounter++;
				// the eofCounter was put in place to correct for false EOF detections.
				// ToDo: figure out a test to detect if this is still required
				if (eofCounter > 10) {
					keepGoing = false;
					ofLogNotice() << "End of file: " + ofToString(eofCounter);
				}
				else {
					inFile.clear(); // Dirty hack to deal with mangled data reading
					inFile.ignore(1000, '\n');
				}
			}
			else if (inFile.fail()) {
				cout << "File read failed: stream failbit (or badbit). error state" << endl;
				keepGoing = false;
			}
			else if (inFile.fail()) {
				cout << "File read failed: stream badbit. error state" << endl;
				keepGoing = false;
			}
			else {
				eofCounter = 0;
			}

			if (!keepGoing) {
				if (currentState == State::PARSING_TIMESTAMPS) {
					//ofMap()
					timeSyncMap = calculateTimeSyncMap(allTimestampData);
					// check if enough time syncs were found to create a map.
					//processButton.set(false);
					currentState = State::PARSING_DATA;
					nLinesInFile = lineCounter;
					lineCounter = 0;
					inFile.clear(); // clear the eof status
					inFile.seekg(0, ios::beg); // go back to beginning of file

					string filename = inFileDir + inFileBase + "_" + "timeSyncMap" + fileExt;
					cout << "Creating file: " << filename << endl;
					ofstream mFile;
					mFile.open(filename.c_str(), ios::out);
					mFile << timeSyncMap.columnHeaders + "TimeSyncsReceived,EmotiBitStartTime, EmotiBitEndTime, DataParserVersion" << endl;
					mFile << ofToString(timeSyncMap.anchorPoints[EmotiBitPacket::TypeTag::TIMESTAMP_EMOTIBIT].first, 6) << ","
						<< ofToString(timeSyncMap.anchorPoints[EmotiBitPacket::TypeTag::TIMESTAMP_EMOTIBIT].second, 6) << ",";
					// Add column headers for additional time domains read from parsed data format file
					for (int i = 0;i <= (int)parsedDataFormat.additionalTimestamps.size() - 1; i++)
					{
						auto timeDomain = parsedDataFormat.additionalTimestamps.at(i);
						mFile << ofToString(timeSyncMap.anchorPoints[timeDomain].first, 6) << ","
							<< ofToString(timeSyncMap.anchorPoints[timeDomain].second, 6) << ",";
					}
					mFile <<ofToString(allTimestampData.size()) << ","
						<<ofToString(recordedDataTimeRange.emotibitStartTime) << ","
						<<ofToString(recordedDataTimeRange.emotibitEndTime) << ","
						<<ofxEmotiBitVersion;
					mFile.close();

				}
				else if (currentState == State::PARSING_DATA) {
					if (allTimestampData.size() < 2)
					{
						ofLogNotice() << ofToString(allTimestampData.size()) + " Timesyncs Found";
						ofLog(OF_LOG_NOTICE, timesyncsWarning);
						currentState = State::WARNING_INSUFFICIENT_TIMESYNCS;
					}
					else
					{
						ofExit();
					}
					//processButton.set(false);
				}
				break;
			}
			else {
				lineCounter++;
				parseDataLine(dataLine);
			}
		}
	}
	// reset the GUI panel text and BG color
	if (currentState == State::WARNING_INSUFFICIENT_TIMESYNCS)
	{
		guiPanels.at(0).getControl(GUI_PANEL_LOAD_FILE)->setBackgroundColor(ofColor(0, 0, 0));
		processStatus.setBackgroundColor(ofColor(0, 0, 0));
		if (cmdLineStart)
		{
			ofExit();
		}
		else
		{
			closeLoggers();
			processStatus.getParameter().fromString(GUI_STATUS_IDLE);
		}
	}
}

void ofApp::ParsedDataFormat::loadFromFile(std::string filename, bool absolute)
{
	ofxJSONElement jsonSettings;
	try
	{
		// ToDo: find a place for these settings to be stored. a struct in ofApp?
		ofFile parsedDataFormatFile(ofToDataPath(filename));
		if (parsedDataFormatFile.exists())
		{
			if (jsonSettings.open(ofToDataPath(filename, absolute)))
			{
				if (jsonSettings.isMember("timestampColumns"))
				{
					int numEntries = jsonSettings["timestampColumns"].size();
					for (int i = 0; i < numEntries; i++)
					{
						if (jsonSettings["timestampColumns"][i].isMember("addToOutput"))
						{
							if (jsonSettings["timestampColumns"][i]["addToOutput"].asBool())
							{
								std::string colHeader = jsonSettings["timestampColumns"][i]["columnHeader"].asString();
								parsedDataHeaders.insert(parsedDataHeaders.begin(), colHeader);
								additionalTimestamps.push_back(jsonSettings["timestampColumns"][i]["identifier"].asString());
							}
						}
						else
						{
							ofLogNotice() << "addToOutput not found";
						}
					}
					ofLog(OF_LOG_NOTICE, "Loaded " + filename + ": \n" + jsonSettings.getRawString(true));
				}
				else
				{
					ofLogNotice("Timestamp column list not found") << "Parsing with EmotiBit timestamps";
				}
			}
			else
			{
				ofLogNotice() << "Parsing with only EmotiBit timestamps";
			}
		}
		else
		{
			ofLogNotice("File not found") << filename + ".Parsing with only EmotiBit timestamps";
		}
	}
	catch (exception e)
	{
		ofLog(OF_LOG_ERROR, "ERROR: Failed to load " + filename + ": \n" + jsonSettings.getRawString(true));
	}
}

std::string ofApp::ParsedDataFormat::getParsedFileColHeaders()
{
	std::string colHeaders;
	for (int i = 0; i < parsedDataHeaders.size(); i++)
	{
		colHeaders += parsedDataHeaders.at(i);
		colHeaders += EmotiBitPacket::PAYLOAD_DELIMITER;
	}
	return colHeaders;
}

int ofApp::getShortestRtIndex(vector<pair<int, int>> rtIndexes) {
	int out;
	sort(rtIndexes.begin(), rtIndexes.end()); // Sort and track original indexes
	out = rtIndexes.at(0).second;
	return out;
}

pair<ofApp::TimestampData, ofApp::TimestampData> ofApp::getBestTimestampIndexes(const vector<TimestampData> &timestampData) {
	// Cases Best to Worst
	// Q1 : Q4
	// Q1 : Q3
	// Q2 : Q4
	// Q2 : Q3
	// Q1 : Q2
	// Q3 : Q4
	// Q2 : Q2
	// Q3 : Q3
	// Q1 : Q1
	// Q4 : Q4
	// Sorted by pair<roundTripTime, index>
	// Returns pair<index-A, index-B>

	pair<ofApp::TimestampData, ofApp::TimestampData> out;
	pair<int, int> shortestRtInd;


	// ToDo: improve algorithm finding shortest round trips
	// -- Deal with cases where recording is stopped when emotibit is offline (i.e. no Q4 round trips)

	// Sort the round trip times into quartiles
	int q2Ind = ceil(timestampData.size() / 4);
	int q3Ind = ceil(timestampData.size() / 2);
	int q4Ind = ceil(timestampData.size() * 3 / 4);

	vector<pair<int, int>> q1;
	for (int i = 0; i < q2Ind; i++) {
		q1.push_back(make_pair(timestampData.at(i).roundTrip, i));
	}
	vector<pair<int, int>> q2;
	for (int i = q2Ind; i < q3Ind; i++) {
		q2.push_back(make_pair(timestampData.at(i).roundTrip, i));
	}
	vector<pair<int, int>> q3;
	for (int i = q3Ind; i < q4Ind; i++) {
		q3.push_back(make_pair(timestampData.at(i).roundTrip, i));
	}
	vector<pair<int, int>> q4;
	for (int i = q4Ind; i < timestampData.size(); i++) {
		q4.push_back(make_pair(timestampData.at(i).roundTrip, i));
	}

	// Select the best quartiles 
	if (q1.size() > 0 && q4.size() > 0) {
		out = make_pair(timestampData.at(getShortestRtIndex(q1)), timestampData.at(getShortestRtIndex(q4)));
		return out;
	}
	else if (q1.size() > 0 && q3.size() > 0) {
		out = make_pair(timestampData.at(getShortestRtIndex(q1)), timestampData.at(getShortestRtIndex(q3)));
		return out;
	}
	else if (q2.size() > 0 && q4.size() > 0) {
		out = make_pair(timestampData.at(getShortestRtIndex(q2)), timestampData.at(getShortestRtIndex(q4)));
		return out;
	}
	else if (q2.size() > 0 && q3.size() > 0) {
		out = make_pair(timestampData.at(getShortestRtIndex(q2)), timestampData.at(getShortestRtIndex(q3)));
		return out;
	}
	else if (q1.size() > 0 && q2.size() > 0) {
		out = make_pair(timestampData.at(getShortestRtIndex(q1)), timestampData.at(getShortestRtIndex(q2)));
		return out;
	}
	else if (q3.size() > 0 && q4.size() > 0) {
		out = make_pair(timestampData.at(getShortestRtIndex(q3)), timestampData.at(getShortestRtIndex(q4)));
		return out;
	}
	// ToDo: consider if we should add a warning when the timesyncs from the same quartile are used
	else if (q2.size() > 1) {
		sort(q2.begin(), q2.end()); // Sort and track original indexes
		out = make_pair(timestampData.at(q2.at(0).second), timestampData.at(q2.at(1).second));
		return out;
	}
	else if (q3.size() > 1) {
		sort(q3.begin(), q3.end()); // Sort and track original indexes
		out = make_pair(timestampData.at(q3.at(0).second), timestampData.at(q3.at(1).second));
		return out;
	}
	else if (q1.size() > 1) {
		sort(q1.begin(), q1.end()); // Sort and track original indexes
		out = make_pair(timestampData.at(q1.at(0).second), timestampData.at(q1.at(1).second));
		return out;
	}
	else if (q4.size() > 1) {
		sort(q4.begin(), q4.end()); // Sort and track original indexes
		out = make_pair(timestampData.at(q4.at(0).second), timestampData.at(q4.at(1).second));
		return out;
	}
	else {
		// We should never end up here
		ofLog(OF_LOG_ERROR, "ERROR: getBestTimestampIndexes() failed to find 2 points");
		out = make_pair(timestampData.at(0), timestampData.at(timestampData.size() - 1));
		return out;
	}
}

ofApp::TimeSyncMap ofApp::calculateTimeSyncMap(vector<TimestampData> &timestampData) {
	// Plan: Create a mapping from EmotiBit timestamps to computer POSIX timestamps
	// Use the fastest round trip from first and last quartile of timestamp data to get good esimation over whole dataset
	// Calculate the computer-to-EmotiBit travel time to adjust for lag
	// Use the fastest 5% of the round trips in each quartile to calculate c-to-e travel time

	//for (size_t i = 0; i < timestampData.size(); ) {
	//	timestampData.
	//}
	int totalRttFound = 0;
	// erase any blank lines
	for (auto it = timestampData.begin(); it != timestampData.end(); ) {
		if (it->roundTrip < 0) {
			ofLog(OF_LOG_NOTICE, "Removing erroneous RTT: " + ofToString(it->roundTrip));
			it = timestampData.erase(it);
		}
		else {
			it++;
			totalRttFound++;
		}
	}
	ofLog(OF_LOG_NOTICE, "Total RTT's found: " + ofToString(totalRttFound));
	TimeSyncMap tsMap;
	long double e0 = 0;
	long double e1 = 0;
	long double c0 = 0;
	long double c1 = 1;
	if (timestampData.size() == 0)
	{
		ofLogNotice() << "calculateTimeSyncMap: 0 timesyncs found" << endl;
		// update e0 and e1 to be min and max timestamp recorded.
		// c0 = 0 and c1 = 1. Whole data will be mapped to 1970, jan 1st
		e0 = recordedDataTimeRange.emotibitStartTime;
		e1 = recordedDataTimeRange.emotibitEndTime;
		c1 = c0 + (e1 - e0) / 1000.f;
	}
	else if (timestampData.size() == 1)
	{
		ofLogNotice() << "calculateTimeSyncMap: 1 timesync found" << endl;
		long double e_x, c_x;
		std::string ts;
		std::time_t c;
		long double m;
		size_t lastDelim;
		size_t lastNChar;
		// calculate the epoch time from the single Computer time received
		e_x = timestampData.back().TS_received;
		ts = timestampData.back().TS_sent;
		lastDelim = ts.find_last_of('-'); // find subsecond decimal
		lastNChar = ts.size() - lastDelim - 1;
		c = getEpochTime(std::wstring(ts.begin(), ts.end() - lastNChar - 1)); // Convert to epoch time without subsecond decimal
		m = ((float)atoi(ts.substr(lastDelim + 1, lastNChar).c_str())) / pow(10.f, lastNChar); // Convert subsecond
		c_x = (long double)c + m; // Append subsecond as decimal;
		c_x += timestampData.back().roundTrip / 2.f / 1000.f; // adjust computer time by 1/2 the shortest round-trip time

		uint32_t emotibitTime = timestampData.back().TS_received;
		if (emotibitTime - recordedDataTimeRange.emotibitStartTime >= recordedDataTimeRange.emotibitEndTime - emotibitTime)
		{
			// timestamp received is closer to the end
			// consider the single RTT to be e1/c1 pair
			e1 = e_x;
			c1 = c_x;
			// calculate e0/c0
			e0 = recordedDataTimeRange.emotibitStartTime;
			c0 = c1 - (e1 - e0)/1000.f;
		}
		else
		{
			// timestamp received is closer to the beginning
			// consider the single RTT to be e0/c0 pair
			e0 = e_x;
			c0 = c_x;

			// calculate e1/c1 pair
			e1 = recordedDataTimeRange.emotibitEndTime;
			c1 = c0 + (e1 - e0)/1000.f;
		}
	}
	else
	{
		// ToDo: Remove unused code
		//vector<double> c2e;
		//int num_c2e;
		//// Calculate median c2e for Q1
		//c2e.clear();
		//num_c2e = MAX(q1.size() * 0.2f, 2); // Use at least 2 points
		//num_c2e = MIN(num_c2e, q1.size()); // Limit to the number of available points
		//for (int i = 0; i < num_c2e; i++) {
		//	c2e.push_back(timestampData.at(q1.at(i).second).c2e);
		//}
		//float c2eQ1Med = GetMedian(&(c2e.at(0)), num_c2e);
		//cout << c2eQ1Med << endl;

		//// Calculate median c2e for Q4
		//c2e.clear();
		//num_c2e = MAX(q4.size() * 0.2f, 2); // Use at least 2 points
		//num_c2e = MIN(num_c2e, q4.size());  // Limit to the number of available points
		//for (int i = 0; i < num_c2e; i++) {
		//	c2e.push_back(timestampData.at(q4.at(i).second).c2e);
		//}
		//float c2eQ4Med = GetMedian(&(c2e.at(0)), num_c2e);
		//cout << c2eQ4Med << endl;

		pair<ofApp::TimestampData, ofApp::TimestampData> bestTimestamps;
		bestTimestamps = ofApp::getBestTimestampIndexes(timestampData);

		std::string ts;
		std::time_t c;
		long double m;
		size_t lastDelim;
		size_t lastNChar;

		// Calculate Q1 timesync map
		e0 = bestTimestamps.first.TS_received;
		ts = bestTimestamps.first.TS_sent;
		lastDelim = ts.find_last_of('-'); // find subsecond decimal
		lastNChar = ts.size() - lastDelim - 1;
		c = getEpochTime(std::wstring(ts.begin(), ts.end() - lastNChar - 1)); // Convert to epoch time without subsecond decimal
		m = ((float)atoi(ts.substr(lastDelim + 1, lastNChar).c_str())) / pow(10.f, lastNChar); // Convert subsecond
		c0 = (long double)c + m; // Append subsecond as decimal
		c0 += bestTimestamps.first.roundTrip / 2.f / 1000.f; // adjust computer time by 1/2 the shortest round-trip time

		// Calculate Q4 timesync map
		e1 = bestTimestamps.second.TS_received;
		ts = bestTimestamps.second.TS_sent;
		lastDelim = ts.find_last_of('-'); // find subsecond decimal
		lastNChar = ts.size() - lastDelim - 1;
		c = getEpochTime(std::wstring(ts.begin(), ts.end() - lastNChar - 1)); // Convert to epoch time without subsecond decimal
		m = ((float)atoi(ts.substr(lastDelim + 1, lastNChar).c_str())) / pow(10.f, lastNChar);	c1 = (long double)c + m; // Convert subsecond
		c1 = (long double)c + m; // Append subsecond as decimal
		c1 += bestTimestamps.second.roundTrip / 2.f / 1000.f; // adjust computer time by 1/2 the shortest round-trip time
	}
	// update the calculated anchor points
	tsMap.updateAnchorPoints(EmotiBitPacket::TypeTag::TIMESTAMP_EMOTIBIT, make_pair(e0,e1));
	tsMap.updateAnchorPoints(EmotiBitPacket::TypeTag::TIMESTAMP_LOCAL, make_pair(c0,c1));
	// find the best CrossTime points for time domain transfers
	selectBestCrossTimePoints();

	// calculate anchor points for all other time domains
	if (timestampData.size() >= 1 && parsedDataFormat.additionalTimestamps.size())
	{
		// for all new domains specified by user
		for (auto const& newTimeDomain : parsedDataFormat.additionalTimestamps)
		{
			// only for non TL/TU timestamps
			if(newTimeDomain.compare(EmotiBitPacket::TypeTag::TIMESTAMP_LOCAL) != 0 && newTimeDomain.compare(EmotiBitPacket::TypeTag::TIMESTAMP_UTC) != 0)
			{
				// load the conversion map
				auto conversionMap = timeSyncMap.links[newTimeDomain];
				// traversing through the conversionMap
				for (int i = 0; i < conversionMap.size() - 1; i++)
				{
					std::string fromDomain = conversionMap.at(i);
					std::string toDomain  = conversionMap.at(i + 1);
					// perform calculation only if anchor points do not exist
					if (tsMap.anchorPoints.find(toDomain) != tsMap.anchorPoints.end())
					{
						continue;
					}
					else
					{
						// perform calculation only if we received enough timeCross points for domain transfer
						if (bestCrossDomainPoints[fromDomain].find(toDomain) != bestCrossDomainPoints[fromDomain].end())
						{
							long double ax, ay, bx, by;
							if (fromDomain.compare(EmotiBitPacket::TypeTag::TIMESTAMP_LOCAL) == 0)
							{
								// convert to local time in secs if converting from TL.
								// TL is specified in non-numeral format in raw file
								ax = getLocalTimeSecs(bestCrossDomainPoints[fromDomain][toDomain].first.domainA);
								ay = getLocalTimeSecs(bestCrossDomainPoints[fromDomain][toDomain].second.domainA);
							}
							else
							{
								// get the first and second point in time domain A
								ax = ofToDouble(bestCrossDomainPoints[fromDomain][toDomain].first.domainA);
								ay = ofToDouble(bestCrossDomainPoints[fromDomain][toDomain].second.domainA);
							}
							// get the first and second point in time domain B
							bx = ofToDouble(bestCrossDomainPoints[fromDomain][toDomain].first.domainB);
							by = ofToDouble(bestCrossDomainPoints[fromDomain][toDomain].second.domainB);
							long double k0, k1;
							// find anchor points in second domain, given anchor points in first domain
							k0 = linterp(tsMap.anchorPoints[fromDomain].first, ax, ay, bx, by);
							k1 = linterp(tsMap.anchorPoints[fromDomain].second, ax, ay, bx, by);

							// update the timeSyncMap with calculated anchor points for the new domain
							tsMap.updateAnchorPoints(toDomain, make_pair(k0, k1));
						}
						else
						{
							ofLog(OF_LOG_WARNING, "Too few timestamps received to convert: " + fromDomain + "->" + toDomain);
							// set anchor poitns to 0,0 if domain transfer not possible
							tsMap.updateAnchorPoints(toDomain, make_pair(0, 0));
						}
					}
				}
			}
		}
	}
	return tsMap;
}

void ofApp::selectBestCrossTimePoints()
{
	// for every additional time column in output data
	for (auto timestamp : parsedDataFormat.additionalTimestamps)
	{
		// check if conversion map exists
		if (timeSyncMap.links.find(timestamp) != timeSyncMap.links.end())
		{
			auto link = timeSyncMap.links[timestamp];
			// traversing through the conversion map
			for (int i = 0; link.size() > 1 && i < link.size() - 1; i++)
			{
				// only update if does not exist
				if (bestCrossDomainPoints[link.at(i)].find(link.at(i + 1)) == bestCrossDomainPoints[link.at(i)].end())
				{
					// check if we received more than 2 CROSS_TIMESTAMPS for the required conversion 
					if (allCrossTimePoints[link.at(i)][link.at(i + 1)].size() >= 2)
					{
						XTimeDomainPair first, second;
						first = allCrossTimePoints[link.at(i)][link.at(i + 1)].front();  // first common point between time domains
						second = allCrossTimePoints[link.at(i)][link.at(i + 1)].back();  // last common point between time domains
						bestCrossDomainPoints[link.at(i)][link.at(i + 1)] = make_pair(first, second);
					}
				}
			}
		}
		else
		{
			if (timestamp.compare(EmotiBitPacket::TypeTag::TIMESTAMP_LOCAL) != 0)
			{
				ofLog(OF_LOG_WARNING, "Conversion map not found for: " + timestamp + ". Skipping time converstion.");
			}
		}
	}
}

void ofApp::TimeSyncMap::updateAnchorPoints(std::string identifier, pair<long double, long double> points)
{
	anchorPoints[identifier] = points;
	updateSyncMapHeader(identifier);
}

void ofApp::TimeSyncMap::updateSyncMapHeader(std::string identifier)
{
	std::string suffix0 = "0";
	std::string suffix1 = "1";
	columnHeaders += (identifier + suffix0);
	columnHeaders += EmotiBitPacket::PAYLOAD_DELIMITER;
	columnHeaders += (identifier + suffix1);
	columnHeaders += EmotiBitPacket::PAYLOAD_DELIMITER;
}

long double ofApp::getLocalTimeSecs(std::string timestamp)
{
	std::string ts;
	std::time_t c;
	long double m;
	size_t lastDelim;
	size_t lastNChar;
	long double localUnixTime;

	ts = timestamp;
	lastDelim = ts.find_last_of('-'); // find subsecond decimal
	lastNChar = ts.size() - lastDelim - 1;
	c = getEpochTime(std::wstring(ts.begin(), ts.end() - lastNChar - 1)); // Convert to epoch time without subsecond decimal
	m = ((float)atoi(ts.substr(lastDelim + 1, lastNChar).c_str())) / pow(10.f, lastNChar); // Convert subsecond
	localUnixTime = (long double)c + m; // Append subsecond as decimal
	return localUnixTime;
}

bool ofApp::timestampDataCompare(pair<int, TimestampData> i, pair<int, TimestampData> j) {
	return i.first < j.first;
}

// Converts UTC time string to a time_t value.
std::time_t ofApp::getEpochTime(const std::wstring& dateTime)
{
	// Let's consider we are getting all the input in
	// this format: '2014-07-25T20:17:22Z' (T denotes
	// start of Time part, Z denotes UTC zone).
	// A better approach would be to pass in the format as well.
	static const std::wstring dateTimeFormat{ L"%Y-%m-%d_%H-%M-%S" };

	// Create a stream which we will use to parse the string,
	// which we provide to constructor of stream to fill the buffer.
	std::wistringstream ss{ dateTime };

	// Create a tm object to store the parsed date and time.
	std::tm dt;
	dt.tm_isdst = -1;  // more information: https://en.cppreference.com/w/cpp/chrono/c/tm

	// Now we read from buffer using get_time manipulator
	// and formatting the input appropriately.
	ss >> std::get_time(&dt, dateTimeFormat.c_str());

	// Convert the tm structure to time_t value and return.
	return std::mktime(&dt);
}

double ofApp::GetMedian(double daArray[], int iSize) {
	// Allocate an array of the same size and sort it.
	double* dpSorted = new double[iSize];
	for (int i = 0; i < iSize; ++i) {
		dpSorted[i] = daArray[i];
	}
	for (int i = iSize - 1; i > 0; --i) {
		for (int j = 0; j < i; ++j) {
			if (dpSorted[j] > dpSorted[j + 1]) {
				double dTemp = dpSorted[j];
				dpSorted[j] = dpSorted[j + 1];
				dpSorted[j + 1] = dTemp;
			}
		}
	}

	// Middle or average of middle values in the sorted array.
	double dMedian = 0.0;
	if ((iSize % 2) == 0) {
		dMedian = (dpSorted[iSize / 2] + dpSorted[(iSize / 2) - 1]) / 2.0;
	}
	else {
		dMedian = dpSorted[iSize / 2];
	}
	delete[] dpSorted;
	return dMedian;
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofPushMatrix();
	ofTranslate(0, drawYTranslate);
	ofScale(((float)ofGetWidth()) / 1500.f, ((float)ofGetHeight()) / 900.f * drawYScale);

	for (int i = 0; i < guiPanels.size(); i++) {
		guiPanels.at(i).draw();
	}

	if (currentState == State::PARSING_TIMESTAMPS) {
		legendFont.drawString("PARSING_TIMESTAMPS", 10, 100);
	}
	else if (currentState == State::PARSING_DATA) {
		legendFont.drawString("PARSING_DATA: " + ofToString(lineCounter * 100.f / nLinesInFile, 0) + "% complete", 10, 100);
	}
	else if (currentState == State::WARNING_INSUFFICIENT_TIMESYNCS)
	{
		ofSetColor(255, 128, 0);
		
		legendFont.drawString(timesyncsWarning, 10, 100);
	}
	if (currentState != State::WARNING_INSUFFICIENT_TIMESYNCS)
	{
		legendFont.drawString(dataLine, 10, 200);
	}

	//legendFont.drawString("Frame rate: " + ofToString(ofGetFrameRate()), 10, 300);

	ofPopMatrix();
}

//--------------------------------------------------------------
void  ofApp::closeLoggers() {
	for (auto it = loggers.cbegin(); it != loggers.cend(); ++it) {
		string out = "";
		out +=it->first
			+ " push: " + ofToString(it->second->size(LoggerThread::LoggerQueue::PUSH))
			+ ", pop: " + ofToString(it->second->size(LoggerThread::LoggerQueue::POP));

		out += ", stopThread()";
		it->second->stopThread();

		size_t pushSize = it->second->size(LoggerThread::LoggerQueue::PUSH);
		size_t popSize = it->second->size(LoggerThread::LoggerQueue::POP);
		out += 
			", push: " + ofToString(pushSize)
			+ ", pop: " + ofToString(popSize);

		if (pushSize > 0 || popSize > 0) {
			ofLog(OF_LOG_ERROR, it->first + " buffer writes incomplete. Push Queue: " + ofToString(pushSize) + ", Pop Queue: " + ofToString(popSize) );
		}

		ofLog(OF_LOG_VERBOSE, out);
		delete it->second;
	}

	loggers.clear();
}

//--------------------------------------------------------------
void ofApp::exit() {
	ofLog(OF_LOG_NOTICE, "exit()");
	closeLoggers();
}

//--------------------------------------------------------------
void ofApp::parseDataLine(string packet) {
	static int packetNumber = -1;
	// only parse if not a blank line
	if (packet.compare("") != 0)
	{
		vector<string> splitPacket = ofSplitString(packet, ",");	// split data into separate value pairs

		EmotiBitPacket::Header packetHeader;
		if (!EmotiBitPacket::getHeader(splitPacket, packetHeader)) {
			malformedMessages++;
			cout << "**** MALFORMED PACKET " << malformedMessages << ": " << packetHeader.dataLength << ", " << splitPacket.size() << ": " << packet << " ****" << endl;
			cout << "**** MALFORMED PACKET DATA: " << packet << endl;
			return;
		}

		uint16_t tempPacketNumber = packetHeader.packetNumber;
		if (packetHeader.timestamp < recordedDataTimeRange.emotibitStartTime)
		{
			// first packet. note the first timestamp in the recorded data
			recordedDataTimeRange.emotibitStartTime = packetHeader.timestamp;
		}
		if (packetHeader.timestamp > recordedDataTimeRange.emotibitEndTime)
		{
			// update the recorded end time at every packet parse. 
			// ultimately, emotibitEndTime = last packet timestamp
			recordedDataTimeRange.emotibitEndTime = packetHeader.timestamp;
		}
		// only check for a missed packet if not processing the first packet
		if (packetNumber != -1 && tempPacketNumber - packetNumber > 1) {
			cout << "Missed packet: " << packetNumber << "," << tempPacketNumber << endl;
		}
		// ToDo: Figure out a way to deal with multiple packets of each number (e.g. UDPx3)
		packetNumber = tempPacketNumber;

		if (currentState == State::PARSING_TIMESTAMPS) {
			static int lastRDPacketNumber = -1;

			// ToDo: Handle 2^32 timestamp rollover (~49 days)

			if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::REQUEST_DATA) == 0) {
				for (uint16_t i = EmotiBitPacket::headerLength; i < splitPacket.size(); i++) {
					if (splitPacket.at(i).compare(EmotiBitPacket::TypeTag::TIMESTAMP_LOCAL) == 0) {
						if (allTimestampData.size() > 0 && allTimestampData.back().roundTrip == -1) {
							// If the previous sync round trip wasn't detected, remove it
							allTimestampData.pop_back();
						}
						allTimestampData.emplace_back();
						allTimestampData.back().RD = packetHeader.timestamp;
						//allTimestampData.TS_sent.push_back("");
						//allTimestampData.TS_received.push_back(0);
						//allTimestampData.AK.push_back(0);
						//allTimestampData.roundTrip.push_back(-1);
						//allTimestampData.c2e.push_back(-1);
						lastRDPacketNumber = packetHeader.packetNumber;

						auto loggerPtr = loggers.find(timestampFilenameString);
						if (loggerPtr != loggers.end()) {
							loggerPtr->second->push("\n" + ofToString(allTimestampData.back().RD) + ",");
						}
					}
					// ToDo: handle TIMESTAMP_UTC request
						//|| splitPacket.at(EmotiBitPacket::headerLength).compare(EmotiBitPacket::TypeTag::TIMESTAMP_UTC) == 0)
					// ToDo: handle multiple request RD messages
					//|| (splitPacket.size() > EmotiBitPacket::headerLength + 1 &&
					//(splitPacket.at(EmotiBitPacket::headerLength + 1).compare(EmotiBitPacket::TypeTag::TIMESTAMP_LOCAL) == 0
					//	|| splitPacket.at(EmotiBitPacket::headerLength + 1).compare(EmotiBitPacket::TypeTag::TIMESTAMP_UTC) == 0)
					//	)
				}

				//double now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			}

			if (lastRDPacketNumber > -1) { // Only look for timestamps after REQUEST_DATA
				if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::TIMESTAMP_LOCAL) == 0) {
					// ToDo: Handle TIMESTAMP_UTC
					// update allTimestampData
					allTimestampData.back().TS_received = packetHeader.timestamp;
					if (splitPacket.size() > EmotiBitPacket::headerLength) {
						allTimestampData.back().TS_sent = splitPacket.at(EmotiBitPacket::headerLength);
					}
				}
				if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::ACK) == 0) {
					if (splitPacket.size() > EmotiBitPacket::headerLength) {
						// verify if AK is for the correct RD
						if (lastRDPacketNumber == stoi(splitPacket.at(EmotiBitPacket::headerLength))) {
							allTimestampData.back().AK = packetHeader.timestamp;
							allTimestampData.back().roundTrip = allTimestampData.back().TS_received - allTimestampData.back().RD;
							lastRDPacketNumber = -1; // reset the last RD packet number to avoid overwriting from duplicate
							auto loggerPtr = loggers.find(timestampFilenameString);
							if (loggerPtr != loggers.end()) {
								// write TL information into tiemsync file
								loggerPtr->second->push(ofToString(allTimestampData.back().TS_received) + ",");
								loggerPtr->second->push(allTimestampData.back().TS_sent + ",");
								// write AK information into timesync file
								loggerPtr->second->push(ofToString(allTimestampData.back().AK) + ",");
								loggerPtr->second->push(ofToString(allTimestampData.back().roundTrip) + ",");
							}
						}
						// AK for a previous RD
						else
						{
							// reset the TS_received and TS_sent values
							// If not reset, there could be problems if we encounter the pattern
							// TL (old RD)
							// AK (old RD)
							// missed TL (current RD)
							// AK (current RD)
							allTimestampData.back().TS_received = 0;
							allTimestampData.back().TS_sent = "";
						}
					}
				}
				// ToDo: Add TIMESTAMP_UTC processing
			}
			// gather all cross timestampsin raw data
			if (packetHeader.typeTag.compare(EmotiBitPacket::TypeTag::TIMESTAMP_CROSS_TIME) == 0)
			{
				vector<string> splitData = ofSplitString(packet, ",");
				std::string fromDomain, toDomain;
				if (splitData.size() > 6)
				{
					fromDomain = splitData.at(6);
				}
				else
				{
					ofLogError() << "Cross timestamp \"from\" label not found ";
				}
				if (splitData.size() > 8)
				{
					toDomain = splitData.at(8);
				}
				else
				{
					ofLogError() << "Cross timestamp \"to\" label not found ";
				}
				allCrossTimePoints[fromDomain][toDomain].emplace_back();
				
				if (splitData.size() > 7)
				{
					if (splitData.at(7) != "")
					{
						allCrossTimePoints[fromDomain][toDomain].back().domainA = splitData.at(7);
					}
					else
					{
						ofLogError() << "LSL parse error: local timstamp is NULL";
					}
				}
				else
				{
					ofLogError() << "LSL parse error: local timstamp not found";
				}
				if (splitData.size() > 9)
				{
					if (splitData.at(9) != "")
					{
						allCrossTimePoints[fromDomain][toDomain].back().domainB = splitData.at(9);
					}
					else
					{
						ofLogError() << "LSL parse error: LSL timstamp is NULL";
					}
				}
				else
				{
					ofLogError() << "LSL parse error: LSL timstamp not found";
				}
			}
		}
		else if (currentState == State::PARSING_DATA) {
			// ToDo: Update with EmotiBitPacket::Header usage
			vector<string> splitData = ofSplitString(packet, ",");	// split data into separate value pairs
			if (splitData.size() > 5) {
				uint32_t timestamp;
				uint32_t prevtimestamp;
				static int packetNumber = -1;
				uint16_t dataLength;
				string typeTag;
				uint16_t protocolVersion;
				uint16_t dataReliability;

				if (splitData.at(0) != "") {
					timestamp = ofToInt(splitData.at(0));
				}
				if (splitData.at(1) != "") {
					uint16_t tempPacketNumber = ofToInt(splitData.at(1));
					if (packetNumber != -1 && tempPacketNumber - packetNumber > 1) {
						cout << "Missed packet: " << packetNumber << "," << tempPacketNumber << endl;
					}
					// ToDo: Figure out a way to deal with multiple packets of each number
					packetNumber = tempPacketNumber;
				}
				if (splitData.at(2) != "") {
					dataLength = ofToInt(splitData.at(2));
				}
				if (splitData.at(3) != "") {
					typeTag = splitData.at(3);
				}
				if (splitData.at(4) != "") {
					protocolVersion = ofToInt(splitData.at(4));
				}
				if (splitData.at(5) != "") {
					dataReliability = ofToInt(splitData.at(5));
				}
				//string outFilePath = inFilePath + "_" + typeTag + ".csv";
				//string outFilePath = inFilePath + "_";

				//outFile.open(outFilePath.c_str(), ios::out | ios::app);
				//if (outFile.is_open()) {
				bool isCompositePayload = false;
				for (int i = 0; i < EmotiBitPacket::TypeTagGroups::NUM_COMPOSITE_PAYLOAD; i++)
				{
					if (typeTag.compare(EmotiBitPacket::TypeTagGroups::COMPOSITE_PAYLOAD[i]) == 0)
					{
						isCompositePayload = true;
					}
				}

				std::string fileTypeModifier;
				// If packet is of type TX, create a file with name _TL_AA_BB.csv
				if (typeTag.compare(EmotiBitPacket::TypeTag::TIMESTAMP_CROSS_TIME) == 0)
				{
					std::string modifier;
					// traverse through payload to extract typetags/ payload labels
					for (int i = 0; i < dataLength; i++)
					{
						if (i + 6 >= splitData.size())
						{
							cout << "Error: dataLength > size, " << packet << endl;
						}
						else
						{
							if ((i + 6) % 2 == 0) // is even
							{
								modifier += ParsedDataFormat::FILE_EXT_DELIMITER;
								modifier += splitData.at(i + 6);
							}
						}
					}
					fileTypeModifier = typeTag + modifier;
				}
				else
				{
					fileTypeModifier = typeTag;
				}

				auto indexPtr = timestamps.find(fileTypeModifier);
				if (indexPtr != timestamps.end()) {	// we have a previous timestamp!
					prevtimestamp = indexPtr->second;
					indexPtr->second = timestamp;
				}
				else {
					timestamps.emplace(fileTypeModifier, timestamp);
					prevtimestamp = timestamp;
				}

				auto loggerPtr = loggers.find(fileTypeModifier);
				if (loggerPtr == loggers.end()) {	// we don't have a logger already
					string outFilePath = inFileDir + inFileBase + "_" + fileTypeModifier + fileExt;
					cout << "Creating file: " << outFilePath << endl;
					LoggerThread* newLogger = new LoggerThread("", outFilePath);
					newLogger->setPushThrottlingSize(1000); // Set logger.push() to throttle after falling behind more than 1K lines
					loggers.emplace(fileTypeModifier, newLogger);
					loggerPtr = loggers.find(fileTypeModifier);
					loggerPtr->second->startThread();
					std::string headerString = "";
					if (!isCompositePayload)
					{
						headerString = parsedDataFormat.getParsedFileColHeaders() + typeTag + "\n";
					}
					else
					{
						headerString += parsedDataFormat.getParsedFileColHeaders();
						for (int i = 0; i < dataLength; i++)
						{
							if (i + 6 >= splitData.size())
							{
								cout << "Error: dataLength > size, " << packet << endl;
							}
							else
							{
								if ((i + 6) % 2 == 0) // is even
								{
									headerString += splitData.at(i + 6);
									headerString += EmotiBitPacket::PAYLOAD_DELIMITER;
								}
							}
						}
						headerString += "\n";
					}
					loggerPtr->second->push(headerString);
				}
				bool isAperiodicType = false;
				for (int i = 0; i < EmotiBitPacket::TypeTagGroups::NUM_APERIODIC; i++)
				{
					if (typeTag.compare(EmotiBitPacket::TypeTagGroups::APERIODIC[i]) == 0)
					{
						isAperiodicType = true;
					}
				}
				
				if (isAperiodicType) { //for aperiodic
					std::string parsedDataRow = "";
					if (splitData.size() < 6) {
						cout << "Error: dataLength < size, " << packet << endl;
					}
					else {
						// add timestamp for each new domain
						for (auto newTimeDomain : parsedDataFormat.additionalTimestamps)
						{
							long double newDomainTimestamp;
							// use calculated anchor poitns to find timestamps
							newDomainTimestamp = linterp(timestamp,
								timeSyncMap.anchorPoints[EmotiBitPacket::TypeTag::TIMESTAMP_EMOTIBIT].first,
								timeSyncMap.anchorPoints[EmotiBitPacket::TypeTag::TIMESTAMP_EMOTIBIT].second,
								timeSyncMap.anchorPoints[newTimeDomain].first,
								timeSyncMap.anchorPoints[newTimeDomain].second);
							parsedDataRow = ofToString(newDomainTimestamp, 6) + "," + parsedDataRow;

						}
						// ToDo: Add provision for LM timestamp
						loggerPtr->second->push(
							parsedDataRow +
							ofToString(timestamp, 3) + "," +
							splitData.at(1) + "," +
							splitData.at(2) + "," +
							splitData.at(3) + "," +
							splitData.at(4) + "," +
							splitData.at(5) + "," +
							splitData.at(6) +
							'\n'
						);
					}
				}
				else if (isCompositePayload)
				{
					std::string parsedDataRow = "";
					for (auto newTimeDomain : parsedDataFormat.additionalTimestamps)
					{
						long double newDomainTimestamp;
						newDomainTimestamp = linterp(timestamp,
							timeSyncMap.anchorPoints[EmotiBitPacket::TypeTag::TIMESTAMP_EMOTIBIT].first,
							timeSyncMap.anchorPoints[EmotiBitPacket::TypeTag::TIMESTAMP_EMOTIBIT].second,
							timeSyncMap.anchorPoints[newTimeDomain].first,
							timeSyncMap.anchorPoints[newTimeDomain].second);
						parsedDataRow = ofToString(newDomainTimestamp, 6) + "," + parsedDataRow;

					}
					// ToDo: Add provision for LM timestamp
					parsedDataRow += ofToString(timestamp, 3) + "," +
						splitData.at(1) + "," +
						splitData.at(2) + "," +
						splitData.at(3) + "," +
						splitData.at(4) + "," +
						splitData.at(5) + ",";
					for (int i = 0; i < dataLength; i++) 
					{
						if (i + 6 >= splitData.size()) 
						{
							cout << "Error: dataLength > size, " << packet << endl;
						}
						else 
						{
							if ((i + 6) % 2 == 1)
							{
								parsedDataRow += splitData.at(i + 6);
								parsedDataRow += EmotiBitPacket::PAYLOAD_DELIMITER;
							}
						}
					}
					loggerPtr->second->push(parsedDataRow + '\n');
				}
				else if (typeTag == EmotiBitPacket::TypeTagGroups::USER_MESSAGES[0]) { // for push messages
					if (splitData.size() != 8) {
						cout << "Error: userNote package error " << packet << endl;
					}
					else {
						std::string parsedDataRow = "";
						std::time_t c;
						std::string computerTime = splitData.at(6);
						size_t lastDelim = computerTime.find_last_of('-'); // find subsecond decimal
						size_t lastNChar = computerTime.size() - lastDelim - 1;
						c = getEpochTime(std::wstring(computerTime.begin(), computerTime.end() - lastNChar - 1)); // Convert to epoch time without subsecond decimal
						for (auto newTimeDomain : parsedDataFormat.additionalTimestamps)
						{
							if (newTimeDomain.compare(EmotiBitPacket::TypeTag::TIMESTAMP_LOCAL) == 0)
							{
								parsedDataRow += ofToString((long double)c, 6) + ",";
							}
							else
							{
								long double newDomainTimestamp;
								newDomainTimestamp = linterp((long double)c,
									timeSyncMap.anchorPoints[EmotiBitPacket::TypeTag::TIMESTAMP_LOCAL].first,
									timeSyncMap.anchorPoints[EmotiBitPacket::TypeTag::TIMESTAMP_LOCAL].second,
									timeSyncMap.anchorPoints[newTimeDomain].first,
									timeSyncMap.anchorPoints[newTimeDomain].second);
								parsedDataRow = ofToString(newDomainTimestamp, 6) + "," + parsedDataRow;
							}
						}
						loggerPtr->second->push(
							parsedDataRow + ofToString(timestamp, 3) + "," +
							splitData.at(1) + "," +
							splitData.at(2) + "," +
							splitData.at(3) + "," +
							splitData.at(4) + "," +
							splitData.at(5) + "," +
							splitData.at(7) +
							'\n'
						);
					}

				}
				else { // if typetag is periodic
					for (int i = 0; i < dataLength; i++) {
						if (i + 6 >= splitData.size()) {
							cout << "Error: dataLength > size, " << packet << endl;
						}
						else {
							std::string parsedDataRow = "";
							long double interpTimestamp = linterp(i + 1, 0, dataLength, prevtimestamp, timestamp);
							for (auto newTimeDomain : parsedDataFormat.additionalTimestamps)
							{
								long double newDomainTimestamp;
								newDomainTimestamp = linterp(interpTimestamp,
									timeSyncMap.anchorPoints[EmotiBitPacket::TypeTag::TIMESTAMP_EMOTIBIT].first,
									timeSyncMap.anchorPoints[EmotiBitPacket::TypeTag::TIMESTAMP_EMOTIBIT].second,
									timeSyncMap.anchorPoints[newTimeDomain].first,
									timeSyncMap.anchorPoints[newTimeDomain].second);
								parsedDataRow = ofToString(newDomainTimestamp, 6) + "," + parsedDataRow;
							}
							loggerPtr->second->push(
								parsedDataRow + ofToString(interpTimestamp, 3) + "," +
								splitData.at(1) + "," +
								splitData.at(2) + "," +
								splitData.at(3) + "," +
								splitData.at(4) + "," +
								splitData.at(5) + "," +
								splitData.at(i + 6) +
								'\n'
							);
							//outFile << interpTimestamp << "," << packetNumber << "," << dataLength << "," << typeTag << "," << protocolVersion << "," << dataReliability << "," << splitData.at(i + 6) << endl;
						}
					}
				}

				//outFile.close();
			//}
			}
		}
	}
}

long double ofApp::linterp(long double x, long double x0, long double x1, long double y0, long double y1) {
	return y0 + (x - x0) * ((y1 - y0) / (x1 - x0));
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	// Increment the timeWindow
	if (key == OF_KEY_RIGHT) { // Right Arrow
	}
	if (key == OF_KEY_UP) {
		drawYTranslate--;
		drawYScale = (drawYScale * 900.f + 1.f) / 900.f;
	}
	if (key == OF_KEY_DOWN) {
		drawYTranslate++;
		drawYScale = (drawYScale * 900.f - 1.f) / 900.f;
	}
}


//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
	cout << "Key Released: " << key << "\n";

	if (key == 'r') { // Space Bar
	}
}


//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
