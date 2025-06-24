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
|6|Multiple responses, missing correct TL | ✔️ | ✔️ d
|7|Multiple responses, missing wrong AK | ✔️ | ✔️ |
|8|Multiple responses, missing correct AK| ✔️ | ✔️ |
|9|No response | ✔️ | ✔️ |
|10| received wrong TL and correct AK, but dropped wrong AK | ✔️  | ✔️ |

## Test cases
1. Normal `RD-TL-AK` triplet. See the following lines in the test case
```
97724,11553,2,RD,1,100,TL,TU
97736,11554,1,TL,1,100,2022-04-20_12-11-15-086868
97739,11555,2,AK,1,100,11553,RD
```
2. Multiple TL-AK responses for 1 RD.
This can happen when previous responses (echo's from data channel tx) for RD get processed.
```
107990,13151,2,RD,1,100,TL,TU
108004,13152,2,AK,1,100,12352,RD
108015,13153,1,TL,1,100,2022-04-20_12-11-25-459255
108024,13154,2,AK,1,100,12352,RD
108038,13155,1,DN,1,100,Case 6 correct TL missing for RD 13151
108045,13156,2,AK,1,100,13151,RD
```
3. `AK` dropped under UDP
```
51484,4535,2,RD,1,100,TL,TU
51585,4538,1,TL,1,100,2022-04-20_12-10-33-314042
51586,4538,1,DN,1,100,Case 3 No ACK  for RD 4535
```
4. `TL` dropped under UDP
```
71918,7627,2,RD,1,100,TL,TU
71941,7628,1,DN,1,100,Case 4 No TL for RD 7627
71944,7629,2,AK,1,100,7627,RD
```
5. Multiple TL responses. Incorrect TL missing. (example)
```
34909,1818,2,RD,1,100,TL,TU
[UDP loss]
34930,1820,2,AK,1,100,918,RD
34938,1821,1,TL,1,100,2022-02-03_15-36-38-612039
34942,1822,2,AK,1,100,1818,RD
```
6. Multiple TL responses. Correct TL missing (example)
```
34909,1818,2,RD,1,100,TL,TU
34921,1819,1,TL,1,100,2022-02-03_15-36-33-427125
34930,1820,2,AK,1,100,918,RD
[UDP loss]
34942,1822,2,AK,1,100,1818,RD
```
7. Multiple AK, with the incorrect AK missing
```
118225,14748,2,RD,1,100,TL,TU
118238,14749,2,AK,1,100,13951,RD
118249,14750,1,TL,1,100,2022-04-20_12-11-35-893132
118254,14751,1,DN,1,100,case 7 Wrong AK missing for RD 14748
118270,14752,1,TL,1,100,2022-04-20_12-11-40-883404
118273,14753,2,AK,1,100,14748,RD
```
8. Multiple AK, with the correct AK missing
```
77060,8441,2,RD,1,100,TL,TU
77073,8442,1,DN,1,100,Case 5 wrong Tl missing  for RD 8441
77077,8443,2,AK,1,100,7627,RD
77147,8444,1,TL,1,100,2022-04-20_12-10-59-162922
77150,8445,1,DN,1,100,Case 8 correct AK missing for RD 8441
```
9. No responses. ToDo: Add example here.
10. Received wrong TL and correct AK, but dropped wrong AK
```
34909,1818,2,RD,1,100,TL,TU
34921,1819,1,TL,1,100,2022-02-03_15-36-33-427125
[UDP loss]34930,1820,2,AK,1,100,918,RD
[UDP loss]34938,1821,1,TL,1,100,2022-02-03_15-36-38-612039
34942,1822,2,AK,1,100,1818,RD
```













- See https://github.com/EmotiBit/ofxEmotiBit/issues/99 for more information
