# ofxEmotiBit
Software code for the EmotiBit. 
If you just want to visualize and parse data from the EmotiBit (for Windows/ macOS), you can simply download the software directly in [releases](https://github.com/EmotiBit/ofxEmotiBit/releases).

If you want to modify the code(or build the tools in Linux), below are the requirements to build the project.  

## Requirements
### Openframeworks
Install openFrameworks 0.11.2 from the official [openFrameworks GitHub repository](https://github.com/openframeworks/openFrameworks/releases/tag/0.11.2). Choose the appriopriate version for your operating system.
  - For windows:  `of_v0.11.2_vs2017_release.zip`
  - For macOS: `of_v0.11.2_osx_release.zip`
  - For linux: `of_v0.11.2_linux64gcc6_release.tar.gz`

### Openframeworks addons
#### Addons directory structure
Openframeworks uses `addons` to support adding features to the projects. The addons used by EmotiBit software are listed in the section below.
The addons are placed in the `OF_ROOT/addons` folder and it's structure is shown below. Please download or clone (uses git) the addons listed in the section below in the `OF_ROOT/addons` directory. You can have additional addons in the addons folder, but the addons listed below are **required** for building EmotiBit software.
- <details><summary>Sample addons folder</summary>

  ```plaintext
  addons
  ├── ofxEmotiBit
  │   ├── src
  │   ├── EmotiBitOscilloscope
  │   │   ├── EmotiBitOscilloscope.sln
  │   │   ├── EmotiBitOscilloscope.xcodeproj
  │   │   └── ...
  │   ├── EmotiBitDataParser
  │   └── EmotiBitFirmwareInstaller        
  ├── ofxNetworkUtils
  ├── ofxOscilloscope
  ├── ofxThreadedLogger
  ├── ofxBiquadFilter
  ├── ofxJSON
  ├── EmotiBit_XPlat_Utils
  ├── ofxLSL
  ├── ofxSerial
  └── ofxIO
  ```
  </details>


#### Download the following Openframeworks addons 
- **ofxNetworkUtils:** [GitHub repository](https://github.com/bakercp/ofxNetworkUtils)
- **ofxOscilloscope:** [GitHub repository](https://github.com/produceconsumerobot/ofxOscilloscope/)
- **ofxThreadedLogger:** [GitHub repository](https://github.com/produceconsumerobot/ofxThreadedLogger)
- **ofxBiquadFilter:** [GitHub repository](https://github.com/mrbichel/ofxBiquadFilter)
- **ofxJSON:** [GitHub repository](https://github.com/jeffcrouse/ofxJSON)
- **EmotiBit_XPlat_Utils:** [GitHub repository](https://github.com/EmotiBit/EmotiBit_XPlat_Utils/)
- **ofxLSL:** [GitHub repository](https://github.com/EmotiBit/ofxLSL/)
  - <details><summary>Notes for developing with Visual Studio</summary>
    
    - _**Note:**_ for LSL support, if developing with Visual Studio, code should be compiled for x64
    - liblsl64.dll should always be in the same folder as the .exe (i.e. EmotiBitOscilloscope/bin/liblsl64.dll)
    - liblsl64.lib should always be linked to in under _solution properties->linker->general->additional library directories_ and _solution properties->linker->input-> additional dependencies_
    - both of these libs are handled properly by default but should be considered if deviating from the release code
    </details>
- Required to build EmotiBit FirmwareInstaller
  - **ofxSerial:** [EmotiBit ofxSerial](https://github.com/EmotiBit/ofxSerial)
  - **ofxIO:** [bakercp ofxIO](https://github.com/bakercp/ofxIO)
- Additional notes
  - The project is built on a 64-bit architecture. Ensure you are on a machine supporting the `x64` build platform.
  - **If downloading the zip instead of `git clone`, be sure to remove `-master`  or `-xxx-xxx` from the folder name to maintain correct path references**. 


#### The following script may be run from a bash shell within your openFrameworks/addons/ directory to install ofxEmotiBit and all dependencies. 
_**Note:**_ this requires you to have [github SSH key access set up](https://docs.github.com/en/authentication/connecting-to-github-with-ssh/generating-a-new-ssh-key-and-adding-it-to-the-ssh-agent).
```
eval "$(ssh-agent -s)"
ssh-add ~/.ssh/id_ed25519
git clone git@github.com:EmotiBit/ofxEmotiBit.git
git clone git@github.com:bakercp/ofxNetworkUtils.git
cd ofxNetworkUtils
git checkout stable
cd ..
git clone git@github.com:produceconsumerobot/ofxOscilloscope.git
git clone git@github.com:produceconsumerobot/ofxThreadedLogger.git
git clone git@github.com:smukkejohan/ofxBiquadFilter.git
git clone git@github.com:jeffcrouse/ofxJSON.git
git clone git@github.com:EmotiBit/EmotiBit_XPlat_Utils.git
git clone git@github.com:EmotiBit/ofxLSL.git
git clone git@github.com:EmotiBit/ofxSerial.git
cd ofxSerial
git checkout stable
cd ..
git clone git@github.com:bakercp/ofxIO.git
cd ofxIO
git checkout stable
cd ..
```


## Setup Instructions for Windows 11

<details>
<summary>Instructions for Visual Studio</summary>

1. **Install Visual Studio 2022**: Download and install from [Microsoft Visual Studio](https://visualstudio.microsoft.com/vs/). If previously installed, navigate to the "Tools" tab and select "Get tools and features".
2. During setup, select the "Desktop Development with C++" workload. Ensure the following components are installed:
   - MSVC v143 - VS 2022 C++ x64/x86 build tools
   - C++ ATL for latest 143 build tools
   - Security Issue Analysis
   - C++ Build Insights
   - Just-In-Time debugger
   - C++ profiling tools
   - C++ CMake tools for Windows
   - Test Adapter for Boost.Test
   - Test Adapter for Google Test
   - Live Share
   - C++ AddressSanitizer
   - Windows 11 SDK
   - vcpkg manager
   - GitHub Copilot (optional)
3. Additionally, navigate to the “Individual components” tab and install the "MSVC v141 - VS 2017 C++ x64/x86 build tools (v14.16)".
4. **Restart** your computer once everything is installed.

### Building the Project

1. Open the `EmotiBitOscilloscope` Visual Studio solution file (.sln) located in `of_v0.11.2_vs2017_release\addons\ofxEmotiBit\EmotiBitOscilloscope`.
2. If prompted to install extra components in the solution explorer menu, click on the install button to proceed.
3. Build and run the solution file in the debug profile once all components have been installed. If the build fails, retarget the solution to the latest version (10.0 Windows SDK version, v143 build tools).

### Notes

- The default components in the VS 2022 setup differ from those in the setup guide on the official [openFrameworks website](https://openframeworks.cc/setup/vs/), which uses VS 2019. Notably, the following components are specific to VS 2022 and must be included in our setup:
   - MSVC v143 - VS 2022 C++ x64/x86 build tools
   - Windows 11 SDK
   - vcpkg package manager
   - GitHub Copilot (optional)
   - MSVC v141 - VS 2017 C++ x64/x86 build tools (v14.16), not in the default components list but necessary for our build.
</details>


## Setup Instructions for macOS 

<details>
<summary>Instructions for xcode</summary>

- The EmotiBit software can be compiled on macOS using the provided xcode project files. Each EmotiBit software has it's own project file, already setup with all settings required to build the project from source. Users just have to open the project files and build using xcode.
- EmotiBit Oscilloscope 
  - The Oscilloscope uses external dependencies that are platform (x86 or arm64) specific. The EmotiBitOscilloscope project therefore provides 2 targets, 1 for x86 and another for arm.
  - If you are building on a x86 machine, for example on intel macs, please select the target as `EmotiBitOscilloscope-x86_64` under the build schemes.
  - If you are building on an arm machine, for example on apple-silicon macs, please select the target as `EmotiBitOscilloscope-arm64` under the build schemes.
  - To edit schemes, click on the `build scheme` > `Edit` > choose the correct target to build.
- <details><summary><b>Known EmotiBitDataParser build error fix</b></summary>
  
  - For `EmotiBitDataParser`, if you get an error `ERROR: -NSDocumentRevisionsDebugMode does not exist, try absolute path` when compiling in `debug
    - Choose the `build scheme`.
    - In the `Run` tab, open the `Options` tab
    - unckeck the `Allow debugging when using document Version Browser` checkbox.
    - Try building again.
  </details>

</details>

## Setup Instructions for Linux

<details>
<summary>Instructions for make</summary>

- You will require a version of gcc on your linux machine. Depending on the version, we need to install the appropriate OpenFrameworks code base. You can check the gcc verison on you system using the following command: `gcc --verison`.
- If you do not have gcc installed, you can install it using `sudo apt install gcc`.
- For EmotiBit software development, we have used gcc v6+, which has been tested to run with the OpenFrameworks
- Download and extract the Openframeworks package for the gcc version on your system. [Openframeworks Downloads](https://openframeworks.cc/versions/v0.11.2/)
  - You can use the following command to extract `tar xvzf <filename>`
- Follow the official [openframeworks guide](https://openframeworks.cc/setup/linux-install/) to set things up.
- At this point, you should have 
  - all the dependencies installed(successfull run of `install_dependencies.sh`)
  - compiled OF (successfull run of `./compileOF.sh`)
  - project Generator set up(successfull run of `./compilePG.sh`)
- If you have not already, get all the addons listed above.
- install additional dependencies required for EmotiBit software (see [#248](https://github.com/EmotiBit/ofxEmotiBit/issues/248) for more information)
  - `sudo apt-get install -y -qq libxrandr-dev libxinerama-dev libxcursor-dev cmake`
  - `sudo apt install libxi-dev`
- You will also require net-tools to run certain commands required by the Oscilloscope. Run `sudo apt install net-tools`. See (https://github.com/EmotiBit/ofxEmotiBit/issues/249)
- That's it! You now are ready to run EmotiBit Oscilloscope!
- To run the Oscilloscope, cd to `(OF_ROOT)/addons/ofxEmotiBit/EmotiBitOscilloscope`. Run the command `make Debug` or `make` to create the release executable.
- Note: When trying to run the EmotiBit Oscilloscope, if you get an error with the following message `cannot open shared object file: No such file or directory : liblsl-1.14.0-manylinux2010_x64`, make sure you have the latest master for [ofxLSL](https://github.com/EmotiBit/ofxLSL). The [fix](https://github.com/EmotiBit/ofxLSL/pull/8/files) added the required [shared object file](https://github.com/EmotiBit/ofxLSL/tree/master/libs/labstreaminglayer/lib/linux64).

</details>
