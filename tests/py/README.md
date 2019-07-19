#### Usage
- This is an ipython file that can be run in jupyter notebook in tandem with the ofxLSL python marker code to check accuracy
- File contains computer specific paths and is in the alpha phase, included as a template for in house testing
- The code most easily functions when the LSL stream data is printed to console by uncommenting the couts in:
  - _void ofApp::update()_ found in ofApp.cpp of EmotiBit Oscilloscope
  - _void ofApp::parseIncomingRequestData()_ of EmotiBit Oscilloscope
  - Copy and paste the resulting console stream into consoleDataLM.csv and remove everything but the LM lines

#### About
- Packet Format
  - TX: TL,LC
  - LM: TSC,TS,LC, DATA
- Accuracy (tested across different computers)
  - **TS** is 100% accurate
  - **TSC** periodicity is at worst accurate to 1.8ms
    - _average:_ 19 microseconds
	- defined as {TSC(n) - TSC(n-1)} - {TSpy(n) - TSpy(n-1)}
  - **LC** periodicity is rather unreliable (>300ms)
  - Can be more accurate on same computer