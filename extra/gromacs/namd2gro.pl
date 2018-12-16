#!/usr/bin/perl

# user config

$pbase = "/src/brookes_docs/jochen/parammd";
$pdbname = "step2_solvator.pdb";
$dcdname = "namd_equil.dcd";

# end user config

$notes = "usage: $0 project
project is the parammd project name, currently based at $pbase
convert pdb to gro
convert dcd to xtc
possibly substitute tip3 to sol?
strip ions?
then you can use xtctopdbs.pl
possible alternative
dcdcat and process separately

"
    ;
$notes .= "Current projects:\n" . `cd $pbase && ls -1d *`;

sub docmd {
    my $cmd = shift;
    print "$cmd\n";
    print `$cmd`;
    die "command returned bad status $!\n" if $?;
}

$project = shift || die $notes;
$d = "$pbase/$project";
die "directory $d does not exist\n" if !-d $d;
$pdbo = "$d/$pdbname";
$dcd = "$d/$dcdname";
die "pdb file $pdbo does not exist\n" if !-e $pdbo;
die "dcd file $dcd does not exist\n" if !-e $dcd;

$name = $project;
$name =~ s/^Run_//;

$pdb = "$name.pdb";

$gro = "$name.gro";
$xtc = "$name.xtc";

# gmx editconf -f file.pdb file.gro
# gmx pdb2gmx adds waterbox etc, don't use for this

docmd( "cp $pdbo $pdb" );
docmd( "gmx editconf -f $pdb -o $gro" );
docmd( "rm $pdb" ); # clean up
docmd( "sed -i 's/TIP3 /SOL  /g' $gro" );
docmd( "mdconvert -o $xtc $dcd" );

print "produced:
$gro
$xtc
";

