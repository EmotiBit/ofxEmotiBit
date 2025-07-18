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

echo "Running parsed output data format test"
echo "backup the existing data format file"
backup_filename="parsedDataFormat.json.bckup"

# store original file as backup
mv $ENV_OFXEMOTIBIT_DIR/EmotiBitDataParser/bin/data/parsedDataFormat.json $ENV_OFXEMOTIBIT_DIR/EmotiBitDataParser/bin/data/${backup_filename}
# copy modified format file
cp $ENV_OFXEMOTIBIT_DIR/tests/EmotiBitDataParser/parsed_output_format/parsedDataFormat.json $ENV_OFXEMOTIBIT_DIR/EmotiBitDataParser/bin/data/parsedDataFormat.json

${ENV_OFXEMOTIBIT_DIR}/EmotiBitDataParser/bin/EmotiBitDataParser "${ENV_OFXEMOTIBIT_DIR}/tests/EmotiBitDataParser/sample_data/2025-06-27_09-56-59-929206.csv"
# rename the backup
echo "renaming the backup"
mv ${ENV_OFXEMOTIBIT_DIR}/EmotiBitDataParser/bin/data/${backup_filename} ${ENV_OFXEMOTIBIT_DIR}/EmotiBitDataParser/bin/data/parsedDataFormat.json

cd tests/EmotiBitDataParser/parsed_output_format
# check the hash
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
