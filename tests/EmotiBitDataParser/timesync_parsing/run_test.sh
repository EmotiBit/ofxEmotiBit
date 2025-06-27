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


${ENV_OFXEMOTIBIT_DIR}/EmotiBitDataParser/bin/EmotiBitDataParser "${ENV_OFXEMOTIBIT_DIR}/tests/EmotiBitDataParser/timesync_parsing/test_cases/2022-04-20_12-10-28-051739.csv"

cd tests/EmotiBitDataParser/timesync_parsing/
hash_dir="expected_output_hash"
for file in "$hash_dir"/*; do
  if [ -f "$file" ]; then
    #echo "checking $file"
    md5sum -c $file
    if [ $? -eq 0 ]; then
      echo "MD5 checksum verification successful."
    else
      echo "MD5 checksum verification failed."
      exit 1
    fi
  fi
done
exit 0
