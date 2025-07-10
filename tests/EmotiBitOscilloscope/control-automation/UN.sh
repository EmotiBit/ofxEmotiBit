#!/bin/bash

unset -v USER_NOTE
unset -v UDP_PORT
DEFAULT_UDP_PORT=3130
VALID_ARGS=$(getopt -o u:p:h --long emotibit-id:,help -- "$@")

if [[ $? -ne 0 ]]; then
    exit 1;
fi
eval set -- "$VALID_ARGS"
while [ : ]; do
  case "$1" in
    -u | --user-note)
        #echo "Processing 'alpha' option"
        USER_NOTE=$2
        shift 2
        ;;
    -h | --help)
		echo "Example: $(basename $0) --user-note THIS_IS_MY_NOTE"
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

if [ -z ${USER_NOTE} ]; then
	echo "user note is required"
	exit 1
fi

if [ -z ${UDP_PORT} ]; then
	echo "using default port $DEFAULT_UDP_PORT"
	UDP_PORT=$DEFAULT_UDP_PORT
fi

echo "Sending USER_NOTE"
# use stream editor to update the action list
UDP_PACKET=$(sed "s/\"ACTION\"/\"USER_NOTE\",\"$USER_NOTE\"/" ./template.json)
echo $UDP_PACKET

ncat -u 127.0.0.1 $UDP_PORT <<< "$UDP_PACKET"