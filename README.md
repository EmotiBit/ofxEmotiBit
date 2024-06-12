# ofxEmotiBit
Software code for the EmotiBit. 
If you just want to visualize and parse data from the EmotiBit (for Windows/ macOS), you can simply download the software directly in [releases](https://github.com/EmotiBit/ofxEmotiBit/releases).

If you want to modify the code(or build the tools in Linux), below are the requirements to build the project.  

## Requirements
### Openframeworks
Install openFrameworks 0.11.2 (vs2017_release) from the official [openFrameworks GitHub repository](https://github.com/openframeworks/openFrameworks/releases/tag/0.11.2).

#### Openframeworks addons
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
- The project is built on a 64-bit architecture. Ensure you are on a machine supporting the `x64` build platform.
- Required to build EmotiBit FirmwareInstaller
  - **ofxSerial:** [GitHub repository](https://github.com/EmotiBit/ofxSerial)
  - **ofxIO:** [GitHub repository](https://github.com/bakercp/ofxIO)
- **If downloading the zip instead of `git clone` be sure to remove `-master`  or `-xxx-xxx` from the folder name to maintain correct path references**. 

#### Addons directory structure
The `OF_ROOT/addons` folder should look like shown below. You can have additional addons in the addons folder, but the addons linked above are **required** for building EmotiBit software.
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
git clone git@github.com:bakercp/ofxSerial.git
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

### Note on using M1 macs (apple silicon)
- Building from source on Apple Silicon macs is not yet supported. 
- We are working on compiling a list of changes required to build on Apple silicon macs and will make that patch as soon as possible.

### Setting up xcode project
- **Adding paths to Library search paths**
  - Check if the directory paths for the files `liblsl64-static.a` and `liblslboost.a` are already present in the `project` > `Build Settings` > `Library Search Paths`. If they are not present, follow the below steps:  
    - Select your project in the **Target group**(in xcode project navigator), go to **Build Settings** tab, and add the following path in the **Library Search Paths** section: `../../../addons/ofxLSL/libs/labstreaminglayer/lib/osx`
- For `EmotiBitDataParser`, if you get an error `ERROR: -NSDocumentRevisionsDebugMode does not exist, try absolute path` when compiling in `debug mode`,
  - Choose the `build scheme` on the top left
  - In the `Run` tab, open the `Options` tab
  - unckeck the `Allow debugging when using document Version Browser` checkbox
  - Try building again.

</details>

## Setup Instructions for Linux

<details>
<summary>Instructions for make</summary>

- You will require a version of gcc on your linux machine. Depending on the version, we need to install the appropriate OpenFrameworks code base. You can check the gcc verison on you system using the following command: `gcc --verison`.
- If you do not have gcc installed, you can install it using `sudo apt install gcc`.
- For EmotiBit software development, we have used gcc v6+, which has been tested to run with the OpenFrameworks
- Download and extract the Openframeworks package for the gcc version on your system. [Openframeworks Downloads](https://openframeworks.cc/versions/v0.11.0/)
  - You can use the following command to extract `tar xvzf <filename>`
- Follow the official [openframeworks guide](https://openframeworks.cc/setup/linux-install/) to set things up. Follow the instruction mentioned below during running `install_dependencies.sh`
  - At one point during installation of the dependencies(after you run the shell script `install_dependencies.sh`), a prompt will ask the user to press Y/N to install `Prompt: “installing OF dependencies with -hwe-18.04 packages, confirm Y/N ?`. Press N. [Link to article](https://forum.openframeworks.cc/t/urgent-installing-libgl1-mesa-dev-hwe-18-04/32345/3)
- At this point, you should have 
  - all the dependencies installed(successfull run of `install_dependencies.sh`)
  - compiled OF (successfull run of `./compileOF.sh`)
  - project Generator set up(successfull run of `./compilePG.sh`)
- Now, we need all the repositories required to build EmotiBit_Oscilloscope. Install all the addons mentioned in the `Requirements` section.
  - You can either download the repositories, or use `git clone <repo name>` to get the  addons. If git is not installed on your system, use `sudo apt install git` to install git.
- You will also require net-tools to run certain commands required by the Oscilloscope. Run `sudo apt install net-tools`
- That's it! You now are ready to run EmotiBit Oscilloscope!
- To run the Oscilloscope, cd to `(OF_ROOT)/addons/ofxEmotiBit/EmotiBitOscilloscope`. Run the command `make Debug` or `make` to create the release executable.
- Note: When trying to run the EmotiBit Oscilloscope, if you get an error with the following message `cannot open shared object file: No such file or directory : liblsl-1.14.0-manylinux2010_x64`, make sure you have the latest master for [ofxLSL](https://github.com/EmotiBit/ofxLSL). The [fix](https://github.com/EmotiBit/ofxLSL/pull/8/files) added the required [shared object file](https://github.com/EmotiBit/ofxLSL/tree/master/libs/labstreaminglayer/lib/linux64).

</details>
