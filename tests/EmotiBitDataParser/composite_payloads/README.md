# Description
- This tests validates the parser output for typetags containing composite payloads
- The following typetags are included in this test
  - `EM` - `24187,1518,4,EM,1,100,RS,RB,2025-03-20_12-09-40-822726.csv,PS,MN`
  - `AK` - `28260,2177,2,AK,1,100,1441,RD`
  - `DO`
  - `DC` - `50201,5704,4,DC,2,100,AZ,1,GX,1`
  - `RD` - `53830,6320,2,RD,1,100,TL,TU`

# Helper Scripts
- The test data is created using the python script `create_test_data.py`. The script needs a dynamic library, that provides python bindings for EmotiBit Packet. See `EmotiBit_Plugins` repository for more information. The `.so` file is not checked in since it a binary.
- The md5sum for each typetag under test is created using the `md5_creator.sh`. Run this script from the `composite_payloads` test folder.
- The test data is stored in the `test_data` directory. 
- To run the test, execute the script `run_test.sh`. The script needs the path to ofxEmotiBit as an input to run.
