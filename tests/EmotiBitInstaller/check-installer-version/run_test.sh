#!/bin/bash

#echo "Parsing sample data"
#unset ENV_OFXEMOTIBIT_DIR
#
#OPTIONS=$(getopt -o d: --long --ofxemotibit-dir: -n "run_tesh.sh" -- "$@")
#
#if [ $? -ne 0 ]; then
  #echo "Terminating script." >&2
  #exit 1
#fi
#
#eval set -- "$OPTIONS"
#
#while true; do
  #case "$1" in
    #-d | --ofxemotibit-dir)
      #echo "ofxEmotibit dir location: $2"
      #ENV_OFXEMOTIBIT_DIR=$2
      #shift 2
      #;;
    #--) # End of options marker
      #shift
      #break
      #;;
  #esac
#done
#
#if [ -z "$ENV_OFXEMOTIBIT_DIR" ]; then
	#echo "path to ofxEmotiBit not provided. Aborting"
	#exit 1
#fi
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

