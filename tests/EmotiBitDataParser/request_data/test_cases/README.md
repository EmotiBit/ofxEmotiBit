# Description
- This file contains a typical EmotiBit recording with RD-TL-AK triplets modified to simulate possible failures in data transmission and how the DataParser handles these failures.

# How to run the test
- Use the DataParser to parse this file and compare the `test_cases_timesyncs.csv` output to `EXPECTED_OUTPUT.csv`

# Test details
- The file `test_cases.csv` is a modified recording, where the RD-TL-AK triplets have been modified to simulate errors in the timesyncing.

|Case number| Case description| is timeSync parsed correctly? | is timeSync file written correctly?|
|--------------|-------------------|---------------------------------|---------------------------------|
|1|Expected | ✔️  | ✔️  |
|2| 2 or mode TL received for 1 RD |  ✔️ | ✔️|
|3|No AK from host | ✔️| ✔️|
|4| No TL from host | ✔️ | ✔️ |
|5|Multiple responses, missing wrong TL | ✔️ | ✔️ |
|6|Multiple responses, missing correct TL | ✔️ | ✔️ |
|7|Multiple responses, missing wrong AK | ✔️ | ✔️ |
|8|Multiple responses, missing correct AK| ✔️ | ✔️ |
|9|No response | ✔️ | ✔️ |
|10| received wrong TL and correct AK, but dropped wrong AK | ✔️  | ✔️ |

- See https://github.com/EmotiBit/ofxEmotiBit/issues/99 for more information
