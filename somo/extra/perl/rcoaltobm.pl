#!/usr/bin/perl

$notes = "usage $0: beadsize psv filename
converts rcoal triangle file to bead model
";

$bs = shift;
$psv = shift || die $notes;
$file = shift || die $notes;

die "$0: error $file does not exist\n" if !-e $file;

open IN, $file || die "$0: error opening $file $!\n";

$of = "${file}.bead_model";

$l1 = <IN>;

( $vertices ) = $l1 =~ /^\s*(\d+)\s+/;

$out .= "$vertices $psv\n";
for ( $i = 0; $i < $vertices; ++$i )
{
    $_ = <IN>;
    ( $x, $y, $z ) = $_ =~ /^\s*(\S+)\s+(\S+)\s+(\S+)\s+/;

    $out .= "$x $y $z $bs 1 1 x 1\n";
}

close IN;

open OUT, ">$of" || die "$0: error opening output $of $!\n";
print "> $of\n";
print OUT $out;
close OUT;
