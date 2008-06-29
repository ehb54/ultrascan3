REV1=`svn info|grep Revision`
REV2=`cut -b 19-31 include/us_revision.h`
if [ "$REV1" != "$REV2" ]
then
		  echo $REV1
		  echo $REV2
echo "#define REVISION \"$REV1\"" > include/us_revision.h
fi

