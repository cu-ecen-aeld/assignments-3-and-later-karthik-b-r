#!/bin/bash

# Assignment 1 
# Author    : Karthik Baggaon Rajendra
# filename  : finder.sh


filesdir=$1
searchstr=$2

if [ $# != 2 ]
then
	echo "Error"
	echo "Invalid parameter passing"
	echo "The first parameter needs to be a path to a directory on the filesystem"
	echo "The second parameter needs to be a text string to be searched"
	exit 1
else
	echo ".....processing....." ;
	echo 
fi



if [ -d $filesdir ] 
then 
	echo "Valid directory provided" ;
	echo ".....processing....." ;
else
	echo "$filesdir is not a valid directory"
	echo "please pass a valid directory as the first parameter"
fi

X=$( ls $filesdir | wc -l)
Y=$( grep -r $searchstr $filesdir | wc -l)

echo "The number of files are $X and the number of matching lines are $Y"




