#pragma once

#include "ofMain.h"

namespace EmotiBit
{
	//! Returns a timestamp string in the specified format. Replaces OF
	//! ofUtils function to add support for %f that crashes on Windows
	//! @param timestampFormat see https://cplusplus.com/reference/ctime/strftime/
	//!		and for milliseconds %i=int, %f=float
	//! @return formated timestamp string
	string ofGetTimestampString(const string& timestampFormat);
}