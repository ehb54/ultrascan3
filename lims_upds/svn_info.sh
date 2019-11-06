# One time script to list 'svn info' output for
#  production LIMS,GridCtl directories

INSTSAMP=uslims3_AUCSOL
US3HOME=`(cd ~us3;pwd)`

DIRS="${US3HOME}/lims/bin \
 /srv/www/htdocs/uslims3 \
 /srv/www/htdocs/uslims3/${INSTSAMP} \
 /srv/www/htdocs/uslims3/uslims3_newlims \
 /srv/www/htdocs/common \
 /srv/www/htdocs/common/class \
 /srv/www/htdocs/common/class_local \
 "

for DIR in $DIRS; do
  svn info ${DIR}|egrep '^Path:|URL:'
done

