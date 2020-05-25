; -- 64Bit.iss --
; Demonstrates installation of a program built for the x64 (a.k.a. AMD64)
; architecture.
; To successfully run this installation and the program it installs,
; you must have a "x64" edition of Windows.

; SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!

#define URL "https://github.com/YukiWorks432/YoutubeDownloader"

[Setup]
AppName=YoutubeDownloader
AppVersion=0.1.0.0
AppPublisher="Yuki"
AppPublisherURL={#URL}
AppSupportURL="https://twitter.com/yuk1_works"
AppUpdatesURL={#URL}
WizardStyle=modern
DefaultDirName=C:\Tools\YoutubeDownloader
SourceDir="C:\youtube-dl\YoutubeDownloader"
LicenseFile=Readme.txt
OutputDir=C:\youtube-dl
OutputBaseFilename=YoutubeDownloader_install
SolidCompression=yes
Compression=lzma2/ultra64
LZMAUseSeparateProcess=yes
LZMADictionarySize=589824
LZMANumFastBytes=273
; "ArchitecturesAllowed=x64" specifies that Setup cannot run on
; anything but x64.
ArchitecturesAllowed=x64
; "ArchitecturesInstallIn64BitMode=x64" requests that the install be
; done in "64-bit mode" on x64, meaning it should use the native
; 64-bit Program Files directory and the 64-bit view of the registry.
ArchitecturesInstallIn64BitMode=x64

[Files]
Source: "*";                            DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "Readme.txt";                   DestDir: "{app}"; Flags: isreadme

[Run]
Filename: "{app}\YoutubeDownloader.exe"; Description: "インストール完了後ゲームを起動する"; Flags: postinstall shellexec skipifsilent  

[Languages]
Name: japanease; MessagesFile: compiler:Languages\Japanese.isl