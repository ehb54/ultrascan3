#!/bin/sh

sed -i  's/release/debug/g' libus.pro
sed -i  's/release/debug/g' generic.pri


for each in us*; do
  if [ -d $each ]; then
    cd $each
    echo $each

#  This should not be necessary when the .pro files all use generic.pri
    for f in *.pro; do
        sed -i 's/release/debug/g' $f
    done

    qmake us*.pro
    cd ..
  fi
done
