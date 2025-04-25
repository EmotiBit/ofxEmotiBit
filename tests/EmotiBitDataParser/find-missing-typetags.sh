#!/bin/bash

emotibitPacketFile="temp_emotibitpacket.txt"
curl https://raw.githubusercontent.com/EmotiBit/EmotiBit_XPlat_Utils/refs/heads/master/src/EmotiBitPacket.cpp > $emotibitPacketFile
tt_list=""

directory="./sample_data"
basename="2025-03-18_10-27-52-355758"
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
tt_ignore_list="ER O2 T0 H0 BS BL SD RS TE TU EI GL GS GB GA RE S+ S- PN PO HE HH EC WA WD LS"

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