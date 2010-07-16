#!/usr/bin/perl -w

$US = $ENV{'ULTRASCAN'} || die "The environment variable ULTRASCAN must be set.  Terminating\n";

require "$US/etc/us_gridpipe_db.pl";

$arg = shift || die "usage: $0 epr-file-name\n";

dbopen();
$result = dbread("resubmit|$arg");
dbclose();
$result = 0 if !$result;
print "$result\n";
