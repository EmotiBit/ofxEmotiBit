#pragma once
#include "ofmain.h"

namespace EmotiBit
{
	//! Returns a timestamp string in the specified format. Replaces OF
	//! ofUtils function to add support for %f that crashes on Windows
	//! @param timestampFormat see https://cplusplus.com/reference/ctime/strftime/
	//!		and for milliseconds %i=int, %f=float
	//! @return formated timestamp string
	static string ofGetTimestampString(const string& timestampFormat)
	{
		std::stringstream str;
		auto now = std::chrono::system_clock::now();
		auto t = std::chrono::system_clock::to_time_t(now);    std::chrono::duration<double> s = now - std::chrono::system_clock::from_time_t(t);
		int us = s.count() * 1000000;
		auto tm = *std::localtime(&t);
		constexpr int bufsize = 256;
		char buf[bufsize];

		// Beware! an invalid timestamp string crashes windows apps.
		// so we have to filter out %i AND %f (which is not supported by vs)
		// earlier.
		auto tmpTimestampFormat = timestampFormat;
		ofStringReplace(tmpTimestampFormat, "%i", ofToString(us / 1000, 3, '0'));
		ofStringReplace(tmpTimestampFormat, "%f", ofToString(us, 6, '0'));

		if (strftime(buf, bufsize, tmpTimestampFormat.c_str(), &tm) != 0) {
			str << buf;
		}
		auto ret = str.str();


		return ret;
	}
};