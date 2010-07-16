#!/usr/bin/perl

#
# this program is setup to receive status updates from running grid jobs
# on a port and write updates to the gridpipe database
#
# key is <js|id>
#

$US = $ENV{'ULTRASCAN'} || die "The environment variable ULTRASCAN must be set.  Terminating\n";

require "$US/etc/us_gridpipe_db.pl";

$port = shift || die "usage: $0 port to listen on\n";

use IO::Socket;

$server = IO::Socket::INET->new(LocalPort => $port,
                                Proto => "udp")
    || die "Couldn't start a udp server on port $port : $@\n";

use Inline C;

reduce_perms(48,48);

print "awating UDP messages on port $port\n";

$MAXLEN = 1024;

&dbopen();

while(1) {
    while ($server->recv($msg, $MAXLEN)) {
        print "message <$msg>\n";
        if($msg =~ /^js\|\d+\|.+$/) {
            ( $key , $data ) = $msg =~ /^(js\|\d+)\|(.+)$/;
            print "acceptable format message <$msg>:<$key>:<$data>\n";
            dbwrite($key, $data);
	}
    }
}

&dbclose();

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
