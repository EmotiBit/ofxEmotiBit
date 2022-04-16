#!/bin/bash

exePath=$(pwd)
dataDir=$(pwd)
getHelp=false

echo -e "";
echo -e "** EmotiBitDataParser.sh ** ";
echo -e "Runs EmotiBitDataParser on *.csv in passed data directory\n";
echo -e 'Example input:\n./EmotiBitDataParser.sh -x "C:\\Program Files\\EmotiBit\\EmotiBit DataParser\\EmotiBitDataParser.exe" -d "C:\\priv\\local\\EmotiBitData\\"\n'

while getopts x:d: flag
do
    case "${flag}" in
        x) exePath=${OPTARG};;
        d) dataDir=${OPTARG};;
    esac
done

# Replace backslashes with forward slashes
exePath="${exePath//\\//}"
dataDir="${dataDir//\\//}"

echo "[-x] exePath: $exePath";
echo "[-d] dataDir: $dataDir";

currDir=$(pwd);
echo -e "\npwd: $currDir"

for file in $(eval "ls *.csv")
do
	echo -e "\nprocessing: $file"
	subDirName=$(basename $file .csv)
	echo "mkdir: $subDirName"
	mkdir $subDirName
  mv $file $subDirName
	mv "$subDirName"_info.json $subDirName
	cmd="\"$exePath\" \"$dataDir/$subDirName/$subDirName.csv\""
	echo "cmd: $cmd"
	eval $cmd
done

cd "$currDir"
