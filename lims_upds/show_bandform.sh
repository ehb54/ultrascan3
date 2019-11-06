# Script to show band-forming centerpiece cell experiments in DBs

. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"
CMD="select cellID,abstractCenterpieceID,experimentID from cell where abstractCenterpieceID=7 OR abstractCenterpieceID=8;"

for DB in `cat ~us3/lims/database/dblist.txt`
do
  echo "== Searching $DB for band-forming centerpiece =="
  mysql ${LOGIN} -sNe "${CMD}" $DB
done

