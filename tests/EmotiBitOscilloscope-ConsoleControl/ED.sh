#!/bin/bash

unset -v UDP_PORT
DEFAULT_UDP_PORT=3130

while getopts 'p:h' opt; do
  case "$opt" in
    p)
      UDP_PORT="$OPTARG"
      ;;
   
    ?|h)
      echo "Usage: $(basename $0) [-p UDP_PORT]"
      exit 1
      ;;
  esac
done

if [ -z ${UDP_PORT} ]; then
	echo "using default port $DEFAULT_UDP_PORT"
	UDP_PORT=$DEFAULT_UDP_PORT
fi

UDP_PACKET=$(sed "s/\"ACTION\"/\"EMOTIBIT_DISCONNECT\"/" ./template.json)
#echo $UDP_PACKET

ncat -u 127.0.0.1 $UDP_PORT <<< "$UDP_PACKET"