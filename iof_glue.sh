#!/bin/bash

#TODO: check number of arguments

#if [ $# -lt 3 ];then
#	exit 1
#fi

MONITOR_IP=172.16.0.31
MONITOR_PORT=1340

op=$1 # create/delete
pool=$2 # pool name
volume_name=$3 
volume_size=$4


echo "op: $op" > /usr/iof/temp
echo "pool name: $pool" >> /usr/iof/temp
echo "volume name: $volume_name" >> /usr/iof/temp
echo "volume size: $volume_size" >> /usr/iof/temp

if [ $op == "create" ];then
	echo "create" >> /usr/iof/temp

	#use all devices
	iofsh --host $MONITOR_IP --port $MONITOR_PORT /nodes/ use_all use_ramdisks=False

	#create volume
	iofsh --host $MONITOR_IP --port $MONITOR_PORT /volumes/ add volume_name=$volume_name replicas=2 access_type=converged export_type=block volume_type=manual add_devices=all capacity=$volume_size

	#start volume
	iofsh --host $MONITOR_IP --port $MONITOR_PORT /volumes/$volume_name start

	exit 0
fi

if [ $op == "delete" ];
	echo "delete" >> /usr/iof/temp

	#stop volume
	iofsh --host $MONITOR_IP --port $MONITOR_PORT /volumes/$volume_name stop

	#remove volume
	iofsh --host $MONITOR_IP --port $MONITOR_PORT /volumes/$volume_name remove

	exit 0
fi
