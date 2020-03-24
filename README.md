# ofxEmotiBit
Software code for the EmotiBit. 
If you just want to visualize and parse data from the EmotiBit, you can simply download the software directly in [releases](https://github.com/EmotiBit/ofxEmotiBit/releases).

If you want to modify the code, below are the requirements to build the project.  

## Requirements
- OF 11 - https://openframeworks.cc/download/
- ofxNetworkUtils - https://github.com/bakercp/ofxNetworkUtils
- ofxOscilloscope - https://github.com/produceconsumerobot/ofxOscilloscope
- ofxThreadedLogger - https://github.com/produceconsumerobot/ofxThreadedLogger
- ofxBiquadFilter - https://github.com/mrbichel/ofxBiquadFilter
- EmotiBit_XPlat_Utils - https://github.com/EmotiBit/EmotiBit_XPlat_Utils
- ofxLSL - https://github.com/badfishblues/ofxLSL
  - _**NOTE:**_ for lsl support, if developing with visual studio, code should be compiled for x64
  - liblsl64.dll should always be in the same folder as the .exe (i.e. EmotiBitOscilloscope/bin/liblsl64.dll)
  - liblsl64.lib should always be linked to in under _solution properties->linker->general->additional library directories_ and _solution properties->linker->input-> additional dependencies_
  - both of these libs are handled properly by default, but should be considered if deviating from release code
- The project is built on a 64-bit architecture. Make sure you are on a machine that support `x64` build platform.
## macOS Issues resolution
- **Adding paths to Library search paths**
  - Check if the directory paths for the files `liblsl64-static.a` and `liblslboost.a` are already present in the `project` > `Build Settings` > `Library Search Paths`. If they are not present, follow the below steps:  
  - Select your project in the **Target group**(in xcode project navigator), go to **Build Settings** tab, and add the following path in the **Library Search Paths** section: `../../../addons/ofxLSL/libs/labstreaminglayer/lib/osx`
  - Go to the **Build Phases** tab, expand the **Link Binary With Libraries** section, click the "+" button.
    - In the appeared window, click the **Add Other...** button and specify the path to the `liblsl64-static.a` and `liblslboost.a`
  - reference: https://knowledgebase.ocrsdk.com/article/728
