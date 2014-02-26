#!/usr/bin/perl

$notes = "usage: $0 dirs
in each dir, find all xyz files and compute pairwise rmsd
";

$bd = `pwd`;
chomp $bd;
print "bd is $bd\n";

while ( $d = shift )
{
    chdir $bd;
    die "$0 $d not a directory\n" if !-d $d;
    chdir $d;

    print "pwd is ";
    print `pwd`;

    @xyz = `ls *.xyz`;
    grep chomp, @xyz;
    @xyzb = @xyz;
    grep s/\.xyz$//, @xyzb;

    my $f = "cluster_res.txt";
    open OUT, ">$f" || die "$0 can't make $f $!\n";

    for ( my $i = 0; $i < @xyz - 1; ++$i )
    {
        for ( my $j = $i + 1; $j < @xyz; ++$j )
        {
            my $cmd = "python /src/rmsd-master/calculate_rmsd.py $xyz[ $i ] $xyz[ $j ]\n";
            print $cmd;
            my @res = `$cmd`;
            grep chomp, @res;
            my $kab = $res[ 1 ];
            $kab =~ s/^.*RMSD: //;
            my $fit = $res[ 2 ];
            $fit =~ s/^.*RMSD: //;
            print OUT "$xyzb[ $i ] $xyzb[ $j ] $kab $fit\n";
            print "$xyzb[ $i ] $xyzb[ $j ] $kab $fit\n";
        }
    }
    print ">$bd/$d/$f\n";
    close OUT;
}

