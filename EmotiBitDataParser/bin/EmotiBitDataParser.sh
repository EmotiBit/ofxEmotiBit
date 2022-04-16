#!/bin/bash

exePath=$(pwd)
dataDir=$(pwd)
getHelp=false

while getopts x:d:h: flag
do
    case "${flag}" in
        x) exePath=${OPTARG};;
        d) dataDir=${OPTARG};;
				h) getHelp=${OPTARG};;
    esac
done

# Replace backslashes with forward slashes
exePath="${exePath//\\//}"
dataDir="${dataDir//\\//}"

echo "[-x] exePath: $exePath";
echo "[-d] dataDir: $dataDir";
echo "[-h] Print help: $getHelp";

if "$getHelp"; then
	echo -e "";
	echo -e "** EmotiBitDataParser.sh ** ";
	echo -e "Runs EmotiBitDataParser on *.csv in passed data directory\n";
	echo -e 'Example input:\n./EmotiBitDataParser.sh -x "C:\\Program Files\\EmotiBit\\EmotiBit DataParser\\EmotiBitDataParser.exe" -d "C:\\priv\\local\\EmotiBitData\\"\n'
	exit 0
fi

currDir=$(pwd);
echo "pwd=$currDir"

for file in $(eval "ls *.csv")
do
	#cd "$dataDir"
	echo "$file";
	subDirName=$(basename $file .csv)
	echo "$subDirName"
	mkdir $subDirName
  mv $file $subDirName
	mv "$subDirName"_info.json $subDirName
	cmd="\"$exePath\" \"$dataDir/$subDirName/$subDirName.csv\""
	echo "cmd=$cmd"
	eval $cmd
done

cd "$currDir"
