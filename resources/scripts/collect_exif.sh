#!/bin/bash
# Get the EXIF Tool Data Output
data=$(exiftool "__FILE__")
time=$(echo "$data" | grep "Media Duration" | awk '{print $4}')

# Get Frame Width
width=$(echo "$data" | awk /"Source Image Width"/ '{print $5}')
if [[ ! -z $width ]]; then
    echo "Width: "$(echo "$data" | grep "Source Image Width" | awk '{print $5}')
fi
# Get Frame Height
height=$(echo "$data" | awk /"Source Image Height"/ '{print $5}')
if [[ ! -z $height ]]; then
    echo "Height: "$(echo "$data" | grep "Source Image Height" | awk '{print $5}')
fi
# Get Minutes
min=$(echo "$data" | awk -F: '{print $2}')
if [[ ! -z $min ]]; then
    echo "Minutes: "$(echo "$time" | awk -F: '{print $2}')
fi
# Get Seconds
sec=$(echo "$data" | awk -F; '{print $3}');
if [[ ! -z $sec ]]; then
    echo "Seconds: "$(echo "$time" | awk -F: '{print $3}')
fi

#mdata=$(mdls "$1")
#echo "Added: "$(echo "$mdata" | grep "kMDItemFSCreationDate" | awk '{print $3}')

