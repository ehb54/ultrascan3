#!/usr/bin/perl

$port = shift;
$message = shift || die "usage: $0 port message\n";
use IO::Socket;

$socket = IO::Socket::INET->new(Proto => "udp",
                                PeerAddr => "ultrascan.uthscsa.edu:$port")
    || die "Couldn't get a udp socket : $@\n";

$socket->send($message);
