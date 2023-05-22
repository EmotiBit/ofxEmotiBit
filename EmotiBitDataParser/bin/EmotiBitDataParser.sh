#!/bin/bash

exePath=$(pwd)
dataDir=$(pwd)
reRun=false
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
        r) reRun=${OPTARG};;
    esac
done

# Replace backslashes with forward slashes
exePath="${exePath//\\//}"
dataDir="${dataDir//\\//}"

echo "[-x] exePath: $exePath";
echo "[-d] dataDir: $dataDir";
echo "[-r] reRun: $reRun";

currDir=$(pwd);
echo -e "\npwd: $currDir"

cd "$dataDir"

files="";
if $reRun
then
  files=$(eval "ls -d */")
else
  files=$(eval "ls *.csv")
fi

for file in $files
do
	echo -e "\nprocessing: $file"
	subDirName=$(basename $file .csv)
  if ! $reRun
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
