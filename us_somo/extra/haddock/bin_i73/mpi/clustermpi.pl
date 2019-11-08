#!/usr/bin/perl

$notes = "usage: $0 np dirs
make the calculate_rmsd.py script for each of np processors
in each dir, find all xyz files and compute pairwise rmsd
need original directory structure
";

$np = shift || die $notes;

$bd = `pwd`;

while ( $d = shift )
{
    die "$0 $d not a directory\n" if !-d $d;

    @xyz = `ls $d/*.xyz`;
    grep chomp, @xyz;
    @xyzb = @xyz;
    grep s/\.xyz$//, @xyzb;

    for ( my $i = 0; $i < @xyz - 1; ++$i )
    {
        for ( my $j = $i + 1; $j < @xyz; ++$j )
        {
            push @out,"./myrmsd.pl $xyz[ $i ] $xyz[ $j ]";
        }
    }
}

$per = 1 + int( @out / $np );

$j = 0;
print "total jobs " . scalar( @out ) . "\n";

for ( $i = 0; $i < $np; ++$i )
{
    $f = "job$i";
    print ">$f\n";
    open OUT, ">$f";
    for ( $k = 0; $k < $per && $j < @out; $k++, $j++ )
    {
        print OUT "$out[ $j ]\n";
    }
    close OUT;
}
