#!/usr/bin/perl

# ----------- user configuration area
$db_login_database = "grid";
$db_login_host = "localhost";
$db_login_user = "httpd";
$db_login_password = "pw";
$DB_MAX_RECONNECT_TRIES = 10; # maximum # of times to retry opening the db before failing
# ----------- end user configuration area

use DBI;

$|=1;

## @fn $ dbopen()
# opens the database
# @param nothing
# @return nothing

sub dbopen {
    $dbh = DBI->connect("DBI:mysql:database=$db_login_database;host=$db_login_host",
			"$db_login_user", "$db_login_password",
			{ 
			    'PrintError' => 1,
			    'RaiseError' => 0
			}
	);
    $dbh->{mysql_auto_reconnect} = 1;
}

## @fn $ dbalive()
# checks to see if the connection is alive; if not, reopens it
# @param nothing
# @return nothing
sub dbalive {
    my $count = 0;
    while ( !$dbh->ping && $count < $MAX_TRIES) {
	dbopen();
	return if $dbh->ping;
	$count++;
	print STDERR "WARNING: mysql db connection could not be reopened, retrying $count of $MAX_TRIES\n";
	sleep $count * 10;
    }
}

## @fn $ dbclose()
# closed the database
# @param nothing
# @return nothing

sub dbclose {
    $dbh->disconnect();
}

## @fn $ dbread($key)
# returns $db{$key}
# @param key
# @return value

sub dbread {
    dbalive();
    my $sql = 
	"select message from gridjob where jid = \"$_[0]\";\n";
    my $sth;
    if ( !($sth = $dbh->prepare($sql)) ) {
	warn "dbread:: prepare failed\n";
	my $count = 0;
	while ( !($sth = $dbh->prepare($sql)) && $count < $MAX_TRIES) {
	    dbalive();
	    $count++;
	    warn "WARNING: dbread prepare failed, retrying $count of $MAX_TRIES\n";
	    sleep $count * 10;
	}
	if ( $count >= $MAX_TRIES ) {
	    warn "WARNING: dbread failed, no more retires\n";
	    return "";
	}
    }
    $sth->execute;
    if(!$sth->rows)
    {
	return "";
    }
    if($sth->rows > 1)
    {
	die "multiple rows for key \"$_[0]\"\n";
    }
    my @row = $sth->fetchrow_array;
    $sth->finish;
    if ( $debug_db ) {
	print "dbread row:\n";
	for(my $i = 0; $i < @row; $i++) {
	    print "$i: $row[$i]\n";
	}
    }
    return $row[0];
}

## @fn $ dbwrite($key, $val)
# set $db{$key} = $val
# @param key
# @return nothing

sub dbwrite {
    dbalive();
    my $sql = 
	"select message from gridjob where jid = \"$_[0]\";\n";
    my $sth;
    if ( !($sth = $dbh->prepare($sql)) ) {
	warn "dbwrite:: prepare failed\n";
	my $count = 0;
	while ( !($sth = $dbh->prepare($sql)) && $count < $MAX_TRIES) {
	    dbalive();
	    $count++;
	    warn "WARNING: dbwrite prepare failed, retrying $count of $MAX_TRIES\n";
	    sleep $count * 10;
	}
	if ( $count >= $MAX_TRIES ) {
	    warn "WARNING: dbwrite failed, no more retires\n";
	    return;
	}
    }
    $sth->execute;
    if(!$sth->rows)
    {
	$sth->finish;
	# insert record
        $sql = "insert into gridjob values " .
            "( \"$_[0]\", \"$_[1]\" );\n";
	$dbh->do($sql);
	return;
    }
    if($sth->rows > 1)
    {
	die "db_write: multiple rows for key \"$_[0]\"\n";
    }
    $sth->finish;
    $sql = "update gridjob set message=\"$_[1]\" where jid=\"$_[0]\"\n";
    $dbh->do($sql);
    return "";
}

