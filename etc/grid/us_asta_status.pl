#!/usr/bin/perl -w

# jdbc:mysql://gw33.quarry.iu.teragrid.org:3306/exp_notification?user=exp_ogce&password=exp_pass

use strict;
use DBI;

my $debug;
# $debug++;

# print "Content-type: text/html\n\n";

## mysql user database name
my $db ="exp_notification";
## mysql database user name
my $user = "exp_ogce";

## mysql database password
my $pass = "exp_pass";

## user hostname : This should be "localhost" but it can be diffrent too
my $host="gw33.quarry.iu.teragrid.org";

## user hostname : This should be "localhost" but it can be diffrent too
my $db_port="3306";

my $experimentID = $ARGV[0];

#DB Connection
my $dbh = DBI->connect("DBI:mysql:$db;host=$host:port=$db_port", "$user", "$pass", {'RaiseError' => 1});

## SQL query

my $query;
if ( $experimentID ) {
    $query = "SELECT ExperimentStatus FROM  NotificationInfo where ExperimentID = '$experimentID'";
} else {
    $query = "SELECT ExperimentID, ExperimentStatus FROM NotificationInfo";
}

print "$query\n" if $debug;

my $tth= $dbh->prepare("$query") ;
$tth->execute();

my $sqlQuery  = $dbh->prepare("$query") or die "Can't prepare $query: $dbh->errstr\n";
$sqlQuery->execute(); 

my @row;
while ((@row)= $sqlQuery->fetchrow_array()) {
    my $tables = $row[0];
    $tables .= " $row[1]" if !$experimentID;
    print "$tables\n";
}

$sqlQuery->finish();

__END__

if ( $experimentID ) 
{
    if($tables eq 'COMPLETED' or $tables eq 'FAILED')
    {
	my ($stdout, $workdir);
	my $iquery="select ExperimentMessage from NotificationDetails  where ExperimentID='$experimentID' and NotificationType = 'ComputationDuration' and ExperimentMessage like '%<rsl:job%'";
	my $tth= $dbh->prepare("$iquery") ;
	$tth->execute();
	my $xml=$tth->fetchrow_array();
	$tth->finish();
	#my @xmlrows=split($xml,'\n');

        ($workdir)=$xml=~/\<rsl:directory xsi:type=\"xsd:string\"\>(.*)\<\/rsl:directory\>/;
	($stdout)=$xml=~/\<rsl:stdout xsi:type=\"xsd:string\"\>(.*)\<\/rsl:stdout\>/;
	print "stdout=$stdout\nworkdir=$workdir\n";
    }
}

exit();
