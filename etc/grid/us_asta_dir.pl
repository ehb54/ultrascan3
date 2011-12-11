#!/usr/bin/perl -w

use strict;
use DBI;

## mysql user database name
my $db ="exp_notification";
## mysql database user name
my $user = "exp_ogce";

## mysql database password
my $pass = "exp_pass";

## user hostname 
my $host="gf5.ucs.indiana.edu";

## user port 
my $db_port="5123";

my $experimentID = $ARGV[0];

#DB Connection
my $dbh = DBI->connect("DBI:mysql:$db;host=$host:port=$db_port", "$user", "$pass", {'RaiseError' => 1});

## SQL query
my $query = "SELECT ExperimentStatus FROM  NotificationInfo where ExperimentID = '$experimentID'";

my $sqlQuery  = $dbh->prepare("$query") or die "Can't prepare $query: $dbh->errstr\n";
$sqlQuery->execute(); 


my ($tables,$row);
($row)= $sqlQuery->fetchrow_array();
if (!$row) {
    print "$experimentID does not exist\n";
    exit;
}
$tables = $row;
# printing status
# print "$tables\n";

$sqlQuery->finish();

# if query successful, then retrieving values
if($tables eq 'COMPLETED' or $tables eq 'FAILED')
{
    my ($stdout, $workdir);
    my $iquery="select ExperimentMessage from NotificationDetails  where ExperimentID='$experimentID' and NotificationType = 'ApplicationAudit' and ExperimentMessage like '%( executable =%'";
    my $tth= $dbh->prepare("$iquery") ;
    $tth->execute();
    my $xml=$tth->fetchrow_array();
    $tth->finish();
    #my @xmlrows=split($xml,'\n');
    ($stdout)=$xml=~/\(\s*stdout\s*=\s*\"(.*?)\"\s*\)/i;
    ($workdir)=$xml=~/\(\s*directory\s*=\s*\"(.*?)\"\s*\)/i;
    # print "stdout=$stdout\nworkdir=$workdir\n";
    print "$workdir\n";
}
exit();

__END__

#!/usr/bin/perl 

# -w

# use strict;
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
my $host="gf5.ucs.indiana.edu";

my $experimentID = $ARGV[0] || die "usage $0 experimentID\n";

#DB Connection
my $dbh = DBI->connect("DBI:mysql:$db; host=$host", "$user", "$pass", {'RaiseError' => 1});

## SQL query

my $query;
$query = "SELECT ExperimentMessage FROM  NotificationDetails where ExperimentID = '$experimentID'";

print "$query\n" if $debug;

my $tth= $dbh->prepare("$query") ;
$tth->execute();

my $sqlQuery  = $dbh->prepare("$query") or die "Can't prepare $query: $dbh->errstr\n";
$sqlQuery->execute(); 

my @row;
my $outputDir = "";
while ((@row)= $sqlQuery->fetchrow_array()) {
    print "row " , $row++, "\n" if $debug;

    ( $outputDir ) = $row[0] =~ /\( \"outputData\" \"(.*outputData)\" \)/;
    if($outputDir) {
        $outputDir =~ s/\/outputData//;
        print "$outputDir\n";
        break;
    }
}

$sqlQuery->finish();

