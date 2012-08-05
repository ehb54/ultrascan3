REV1=`svn info|grep 'Changed Rev'`
REV2=`cut -f2 -d\" us_revision.h`
if [ "$REV1" != "$REV2" ]
then
  echo "REVISION WAS -- \"$REV2\""
  echo "REVISION NOW -- \"$REV1\""
  echo "#define REVISION \"$REV1\"" > us_revision.h
fi

