#!/usr/bin/perl

$notes = "usage: $0 new-air-tbl\n
must be run from run root with one .tbl file
replaces all occurances with the new .tbl file
"
    ;

$f = shift || die $notes;
die "$0: $f does not exist\n" if !-e $f;

@l = `ls -1 *tbl`;
die "$0: not exactly one .tbl file in current directory\n" if @l != 1;
$old = $l[ 0 ];
chomp $old;
unlink $old;

@l = `find * -name "ambig.tbl"`;
grep chomp, @l;

print ">$f\n";
`cp $f .`;

map { print ">$_\n", `cp $f $_\n` } @l;
