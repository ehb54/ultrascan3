#!/usr/bin/perl

$notes = "usage: $0 targetdir #it0 #it1 #it1/water list-of-base-dirs
e.g. $0 /haddock/tmp/x 10 15 25 try10 try11 try12 try13
will make list of best 10 in it0, etc.
commands will be output to copy the files and
a text file will be written as a job input file
for c2checks
";

$hr = $ENV{ 'hr' } || die "$0: environment variable hr must be defined\n";
$ht = $ENV{ 'ht' } || die "$0: environment variable ht must be defined\n";

die "$0: directory $hr does not exist\n" if !-d $hr;

$tdir   = shift || die $notes;
@n      = ( shift, shift, shift );
@stages = ( "0", "1", "1/water" );
@suffix = ( "0", "1", "1w" );

$cmds .= "mkdir -p $tdir\n";

$out = "# us_saxs_cmds_t iq controlfile
# blank lines ok, format token <params>

ResidueFile     somo.residue
AtomFile        somo.atom
HybridFile      somo.hybrid
SaxsFile        somo.saxs_atoms
FiveTermGaussian

Saxs
IqMethod        h3a
FdBinSize       0.5
FdModulation    0.23
HyPoints        15
CrysolHarm      15
CrysolGrid      17
CrysolChs       0.03
WaterEDensity   0.334
AsaHydrateThresh      10
AsaThreshPct          50
AsaHydrateProbeRadius 1.4
AsaStep               1
AsaCalculation        1
StartQ          0.001
EndQ            0.6
DeltaQ          0.0146
Output          ssaxs
c2checkcaonly
";

$bd = `pwd`;
chomp $bd;

while ( $fb = shift )
{
    $b = "$hr$fb/run1/structures/it";

    for( $i = 0; $i <  @stages; ++$i )
    {
        $d = "$b$stages[$i]";
        die "$d does not exist or is not a directory\n" if !-d $d;
        chdir $d || die "$0: can't change to directory $d\n";

        @pdbs = `ls -1 *pdb`;
        grep chomp, @pdbs;
        grep s/\.pdb$//, @pdbs;

        undef %used;
        die "not enough pdbs available $d $n[$i]\n" if @pdbs < 2 * $n[ $i ];
        for( $j = 0; $j < $n[ $i ]; ++$j )
        {
            do {
                $p = int( rand( @pdbs ) );
            } while( $used{ $p }++ );
            $target = "$pdbs[$p]-$suffix[$i].pdb";
            $cmds .= "cp $d/$pdbs[$p].pdb $tdir/$target\nsegidtochainid.pl $tdir/$target\n";
            $out .= "inputfile $target\nc2check B C\n";
        }
    }            
}

$out .= "TgzOutput       job_p1_out.tgz\n";


$f = "$bd/mkc2run.cmds";
open OUT, ">$f" || die "$0 $f $!\n";
print OUT $cmds;
close OUT;
print ">$f\n";

$f = "$bd/mkc2run.job";
open OUT, ">$f" || die "$0 $f $!\n";
print OUT $out;
close OUT;
print ">$f\n";


