echo "Parsing sample data"
$OFXEMOTIBIT_DIR/EmotiBitDataParser/bin/EmotiBitDataParser "$OFXEMOTIBIT_DIR/tests/EmotiBitDataParser/sample_data/2025-03-20_12-09-40-822726.csv"
if [ $? -eq 0 ]; then
  echo "Sample file parsed successfully"
else
  echo "parsing failed"
  exit 1
fi