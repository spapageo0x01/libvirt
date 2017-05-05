#!/bin/bash

#TODO: check number of arguments

#if [ $# -lt 3 ];then
#	exit 1
#fi

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
	
	iofsh /volumes/ add volume_name=$volume_name replicas=2 access_type=converged export_type=block volume_type=manual add_devices=all capacity=$volume_size

	exit 0
fi

if [ $op == "delete" ];
	echo "delete" >> /usr/iof/temp
	#TODO: Call iofsh with proper arguments
	exit 0

fi
