# One time script to update all active US3 databases
#  by changing the Glycerol% bufferComponents entry
#  for proper range and range unit

LOGIN="-u root -p`cat ~/.sec/.pwsq`"
CMD="update bufferComponent set density='0.99823 0.030859 0.2249 6.333e-3 2.505e-5 4.151e-5 0-32 %weight/weight' where bufferComponentID=25"
##CMD="update bufferComponent set density='0.99824 0.0 2.08235 0.14172 -0.26453 0.54073 0-13.694 M' where bufferComponentID=26"
##CMD="select description,density from bufferComponent where description like '%lycero%'"
##for db in `cat dblist.txt`
##for db in uslims3_devel
##for db in uslims3_cauma3
for db in `cat dblist.txt`

do
 echo "Updating $db"
 mysql $LOGIN $db -sNe "${CMD}"
done

