#!/bin/bash

unset -v EMOTIBIT_ID
unset -v UDP_PORT
DEFAULT_UDP_PORT=3130
VALID_ARGS=$(getopt -o e:p:h --long emotibit-id:,help -- "$@")

if [[ $? -ne 0 ]]; then
    exit 1;
fi
eval set -- "$VALID_ARGS"
while [ : ]; do
  case "$1" in
    -e | --emotibit-id)
        #echo "Processing 'alpha' option"
        EMOTIBIT_ID=$2
        shift 2
        ;;
    -h | --help)
		echo "Example: $(basename $0) --emotibit-id MD-V5-1000001"
		exit 0
		;;
	-p)
		UDP_PORT=$2
		shift 2
		;;
	--) shift;
		break
		;;
  esac
done

if [ -z ${EMOTIBIT_ID} ]; then
	echo "emotibit_id required in arguments"
	exit 1
fi

if [ -z ${UDP_PORT} ]; then
	echo "using default port $DEFAULT_UDP_PORT"
	UDP_PORT=$DEFAULT_UDP_PORT
fi

UDP_PACKET=$(sed "s/\"ACTION\"/\"EMOTIBIT_CONNECT\",\"$EMOTIBIT_ID\"/" ./template.json)
#echo $UDP_PACKET

ncat -u 127.0.0.1 $UDP_PORT <<< "$UDP_PACKET"