#!/usr/bin/perl

$notes = "usage: $0 tmpdir list-of-base-dirs
split each model up into b,c variants, each with only one of the side chains
";

$hr = $ENV{ 'hr' } || die "$0: environment variable hr must be defined\n";
$ht = $ENV{ 'ht' } || die "$0: environment variable ht must be defined\n";
$hb = $ENV{ 'hb' } || die "$0: environment variable hb must be defined\n";

die "$0: directory $hr does not exist\n" if !-d $hr;

$tdir   = shift || die $notes;
$cmds .= "mkdir -p $tdir\n";

@stages = ( "1", "1/water" );
@suffix = ( "1", "1w" );

$bd = `pwd`;
chomp $bd;

while ( $fb = shift )
{
    $b = "$hr$fb/run1/structures/it";

    for( $i = 0; $i < @stages; ++$i )
    {
        $d = "$b$stages[$i]";
        die "$d does not exist or is not a directory\n" if !-d $d;
        chdir $d || die "$0: can't change to directory $d\n";

        @pdbs = `ls -1 *pdb`;
        grep chomp, @pdbs;
        grep s/\.pdb$//, @pdbs;

        $newdir = "$tdir/$fb/$stages[$i]";
        $cmds .= "mkdir -p $newdir\n";
        for ( my $i = 0; $i < @pdbs; ++$i )
        {
            $cmds .= "$hb/splitchains.pl B C $d/$pdbs[$i] $newdir\n";
        }
    }
}

print $cmds;
