#!/usr/bin/perl

#
# this program is to test read & writes to the database
#

$US = $ENV{'ULTRASCAN'} || die "The environment variable ULTRASCAN must be set.  Terminating\n";

require "$US/etc/us_gridpipe_db.pl";

&dbtest();

dbopen();
print "after open dbstate ", $dbh->state, "\n";
print "after open ping ", $dbh->ping, "\n";
dbclose();
print "after close dbstate ", $dbh->state, "\n";
print "after close ping ", $dbh->ping, "\n";
