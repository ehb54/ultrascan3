# One time script to add entries to newus3.metadata
#  by creating the DMGA method

echo "Updating newus3.metadata"
mysql -u root -p newus3 < ./alter_metadata.sql

