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
AppVerName=EmotiBit
AppPublisher=Connected Future Labs
DefaultDirName={autopf}\EmotiBit
DefaultGroupName=EmotiBit
UninstallDisplayIcon={app}\EmotiBitLogo.ico
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
Source: "..\EmotiBitOscilloscope\bin\data\*.json"; DestDir: "{app}\EmotiBit Oscilloscope\data"
Source: "..\EmotiBitOscilloscope\bin\data\*.xml"; DestDir: "{app}\EmotiBit Oscilloscope\data"
Source: "..\EmotiBitOscilloscope\bin\data\*.ttf"; DestDir: "{app}\EmotiBit Oscilloscope\data"

; EmotiBit DataParser
Source: "..\EmotiBitDataParser\bin\EmotiBitDataParser.exe"; DestDir: "{app}\EmotiBit DataParser"
Source: "..\EmotiBitDataParser\bin\*.dll"; DestDir: "{app}\EmotiBit DataParser"
Source: "..\EmotiBitDataParser\bin\data\*.json"; DestDir: "{app}\EmotiBit DataParser\data"
Source: "..\EmotiBitDataParser\bin\data\*.ttf"; DestDir: "{app}\EmotiBit DataParser\data"

; EmotiBit FirmwareInstaller
Source: "..\EmotiBitFirmwareInstaller\bin\EmotiBitFirmwareInstaller.exe"; DestDir: "{app}\EmotiBit FirmwareInstaller"
Source: "..\EmotiBitFirmwareInstaller\bin\*.dll"; DestDir: "{app}\EmotiBit FirmwareInstaller"
Source: "..\EmotiBitFirmwareInstaller\bin\data\bossac.exe"; DestDir: "{app}\EmotiBit FirmwareInstaller\data"
Source: "..\EmotiBitFirmwareInstaller\bin\data\EmotiBit.png"; DestDir: "{app}\EmotiBit FirmwareInstaller\data"
Source: "..\EmotiBitFirmwareInstaller\bin\data\*.bin"; DestDir: "{app}\EmotiBit FirmwareInstaller\data"
Source: "..\EmotiBitFirmwareInstaller\bin\data\*.ttf"; DestDir: "{app}\EmotiBit FirmwareInstaller\data"
Source: "..\EmotiBitFirmwareInstaller\bin\data\WINC\FirmwareUpdater.ino.feather_m0.bin"; DestDir: "{app}\EmotiBit FirmwareInstaller\data\WINC"
Source: "..\EmotiBitFirmwareInstaller\bin\data\WINC\FirmwareUploader.exe"; DestDir: "{app}\EmotiBit FirmwareInstaller\data\WINC"
Source: "..\EmotiBitFirmwareInstaller\bin\data\WINC\LICENSE.txt"; DestDir: "{app}\EmotiBit FirmwareInstaller\data\WINC"
Source: "..\EmotiBitFirmwareInstaller\bin\data\WINC\m2m_aio_3a0.bin"; DestDir: "{app}\EmotiBit FirmwareInstaller\data\WINC"
Source: "..\EmotiBitFirmwareInstaller\bin\data\esp32\*"; DestDir: "{app}\EmotiBit FirmwareInstaller\data\esp32"
Source: "..\EmotiBitFirmwareInstaller\bin\data\exec\win\*"; DestDir: "{app}\EmotiBit FirmwareInstaller\data\exec\win"
Source: "..\EmotiBitFirmwareInstaller\bin\data\instructions\*.jpg"; DestDir: "{app}\EmotiBit FirmwareInstaller\data\instructions"

; VC++ 2017 Redistributable
Source: "redist\vc_redist.x64.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall

; EmotiBit Icon
Source: "..\EmotiBitIcons\icoFiles\EmotiBitLogo.ico"; DestDir: "{app}"

