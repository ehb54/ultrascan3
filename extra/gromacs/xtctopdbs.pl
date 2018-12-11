#!/usr/bin/perl

$notes = "usage: $0 file-base-name nmdist
where 'name' is the name of the file
and 'nmdist' is the nm cutoff for SOLS
(further refinement can be done in us-somo)

the program will 
1. look for the .gro & .xtc files
2. extract all frames
3. remove SOLs greater than nmdist 
4. remove hydrogens & MW4 Tip4 dummy atoms
";

$traj = shift || die $notes;
$nmdist = shift || die $notes;

$traj =~ s/\.(xtc|gro)$//;

$trajgro = "$traj.gro";
$trajxtc = "$traj.xtc";

die "$0: $trajgro does not exist\n" if !-e $trajgro;
die "$0: $trajxtc does not exist\n" if !-e $trajxtc;

sub docmd {
    my $cmd = shift;
    print "$cmd\n";
    print `$cmd`;
    die "command returned bad status $!\n" if $?;
}

# mkdir for output

docmd( "rm -fr $traj; mkdir $traj" );

if ( !$quickrun ) {
    # extract all frames

    docmd( "echo 0 | gmx trjconv -s $trajgro -f $trajxtc -o $traj/${traj}.pdb -sep" );
} else {
    # extract frame range (funky range bits)

    docmd( "echo 0 | gmx trjconv -s $trajgro -f $trajxtc -b 5000 -e 15400 -o $traj/${traj}.pdb -sep" );
}

# get files created

@f = `ls $traj`;
grep chomp, @f;

for $f ( @f ) {
    docmd( "gmx select -s $traj/$f -select '(resname SOL and same residue as within '$nmdist' of group Protein) or group Protein' -on ${traj}_tmp.ndx" );
    docmd( "gmx editconf -n ${traj}_tmp.ndx -f $traj/$f -o ${traj}_tmp.pdb" );
    docmd( "mv ${traj}_tmp.pdb $traj/$f" );
    docmd( "rm ${traj}_tmp.ndx" );
    ( $base, $num, $ext ) = $f =~ /^(.*[^0-9])(\d+)(\.pdb)$/;
    $num = '0'x(5 - length( $num ) ) . $num;
    $fo = "${base}_F_${num}${ext}";
    open my $fh, "$traj/$f";
    @l = <$fh>;
    close $fh;
    my @lnew;

    $atomno = 0;
    $resno = 0;
    $lastresno = "";
    
    for $l ( @l ) {
        if ( $l !~ /^ATOM/ ) {
            push @lnew, $l;
            next;
        }

        if ( substr( $l, 13, 1 ) eq 'H' ||
             substr( $l, 13, 3 ) eq 'MW4' ) {
            next;
        }

        ++$atomno;
        $thisres = substr( $l, 21, 5 );
        if ( $lastresno ne $thisres ) {
            ++$resno;
        }

        $l =~ s/ HIE / HIS /;
        $l =~ s/ HSP / HIS /;
        $l =~ s/ OC1 LEU / O   LEU /;
        $l =~ s/ OC2 LEU / OXT LEU /;
        $l =~ s/ OT1 LEU / O   LEU /;
        $l =~ s/ OT2 LEU / OXT LEU /;

        push @lnew, sprintf( "ATOM  %5d%s%5d %s",
                             $atomno,
                             substr( $l, 11, 10 ),
                             $resno,
                             substr( $l, 27 )
            );

        $lastresno = $thisres;
    }
    open OUT, ">$traj/$f";
    print OUT join '', @lnew;
    close OUT;
    docmd( "mv $traj/$f $traj/$fo" ) if $f ne $fo;
}