## @fn $ dbdel($key)
# deletes $db{$key}
# @param key
# @return nothing

sub dbdel {
    dbalive();
    my $sql = 
	"delete from gridjob where jid = \"$_[0]\";\n";
    if ( !$dbh->do($sql) ) {
	warn "dbdel:: failed\n";
	my $count = 0;
	while ( !$dbh->do($sql) && $count < $MAX_TRIES) {
	    dbalive();
	    $count++;
	    warn "WARNING: dbdel do failed, retrying $count of $MAX_TRIES\n";
	    sleep $count * 10;
	}
	if ( $count >= $MAX_TRIES ) {
	    warn "WARNING: dbwrite failed, no more retires\n";
	    return;
	}
    }
    return "";
}

## @fn $ dbrocopy()
# returns a local copy of the database
# @param nothing
# @return copy of database

sub dbrocopy {
    dbalive();
    my %mdb;
    my $sql = 
	"select jid,message from gridjob;\n";
    my $sth;
    if ( !($sth = $dbh->prepare($sql)) ) {
	warn "dbrocopy:: prepare failed\n";
	my $count = 0;
	while ( !($sth = $dbh->prepare($sql)) && $count < $MAX_TRIES) {
	    dbalive();
	    $count++;
	    warn "WARNING: dbrocopy prepare failed, retrying $count of $MAX_TRIES\n";
	    sleep $count * 10;
	}
	if ( $count >= $MAX_TRIES ) {
	    warn "WARNING: dbrocopy failed, no more retires\n";
	    return %mdb;
	}
    }
    $sth->execute;
    while ( my @row = $sth->fetchrow_array ) {
	$mdb{$row[0]} = $row[1];
    }
    $sth->finish;
    return %mdb;
}

## @fn $ dblist()
# returns a string listing the database contents
# @param nothing
# @return string

sub dblist {
    my $out = '';
    my %mdb = dbrocopy();
    foreach my $i (keys %mdb) {
	$out .= "<$i>:<$mdb{$i}>\n";
    }
    return $out;
}

## @fn $ dbtest()
# a simple database test
# @param nothing
# @return nothing

sub dbtest {
    &dbopen();
    print "dbread js|123: <", &dbread("js|123") . ">\n";
    print "dbread js|124: <", &dbread("js|124") . ">\n";
    print "dbdel js|124: <", &dbdel("js|124") . ">\n";
    print "dbread js|124: <", &dbread("js|124") . ">\n";
    print "dbwrite js|124: <", &dbwrite("js|124","my 124 message") . ">\n";
    print "dbread js|124: <", &dbread("js|124") . ">\n";
    print "dbwrite js|124: <", &dbwrite("js|124","my 124 2nd message") . ">\n";
    print "dbread js|124: <", &dbread("js|124") . ">\n";
    print "dbdel js|124: <", &dbdel("js|124") . ">\n";
    print "dbread js|124: <", &dbread("js|124") . ">\n";
    
    print "dbwrite js|125: <", &dbwrite("js|125","my 125 message") . ">\n";
    print "dbwrite js|126: <", &dbwrite("js|126","my 126 message") . ">\n";
    print "dbwrite js|127: <", &dbwrite("js|127","my 127 message") . ">\n";
    print "dblist:\n";
    print &dblist();
    my %tdb = &dbrocopy();
    &dbclose();
    foreach my $i ( keys %tdb ) {
	print "dbrocopy: key <$i> value <$tdb{$i}>\n";
    }
    &dbopen();
    print "dbdel js|125: <", &dbdel("js|125") . ">\n";
    print "dbdel js|126: <", &dbdel("js|126") . ">\n";
    print "dbdel js|127: <", &dbdel("js|127") . ">\n";
    print "dblist:\n";
    print &dblist();
    %tdb = &dbrocopy();
    foreach my $i ( keys %tdb ) {
	print "dbrocopy: key <$i> value <$tdb{$i}>\n";
    }
    &dbclose();
}
