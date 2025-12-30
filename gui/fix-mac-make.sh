#!/bin/sh
#      Fix Makefile in gui for Mac  (add qrc_images lines)

if [ `grep -c 'qrc_us3_gui_images' Makefile` -eq 0 ]; then
  # This Makefile is missing qrc_images:  insert lines

  # Count lines to 3 insertion points
  LINE1=`sed -n '1,/^SOURCES/p' Makefile|wc -l`
  LINE2=`sed -n '1,/^OBJECTS  /p' Makefile|wc -l`
  LINE3=`sed -n '1,/^staticlib/p' Makefile|wc -l`

  # Line numbers as numerics, plus next lines
  LINE1=`expr ${LINE1}`
  LINE1B=`expr ${LINE1} + 1`
  LINE2=`expr ${LINE2}`
  LINE2B=`expr ${LINE2} + 1`
  LINE3=`expr ${LINE3} - 2`
  LINE3B=`expr ${LINE3} + 1`

  # Copy lines, insert text, copy more lines, ...
  sed -n "1,${LINE1}p" Makefile >Makefile_mac
  echo "	qrc_us3_gui_images.cpp \\" >>Makefile_mac
  sed -n "${LINE1B},${LINE2}p" Makefile >>Makefile_mac
  echo "	obj/qrc_us3_gui_images.o \\" >>Makefile_mac
  sed -n "${LINE2B},${LINE3}p" Makefile >>Makefile_mac
  cat qrc_make_adds.txt >> Makefile_mac
  sed -n "${LINE3B},9999p" Makefile >>Makefile_mac

  # Save qmake-created Makefile, rename modified Makefile
  mv Makefile     Makefile_qmake
  mv Makefile_mac Makefile
fi

