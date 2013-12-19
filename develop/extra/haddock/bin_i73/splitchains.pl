#!/usr/bin/perl

$notes = "usage: $0 chain1 chain2 pdbfile targetdir
splits chain1 & chain2 of pdbfile as identified in segid
puts segid into 
";

$c1   = shift;
$c2   = shift;
$pdb  = shift;
$tdir = shift || die $notes;

$pdb =~ s/\.pdb$//;
$pb  = $pdb;
$pdb .= ".pdb";
$pb =~ s/^.*\///g;

print "pb is $pb\n";

$f1 = "$tdir/$pb-$c1.pdb";
$f2 = "$tdir/$pb-$c2.pdb";

die "$0: $pdb does not exist\n" if !-e $pdb;
die "$0: $tdir is not a directory\n" if !-d $tdir;

open IN, $pdb || die "$0: can not open $pdb $!\n";
@l = <IN>;
close IN;

for ( $i = 0; $i < @l; ++$i )
{
    $t = $l[ $i ];
    if ( $t !~ /^(ATOM|HETATM)/ )
    {
        $o1 .= $t;
        $o2 .= $t;
        next;
    }

    $seg = substr( $t, 72, 1 );
    $nl = substr( $t, 0, 21 ) . $seg . substr( $t, 22 );

    if ( $seg eq $c1 )
    {
        $o1 .= $nl;
        next;
    }
    if ( $seg eq $c2 )
    {
        $nl = substr( $t, 0, 21 ) . $c1 . substr( $t, 22 );
        $o2 .= $nl;
        next;
    }
    $o1 .= $nl;
    $o2 .= $nl;
}

print ">$f1\n";
open OUT, ">$f1" || die "$0 can not create $f1 $!\n";    
print OUT $o1;
close OUT;

print ">$f2\n";
open OUT, ">$f2" || die "$0 can not create $f2 $!\n";    
print OUT $o2;
close OUT;


