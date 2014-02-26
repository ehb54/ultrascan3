#!/usr/bin/perl

$notes = "usage: $0 new-air-tbl\n
must be run from run root with one .tbl file
replaces all occurances with the new .tbl file
"
    ;

$new_tbl = shift || die $notes;
$f = $new_tbl;
die "$0: $f does not exist\n" if !-e $f;

@l = `ls -1 *tbl`;
die "$0: not exactly one .tbl file in current directory\n" if @l != 1;
$old = $l[ 0 ];
chomp $old;

# change in new.html

# -------- new.html -----------

@l = `find . -name "new.html"`;
die "$0: not exactly one new.html file in tree under current directory\n" if @l != 1;

$f = $l[ 0 ];
chomp $f;

die "$0: $f not found\n" if !-e $f;

open IN, $f || die "$0: $f $!\n";
@l = <IN>;
close IN;

grep s/AMBIG_TBL=\S+<BR>/AMBIG_TBL=$new_tbl<BR>/, @l;
`cp $f $f.old`;
open OUT, ">$f" || die "$0: >$f $!\n";
print OUT join '', @l;
close OUT;
print `diff $f.old $f`;

unlink $old;

@l = `find * -name "ambig.tbl"`;
grep chomp, @l;

print ">$new_tbl \n";
`cp $new_tbl .`;

map { print ">$_\n", `cp $new_tbl $_\n` } @l;
