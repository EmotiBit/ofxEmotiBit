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
      echo "ofxEmotibit dir location"
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

