#!/bin/bash
set -x

BINPATH="$(dirname "$0")"

file1="$1"
file2="$2"
profile="${3:-atsc_1080p_60}"

speed_and_lag="$("$BINPATH/drift" --script "$file1" "$file2" --profile "$profile" --print-precision 8)"

speed="$(echo "$speed_and_lag" | cut -d ";" -f 1)"
lag_file="$(echo "$speed_and_lag" | cut -d ";" -f 2)"
lag="$(echo "$speed_and_lag" | cut -d ";" -f 3)"

blank1=""
blank2=""
declare "blank$lag_file"="-blank $lag"

melt -profile "$profile" -video-track $blank1 "$file1" -audio-track $blank2 timewarp:"$speed":"$file2" -consumer xml
