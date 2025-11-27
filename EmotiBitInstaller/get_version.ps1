# Extract version from ofxEmotiBitVersion.h
$content = Get-Content "..\src\ofxEmotiBitVersion.h" | Select-String 'ofxEmotiBitVersion = '
$version = $content.ToString().Split('"')[1]
$version | Out-File -Encoding ASCII -NoNewline "version.tmp"
