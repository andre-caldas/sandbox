#!/bin/bash
set -x

BINPATH="$(dirname "$0")"

file1="$1"
shift
file2="$1"
shift

speed_and_lag="$("$BINPATH/drift" "$file1" "$file2" 0.01 "atsc_1080p_60" |
	egrep '(Adjust the speed)|(Calculated lag)' |
	sed -r -e 's,.*Adjust the speed.* ([0-9.]*%).*,\1,' \
	-e 's,Calculated lag: ([-+])([0-9]*) .*,\1\n\2,')"

speed="$(echo "$speed_and_lag" | cut -d "
" -f 1)"
lag_sign="$(echo "$speed_and_lag" | cut -d "
" -f 2)"
lag="$(echo "$speed_and_lag" | cut -d "
" -f 3)"

blank1=""
blank2=""
if [ '-' == "$lag_sign" ]; then
	blank1="-blank $lag";
else
	blank2="-blank $lag";
fi

melt -profile "atsc_1080p_60" $blank1 "$file1" -track $blank2 "$file2" -consumer xml
