#!/bin/bash

# Assignment 1 
# Author    : Karthik Baggaon Rajendra
# filename  : writer.sh


writefile=$1
writestr=$2

if [ $# != 2 ]
then
	echo "Error"
	echo "Invalid parameter passing"
	echo "The first parameter needs to be a path to a directory on the filesystem"
	echo "The second parameter needs to be a text string to be searched"
	exit 1
else
	echo "..."
fi


mkdir -p "$(dirname "$writefile")" && touch "$writefile"
echo "$writestr" > "$writefile"



if [[ ! -f $writefile ]]
then
    echo "$writefile file could not be created."
    exit 1
else
	echo "$writefile has been updated"
fi
