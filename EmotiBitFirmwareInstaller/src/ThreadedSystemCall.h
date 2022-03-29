#pragma once
#include "ofMain.h"

/*!
	@brief Class to execute system call on a separate thread. This keeps the main thread for GUI updates.
*/
class ThreadedSystemCall :public ofThread{
public:
	std::string cmd;  //!< The system command to run
	std::string targetResponse; //!< The response string we are detecting in the system output
	bool cmdResult; //!< true is command was successully executed, else false
	std::string systemOutput;  //!< The system output
	std::string PIPE_OPEN_FAILED = "Failed to open pipe";
	/*!
		@brief Function to set the system command and response string
		@param cmd The system command to run
		@param targetResponse The string we are looking for in the command output
	*/
	void setup(std::string cmd, std::string targetResponse = "");
	
	void threadedFunction();
};