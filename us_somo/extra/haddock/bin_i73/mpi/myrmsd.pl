#!/usr/bin/perl

$f1 = shift;
$f2 = shift;
$cmd = "python calculate_rmsd.py $f1 $f2";
my @res = `$cmd`;
grep chomp, @res;
my $kab = $res[ 1 ];
$kab =~ s/^.*RMSD: //;
my $fit = $res[ 2 ];
$fit =~ s/^.*RMSD: //;
$f1 =~ s/\.xyz$//;
$f1 =~ s/^.*\///;
$f2 =~ s/\.xyz$//;
$f2 =~ s/^.*\///;
print "$f1 $f2 $kab $fit\n";
