#!/bin/bash
#shopt -s globstar
./makeallclean.sh
wc -l ` find . -name '*.h' -o -name '*.cpp' -o -name '*.body' -o -name '*.pro' `

