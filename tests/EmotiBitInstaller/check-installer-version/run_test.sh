#!/bin/bash
product_version_str=`grep ProductVersion ../../../EmotiBitInstaller/EmotiBitInstaller/EmotiBitInstaller.vdproj`
#echo $product_version_str
productVersion=$(echo "$product_version_str" | sed 's/.*8://' | sed 's/"//')
echo "Product version: $productVersion"

ofxEmotiBitVersion=`grep "ofxEmotiBitVersion =" ../../../src/ofxEmotiBitVersion.h`
#echo $ofxEmotiBitVersion
ofxVersion=$(echo "$ofxEmotiBitVersion" | sed 's/.*= "//' | sed 's/".//')
echo "ofxEmotiBitVersion: $ofxVersion"
if [ "$ofxVersion" = "$productVersion" ]; then
	echo "product version is up to date"
	exit 0
else
	echo "product version not updated in installer project"
	exit 1
fi

