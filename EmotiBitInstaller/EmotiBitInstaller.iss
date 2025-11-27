; EmotiBit Installer Script
; Auto-reads version from src/ofxEmotiBitVersion.h

; Extract version using helper PowerShell script
#expr Exec("powershell", "-ExecutionPolicy Bypass -File get_version.ps1", SourcePath, , SW_HIDE)
#define VerFile FileOpen(SourcePath + "\version.tmp")
#define MyAppVersion Trim(FileRead(VerFile))
#expr FileClose(VerFile)
#expr DeleteFile(SourcePath + "\version.tmp")
#pragma message "Auto-detected version: " + MyAppVersion

[Setup]
; AppId: Keep same GUID for upgrades to work (from old .vdproj UpgradeCode)
AppId={{AAD962FC-B6FF-472D-BFD0-D28410DAD03A}
AppName=EmotiBit
AppVersion={#MyAppVersion}
AppPublisher=Connected Future Labs
DefaultDirName={autopf}\EmotiBit
DefaultGroupName=EmotiBit
OutputDir=Output
OutputBaseFilename=EmotiBitInstaller-{#MyAppVersion}
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

[Files]
; EmotiBit Oscilloscope
Source: "..\EmotiBitOscilloscope\bin\EmotiBitOscilloscope.exe"; DestDir: "{app}\EmotiBit Oscilloscope"
Source: "..\EmotiBitOscilloscope\bin\*.dll"; DestDir: "{app}\EmotiBit Oscilloscope"
Source: "..\EmotiBitOscilloscope\bin\data\*"; DestDir: "{app}\EmotiBit Oscilloscope\data"

; EmotiBit DataParser
Source: "..\EmotiBitDataParser\bin\EmotiBitDataParser.exe"; DestDir: "{app}\EmotiBit DataParser"
Source: "..\EmotiBitDataParser\bin\*.dll"; DestDir: "{app}\EmotiBit DataParser"
Source: "..\EmotiBitDataParser\bin\data\*"; DestDir: "{app}\EmotiBit DataParser\data"

; EmotiBit FirmwareInstaller
Source: "..\EmotiBitFirmwareInstaller\bin\EmotiBitFirmwareInstaller.exe"; DestDir: "{app}\EmotiBit FirmwareInstaller"
Source: "..\EmotiBitFirmwareInstaller\bin\*.dll"; DestDir: "{app}\EmotiBit FirmwareInstaller"
Source: "..\EmotiBitFirmwareInstaller\bin\data\*"; DestDir: "{app}\EmotiBit FirmwareInstaller\data"
Source: "..\EmotiBitFirmwareInstaller\bin\data\WINC\*"; DestDir: "{app}\EmotiBit FirmwareInstaller\data\WINC"
Source: "..\EmotiBitFirmwareInstaller\bin\data\esp32\*"; DestDir: "{app}\EmotiBit FirmwareInstaller\data\esp32"
Source: "..\EmotiBitFirmwareInstaller\bin\data\exec\win\*"; DestDir: "{app}\EmotiBit FirmwareInstaller\data\exec\win"
Source: "..\EmotiBitFirmwareInstaller\bin\data\instructions\*.jpg"; DestDir: "{app}\EmotiBit FirmwareInstaller\data\instructions"

; VC++ 2017 Redistributable
Source: "redist\vc_redist.x64.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall

[Icons]
; Start Menu shortcuts - {group} = "C:\ProgramData\Microsoft\Windows\Start Menu\Programs\EmotiBit\"
Name: "{group}\EmotiBit Oscilloscope"; Filename: "{app}\EmotiBit Oscilloscope\EmotiBitOscilloscope.exe"; WorkingDir: "{app}\EmotiBit Oscilloscope"
Name: "{group}\EmotiBit DataParser"; Filename: "{app}\EmotiBit DataParser\EmotiBitDataParser.exe"; WorkingDir: "{app}\EmotiBit DataParser"
Name: "{group}\EmotiBit FirmwareInstaller"; Filename: "{app}\EmotiBit FirmwareInstaller\EmotiBitFirmwareInstaller.exe"; WorkingDir: "{app}\EmotiBit FirmwareInstaller"

[Run]
; Install VC++ 2017 Redistributable if not already installed
Filename: "{tmp}\vc_redist.x64.exe"; Parameters: "/quiet /norestart"; StatusMsg: "Installing Visual C++ 2017 Redistributable..."; Check: VCRedistNeedsInstall

[Code]
function VCRedistNeedsInstall: Boolean;
var
  Version: String;
begin
  // Check registry for VC++ 2017 (version 14.10+)
  if RegQueryStringValue(HKLM, 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64', 'Version', Version) then
    Result := (Copy(Version, 2, 5) < '14.10')
  else
    Result := True;
end;
