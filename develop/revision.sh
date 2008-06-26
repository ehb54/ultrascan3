REV=`svn info|grep Revision`
echo "#define REVISION \"$REV\"" > include/us_revision.h

