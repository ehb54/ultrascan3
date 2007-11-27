#!/bin/sh
mkdir newcpp
for x in *.html;do
        sed<$x>newcpp/$x 's/Copyright 1998 - 2000/Copyright 1998 - 2001/'
        mv newcpp/$x .
done
rm -rf newcpp
