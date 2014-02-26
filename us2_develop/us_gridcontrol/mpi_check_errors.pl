#!/usr/bin/perl

$last = `ls -t /tmp/gc_mpi_*.0 | head -1`;
chomp $last;
$last =~ s/\.0$//;

print "last job was $last\n";

$error1 = `grep p4_error ${last}.1`;
$error2 = `grep node ${last}.2`;

print $error1 . $error2;
