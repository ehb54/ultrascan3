#!/usr/bin/perl

#
# this program is setup to receive status updates from running grid jobs
# on a port and write updates to the gridpipe database
#
# key is <js|id>
#

$us = $ENV{'ULTRASCAN'} || die "The environment variable ULTRASCAN must be set.  Terminating\n";
# ----------- user configuration area
$dbname = "$us/etc/us_gridpipe_db";
# ----------- end user configuration area

use DB_File;
use FileHandle;

$|=1;

$dbfile = "$dbname.db";
$dblock = "$dbname.lock";
`touch $dblock` if ! -e $dblock;
die "couldn't create $dblock\n" if ! -e $dblock;

sub LOCK_EX { 2 }
sub LOCK_UN { 8 }

## @fn $ dbopen()
# opens the database
# @param nothing
# @return nothing

sub dbopen {
    open(DBLOCK, $dblock) || die "couldn't access dblock file $dblock\n";
    tie %db, "DB_File", $dbfile || die "can not open $dbfile: $!\n";
}

## @fn $ dbclose()
# closed the database
# @param nothing
# @return nothing

sub dbclose {
    untie %db;
    close(DBLOCK);
}

## @fn $ dblock()
# exclusively locks the database, failure to lock retries
# @param nothing
# @return nothing

sub dblock {
    if (!flock(DBLOCK, LOCK_EX)) {
	do {
	    print STDERR "$0: warning: error trying to lock file $dblock <\!>\n";
	    sleep 5;
	} while (!flock(DBLOCK, LOCK_EX));
    }
}

## @fn $ dbunlock()
# unlocks the database, failure to lock retries
# @param nothing
# @return nothing

sub dbunlock {
    if (!flock(DBLOCK, LOCK_UN)) {
	do {
	    print STDERR "$0: warning: error trying to unlock file $dblock <\!>\n";
	    sleep 5;
	} while (!flock(DBLOCK, LOCK_UN));
    }
}

## @fn $ dbwrite($key, $val)
# set $db{$key} = $val
# @param key
# @return value

sub dbwrite {
    dblock();
    $db{$_[0]} = $_[1];
    dbunlock();
}

## @fn $ dbwrite($key, $val)
# returns $db{$key}
# @param key
# @return nothing

sub dbread {
    dblock();
    my $return = $db{$_[0]};
    dbunlock();
    return $return;
}

## @fn $ dbdel($key)
# deletes $db{$key}
# @param key
# @return nothing

sub dbdel {
    dblock();
    delete $db{$_[0]};
    dbunlock();
}

## @fn $ dbdel($key)
# returns a list of the keys of the database
# @param key
# @return list of the keys

sub dbkeys {
    dblock();
    my @keys = keys %db;
    dbunlock();
    return sort @keys;
}

## @fn $ dbdel($key)
# returns a local copy of the database
# @param key
# @return nothing

sub dbrocopy {
    dblock();
    my %mdb = %db;
    dbunlock();
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
    dbopen();
    dbwrite("key1", "key arg 1");
    dbwrite("key2", "key arg 2");
    dbwrite("key3", "key arg 3");
    print dblist();
    print "----------\n";
    dbdel("key2");
    print dblist();
    dbclose();
}

$port = shift || die "usage: $0 port to listen on\n";

use IO::Socket;

$server = IO::Socket::INET->new(LocalPort => $port,
                                Proto => "udp")
    || die "Couldn't start a udp server on port $port : $@\n";

use Inline C;

reduce_perms(48,48);

print "awating UDP messages on port $port\n";

$MAXLEN = 1024;

while(1) {
    while ($server->recv($msg, $MAXLEN)) {
        print "message <$msg>\n";
        if($msg =~ /^js\|\d+\|.+$/) {
            ( $key , $data ) = $msg =~ /^(js\|\d+)\|(.+)$/;
            print "acceptable format message <$msg>:<$key>:<$data>\n";
            dbopen();
            dbwrite($key, $data);
            dbclose();
        }
    }
}
__END__
__C__
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

void reduce_perms(int uid, int gid) {
    printf("reducing privilege to run as apache\n");
    setuid((uid_t) uid);
    setgid((gid_t) gid);
}
