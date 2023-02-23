# ofxEmotiBit
Software code for the EmotiBit. 
If you just want to visualize and parse data from the EmotiBit (for Windows/ macOS), you can simply download the software directly in [releases](https://github.com/EmotiBit/ofxEmotiBit/releases).

If you want to modify the code(or build the tools in Linux), below are the requirements to build the project.  

## Requirements
- OF 11 ~~https://openframeworks.cc/download/~~ NOTE: ofxEmotiBit is not yet compatible with 0.11.2. OF v0.11.0 can be downloaded here https://openframeworks.cc/versions/v0.11.0/
### The following addons should be placed inside the OpenFrameworks addons folder:
#### Note: If downloading zip instead of `git clone` be sure to remove `-master` from the folder name to avoid path discrepancies
- ofxNetworkUtils - https://github.com/bakercp/ofxNetworkUtils
- ofxOscilloscope - https://github.com/produceconsumerobot/ofxOscilloscope
- ofxThreadedLogger - https://github.com/produceconsumerobot/ofxThreadedLogger
- ofxBiquadFilter - https://github.com/mrbichel/ofxBiquadFilter
- ofxJSON - https://github.com/jeffcrouse/ofxJSON
- EmotiBit_XPlat_Utils - https://github.com/EmotiBit/EmotiBit_XPlat_Utils
- ofxLSL - https://github.com/EmotiBit/ofxLSL
  - _**NOTE:**_ for lsl support, if developing with visual studio, code should be compiled for x64
  - liblsl64.dll should always be in the same folder as the .exe (i.e. EmotiBitOscilloscope/bin/liblsl64.dll)
  - liblsl64.lib should always be linked to in under _solution properties->linker->general->additional library directories_ and _solution properties->linker->input-> additional dependencies_
  - both of these libs are handled properly by default, but should be considered if deviating from release code
- The project is built on a 64-bit architecture. Make sure you are on a machine that support `x64` build platform.
- Required to build EmotiBit FirmwareInstaller
  - ofxSerial - https://github.com/bakercp/ofxSerial
  - ofxIO - https://github.com/bakercp/ofxIO
## macOS 
### Note on using M1 macs (apple silicon)
- If you are using the new M1 macs, then make sure to use Rosetta installed in xcode.
- EmotiBit software has not been officially tested on M1/M2 macs. If you are building on these systems, please let the community know on the [forum](http://forum.emotibit.com) about any additional steps you had to take get the source to build!
### Setting up xcode project
- **Adding paths to Library search paths**
  - Check if the directory paths for the files `liblsl64-static.a` and `liblslboost.a` are already present in the `project` > `Build Settings` > `Library Search Paths`. If they are not present, follow the below steps:  
    - Select your project in the **Target group**(in xcode project navigator), go to **Build Settings** tab, and add the following path in the **Library Search Paths** section: `../../../addons/ofxLSL/libs/labstreaminglayer/lib/osx`

## Developing on Linux
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
- Note: When trying to run the EmotiBit Oscilloscope, if you get an error with the following message `cannot open shared object file: No such file or directory : liblsl-1.14.0-manylinux2010_x64`, make sure you have the latest master for [ofxLSL](https://github.com/EmotiBit/ofxLSL). The [fix](https://github.com/EmotiBit/ofxLSL/pull/8/files) added the required [shared object file](https://github.com/EmotiBit/ofxLSL/tree/add-linuxSharedLib/libs/labstreaminglayer/lib/linux64).

