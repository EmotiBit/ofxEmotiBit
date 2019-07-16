# ofxEmotiBit
Software for the biosensor module. Precompiled executables can be found in releases and run without below requirements.

## Requirements
- OF 0.9.8 - https://openframeworks.cc/download/older/
- ofxInputField - Necessary for OF0.9.8 https://github.com/fx-lange/ofxInputField
- ofxNetworkUtils:stable - git@github.com:bakercp/ofxNetworkUtils.git
- ofxOscilloscope - git@github.com:produceconsumerobot/ofxOscilloscope.git
- ofxThreadedLogger - git@github.com:produceconsumerobot/ofxThreadedLogger.git
- ofxLSL - git@github.com:badfishblues/ofxLSL.git
  - _**NOTE:**_ for lsl support, if developing with visual studio, code should be compiled for x64
  - liblsl64.dll should always be in the same folder as the .exe (i.e. EmotiBitOscilloscope/bin/liblsl64.dll)
  - liblsl64.lib should always be linked to in under _solution properties->linker->general->additional library directories_ and _solution properties->linker->input-> additional dependencies_
  - both of these libs are handled properly by default, but should be considered if deviating from release code
