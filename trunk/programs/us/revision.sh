REV1=`svn info --depth files|grep Revision|sort -rk 2,2|sed -n 1p`
REV2=`cut -b 19-31 us_revision.h`
if [ "$REV1" != "$REV2" ]
then
  
  echo "Setting revision to $REV1"
  # Remove return character from string when running under cygwin
  REV1=`echo $REV1 | sed s/\r//`
  echo "#define REVISION \"$REV1\"" > us_revision.h
fi


