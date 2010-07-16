#!/usr/bin/perl -w

$US = $ENV{'ULTRASCAN'} || die "The environment variable ULTRASCAN must be set.  Terminating\n";

require "$US/etc/us_gridpipe_db.pl";

&dbopen();
print &dblist();
&dbclose();

