#!/bin/bash

echo "Parsing sample data"
unset ENV_OFXEMOTIBIT_DIR

OPTIONS=$(getopt -o d: --long --ofxemotibit-dir: -n "run_tesh.sh" -- "$@")

if [ $? -ne 0 ]; then
  echo "Terminating script." >&2
  exit 1
fi

eval set -- "$OPTIONS"

while true; do
  case "$1" in
    -d | --ofxemotibit-dir)
      echo "ofxEmotibit dir location: $2"
      ENV_OFXEMOTIBIT_DIR=$2
      shift 2
      ;;
    --) # End of options marker
      shift
      break
      ;;
  esac
done

if [ -z "$ENV_OFXEMOTIBIT_DIR" ]; then
	echo "path to ofxEmotiBit not provided. Aborting"
	exit 1
fi


${ENV_OFXEMOTIBIT_DIR}/EmotiBitDataParser/bin/EmotiBitDataParser "${ENV_OFXEMOTIBIT_DIR}/tests/EmotiBitDataParser/sample_data/2025-03-20_12-09-40-822726.csv"

echo "find missing typetags from parsed data"
emotibitPacketFile="temp_emotibitpacket.txt"
curl https://raw.githubusercontent.com/EmotiBit/EmotiBit_XPlat_Utils/refs/heads/master/src/EmotiBitPacket.cpp > $emotibitPacketFile
tt_list=""

directory="../sample_data"
basename="2025-03-20_12-09-40-822726"
for file in "$directory"/*; do
    if [ -f "$file" ]; then
		file=${file#$directory"/"}
		if echo $file | grep -q ".csv"; then  # if its a csv file
			dot_index=`expr index "$file" .`  # find index of "."
			ext=${file:dot_index:3}  # extract extension
			tt=${file#$basename"_"}  # extract typetag
			tt=${tt%"."$ext}  # extract typetag
			file_tt_list="${file_tt_list} ${tt}" # appenf typetag to list
			#echo $tt
			#echo "$file"
		fi
    fi
done
tt_ignore_list="ER O2 T0 H0 BS BL SD RS TE TU EI GL GS GB GA RE S+ S- PN PO HE HH EC ED WA WD LS MN ML MM MO MH"

echo "The following typatags are not in the parsed data"
while IFS= read -r line; do
	match_val=`expr match "$line" 'const char\* EmotiBitPacket::TypeTag::.*'`
	if [ $match_val -eq 0 ]; then # if line contains a typetag definition
		continue
	else
		tt_index=`expr index "$line" =` # find typetag definition index
		tt=${line:tt_index+2:2}  # extract typetag
		if echo "$file_tt_list" | grep -q "$tt"; then
			continue
		else
			if echo "$tt_ignore_list" | grep -q "$tt"; then
				# tt in ginore list
				continue
			else
				echo $line  # if typetag not in list, print it out
			fi
		fi
	fi
done < "$emotibitPacketFile"
rm $emotibitPacketFile