[Icons]
; Start Menu shortcuts - {group} = "C:\ProgramData\Microsoft\Windows\Start Menu\Programs\EmotiBit\"
Name: "{group}\EmotiBit Oscilloscope"; Filename: "{app}\EmotiBit Oscilloscope\EmotiBitOscilloscope.exe"; WorkingDir: "{app}\EmotiBit Oscilloscope"
Name: "{group}\EmotiBit DataParser"; Filename: "{app}\EmotiBit DataParser\EmotiBitDataParser.exe"; WorkingDir: "{app}\EmotiBit DataParser"
Name: "{group}\EmotiBit FirmwareInstaller"; Filename: "{app}\EmotiBit FirmwareInstaller\EmotiBitFirmwareInstaller.exe"; WorkingDir: "{app}\EmotiBit FirmwareInstaller"

[Run]
; Install VC++ 2017 Redistributable if not already installed
Filename: "{tmp}\vc_redist.x64.exe"; Parameters: "/passive /norestart"; StatusMsg: "Installing Visual C++ 2017 Redistributable..."; Check: VCRedistNeedsInstall

[Code]
var
  DriverAckPage: TWizardPage;
  DriverInfoLabel: TNewStaticText;
  DriverLinkLabel: TNewStaticText;
  DriverAckCheckbox: TNewCheckBox;

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

procedure DriverLinkClick(Sender: TObject);
var
  ErrorCode: Integer;
begin
  ShellExec('', 'https://github.com/EmotiBit/EmotiBit_Docs/blob/master/Getting_Started.md#install-drivers', '', '', SW_SHOW, ewNoWait, ErrorCode);
end;

procedure InitializeWizard();
begin
  // Create custom page for driver acknowledgment (after installation completes)
  DriverAckPage := CreateCustomPage(wpInstalling,
    'Driver Installation Required',
    'Users need to install USB drivers to successfully program the ESP32 Feather');

  // Add informational text
  DriverInfoLabel := TNewStaticText.Create(DriverAckPage);
  DriverInfoLabel.Parent := DriverAckPage.Surface;
  DriverInfoLabel.Left := 0;
  DriverInfoLabel.Top := 0;
  DriverInfoLabel.Width := DriverAckPage.SurfaceWidth;
  DriverInfoLabel.Height := ScaleY(120);
  DriverInfoLabel.AutoSize := False;
  DriverInfoLabel.WordWrap := True;
  DriverInfoLabel.Caption :=
    'After this installation completes, you MUST install the USB drivers to be able to run the FirmwareInstaller with Feather ESP32.' + #13#10#13#10 +
    'Driver installation steps:' + #13#10 +
    '1. The driver installer is included in the downloaded package' + #13#10 +
    '2. Navigate to the download location' + #13#10 +
    '3. Run the CP210x driver installer for your system';

  // Add clickable link for driver installation guide
  DriverLinkLabel := TNewStaticText.Create(DriverAckPage);
  DriverLinkLabel.Parent := DriverAckPage.Surface;
  DriverLinkLabel.Left := 0;
  DriverLinkLabel.Top := DriverInfoLabel.Top + DriverInfoLabel.Height + ScaleY(10);
  DriverLinkLabel.Width := DriverAckPage.SurfaceWidth;
  DriverLinkLabel.Height := ScaleY(16);
  DriverLinkLabel.Caption := 'Click here for detailed driver installation instructions';
  DriverLinkLabel.Font.Color := clBlue;
  DriverLinkLabel.Font.Style := [fsUnderline];
  DriverLinkLabel.Cursor := crHand;
  DriverLinkLabel.OnClick := @DriverLinkClick;

  // Add checkbox at the bottom
  DriverAckCheckbox := TNewCheckBox.Create(DriverAckPage);
  DriverAckCheckbox.Parent := DriverAckPage.Surface;
  DriverAckCheckbox.Left := 0;
  DriverAckCheckbox.Top := DriverAckPage.SurfaceHeight - ScaleY(20);
  DriverAckCheckbox.Width := DriverAckPage.SurfaceWidth;
  DriverAckCheckbox.Caption := 'Click to acknowledge: I understand that I need to install the USB drivers separately';
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
  Result := True;

  // Check if user acknowledged the driver requirement
  if CurPageID = DriverAckPage.ID then
  begin
    if not DriverAckCheckbox.Checked then
    begin
      MsgBox('You must acknowledge that you will install the required USB drivers to continue.', mbError, MB_OK);
      Result := False;
    end;
  end;
end;
