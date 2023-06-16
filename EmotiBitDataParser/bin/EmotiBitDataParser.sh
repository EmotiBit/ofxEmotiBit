#!/bin/bash

exePath=$(pwd)
dataDir=$(pwd)
reParse=false
getHelp=false

echo -e "";
echo -e "** EmotiBitDataParser.sh ** ";
echo -e "Runs EmotiBitDataParser on *.csv in passed data directory\n";
echo -e 'Example input:\n./EmotiBitDataParser.sh -x "C:\\Program Files\\EmotiBit\\EmotiBit DataParser\\EmotiBitDataParser.exe" -d "C:\\priv\\local\\EmotiBitData\\"\n'

while getopts x:d:r: flag
do
    case "${flag}" in
        x) exePath=${OPTARG};;
        d) dataDir=${OPTARG};;
        r) reParse=${OPTARG};;
    esac
done

# Replace backslashes with forward slashes
exePath="${exePath//\\//}"
dataDir="${dataDir//\\//}"

echo "[-x] exePath: $exePath";
echo "[-d] dataDir: $dataDir";
echo "[-r] reParse: $reParse";

currDir=$(pwd);
echo -e "\npwd: $currDir"

cd "$dataDir"

files="";
if $reParse
then
  # Process through [dirName]/[dirName].csv
  files=$(eval "ls -d */")
else
  # Process through all ./*.csv 
  files=$(eval "ls *.csv")
fi

for file in $files
do
	echo -e "\nprocessing: $file"
	subDirName=$(basename $file .csv)
  if ! $reParse
  then
    echo "mkdir: $subDirName"
    mkdir $subDirName
    mv $file $subDirName
    mv "$subDirName"_info.json $subDirName
  fi
	cd "$currDir" #cd to currDir before cmd Prevents ofDirectory errors
	cmd="\"$exePath\" \"$dataDir/$subDirName/$subDirName.csv\""
	echo "cmd: $cmd"
	eval $cmd
	cd "$dataDir"
done

cd "$currDir"
