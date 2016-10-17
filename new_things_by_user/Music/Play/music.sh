#!/bin/bash

psMusic=`ps | grep music | grep .sh | sed 's/\(\w\+\)\s[^$]*/\1/'`
echo -e "music process(es):\n$psMusic"
arr=(${psMusic// /})
if [ ${#arr[@]} -eq 2 ];then
	echo no playing music
	if [ "$1" == "stop" ];then
		kill ${arr[0]}
		echo stopped
		exit
	fi
else
	echo playing music
	kill ${arr[0]}
	echo ${arr[0]} stopped
fi

psMpg=`ps | grep mpg123 |grep /media | sed 's/\s\(\w\+\)\s[^$]*/\1/'`
echo "song process: $psMpg"
if [ "$psMpg" == "" ];then
	echo no playing song
else
	kill -2 $psMpg
	echo $psMpg stopped
fi

songID=$1
if [ "$songID" == "stop" ];then
	kill ${arr[1]}
	echo stopped
else
	while true
	do
		song=`./read_jsonfile $songID`
		mpg123 "$song" 
		if [ "$songID" == "5" ];then
			let songID=1
		else
			let songID++
		fi
	done
fi
