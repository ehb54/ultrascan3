REV1=`(cd ../develop;svn info|grep Revision)`
REV2=`cut -f2 -d\" include/us_revision.h`
if [ "$REV1" != "$REV2" ]
then
  echo $REV1
  echo $REV2
  # Remove return character from string when running under cygwin
  REV1=`echo $REV1 | sed s/\r//`
  echo "#define REVISION \"$REV1\"" > include/us_revision.h
fi

