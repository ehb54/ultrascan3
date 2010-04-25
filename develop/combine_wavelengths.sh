#!/bin/sh
mkdir combined
for c in 1 2 3; do
	y=0
	for x in 1*; do
		if [ -d $x ]; then
			if [ -e $x/00001.WA$c ]; then
			 	y=`expr $y + 1`
				if [ $y -gt 9 ]; then
					cp $x/00001.WA$c combined/000$y.wa$c
				else
					cp $x/00001.WA$c combined/0000$y.wa$c
				fi
			fi
		fi
	done
done
