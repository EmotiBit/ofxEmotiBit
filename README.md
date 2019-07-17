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

## macOS Issues resolution
- **To run OF0.9.8 in xcode 10+**
  - (resolves a linker error when building)remove _“-framework QuickTime -framework QTKit”_ from `of_v0.9.8_osx_release/libs/openFrameworksCompiled/project/osx/CoreOF.xcconfig`
  - Edit build settings in both .xcodeproj files(in the project navigator in xcode) (project file and openFrameworksLib file).Set both architectures to standard 64 bit.
  - reference: https://forum.openframeworks.cc/t/xcode-10-0-build-errors/30447/6
- **Adding paths to Library search paths**
  - Required to add ofxLSL linker files to the project. The path for the files are `addons/ofxLSL/libs/labstreaminglayer/lib/osx/liblsl64-static.a` and `addons/ofxLSL/libs/labstreaminglayer/lib/osx/liblslboost.a`
  - Select your project in the **Target group**(in xcode project navigator), go to **Build Settings** tab, and specify the following fields in the **Library Search Paths** section: `../../../addons/ofxLSL/libs/labstreaminglayer/lib/osx`
  - Go to the **Build Phases** tab, expand the **Link Binary With Libraries** section, click the "+" button.
    - In the appeared window, click the **Add Other...** button and specify the path to the `liblsl64-static.a` and `liblslboost.a`
  - reference: https://knowledgebase.ocrsdk.com/article/728
