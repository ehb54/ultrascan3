#!/usr/bin/perl

$fshmprev = "shm_last";
$fshmthis  = "shm_this";

$notes = "usage: $0

diffs current ipcs -s -m & /tmp/qi*
against '$fshmprev'
    ";


`(ipcs -m && ipcs -s && ls /tmp/qi* 2> /dev/null) > $fshmthis`;

`touch $fshmprev` if !-e !-e $fshmprev;

print `diff $fshmprev $fshmthis`;

`mv $fshmthis $fshmprev`;


