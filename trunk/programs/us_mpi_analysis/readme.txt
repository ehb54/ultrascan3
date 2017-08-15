When building us_mpi_analysis, to link MyQql libraries

 - try add `mysql_config --libs` in linker invocation when error on mySql undefined references occur...

-----------------------------------------
E.g., in 'us_mpi_analysis.pro':

LIBS         += -lus_utils -L../../lib -L$$(QTDIR)/lib `mysql_config --libs`, 

with mysql_config --libs equals something like 
 -L/usr/lib64/mysql -lmysqlclient -lpthread -lm -ldl
