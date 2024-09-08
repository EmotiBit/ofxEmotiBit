#!/bin/bash
if [[ $# -eq 0 ]]; then
    echo "Need to pass EMOTIBIT_ID to run script."
    echo "run ./EC.sh -h for more information" 
    exit 1;
fi
ARGS=$@
source EC.sh $ARGS
sleep 3
source ED.sh
sleep 3
source EC.sh $ARGS
sleep 3
source RB.sh
sleep 5
source RE.sh
sleep 5
source ED.sh
echo "test complete!"