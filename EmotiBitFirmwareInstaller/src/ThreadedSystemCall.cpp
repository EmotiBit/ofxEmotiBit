#include "ThreadedSystemCall.h"
#include <stdio.h>
void ThreadedSystemCall::setup(std::string cmd, std::string targetResponse)
{
	this->cmd = cmd;
	this->targetResponse = targetResponse;
	cmdResult = false;
	systemOutput = "";
}

void ThreadedSystemCall::threadedFunction()
{
	if (isThreadRunning())
	{
		// system call
		char buffer[200];
		bool status = false;
#if defined (TARGET_OSX) || defined (TARGET_LINUX)
		FILE* pipe = popen(cmd.c_str(), "r");
#else
		FILE* pipe = _popen(cmd.c_str(), "r");
#endif
		if (!pipe)
		{
			throw std::runtime_error("popen() failed!");
			stopThread();
		}
		try 
		{
			while (fgets(buffer, sizeof buffer, pipe) != NULL) 
			{
				lock();
				std::string tempStr = "";
				tempStr += buffer;
				if (targetResponse != "")
				{
					// check if the target string is a part of the output of the system command
					if (tempStr.find(targetResponse) != std::string::npos)
					{
						// found response which indicates successfull system call
						cmdResult = true;
					}
				}
				systemOutput += tempStr;
				unlock();
			}
			if (targetResponse == "")
			{
				lock();
				cmdResult = true;
				unlock();
			}
		}
		catch (...) {
#if defined (TARGET_OSX) || defined (TARGET_LINUX)
			pclose(pipe);
#else
			_pclose(pipe);
#endif
			stopThread();
			throw;
		}
#if defined (TARGET_OSX) || defined (TARGET_LINUX)
		pclose(pipe);
#else
		_pclose(pipe);
#endif

		stopThread();
	}
}
