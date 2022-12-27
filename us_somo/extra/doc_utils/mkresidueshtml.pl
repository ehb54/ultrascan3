#!/usr/bin/perl

use File::Basename;
$scriptdir     = dirname(__FILE__);
$docsdir       = "../../somo/doc/manual/somo";
$etcdir        = "../../etc";
$somoresidues  = "$etcdir/somo.residue.new";
$residueshtml  = "$docsdir/residues.txt";

$notes = "usage: $0

processes $$somoresidues to make $$residueshtml in a human readable format

";

die $notes if @ARGV;

die "$docsdir does not exist\n" if !-d $docsdir;
die "$etcdir does not exist\n" if !-d $etcdir;
die "$somoresidues does not exist\n" if !-e $somoresidues;
die "$somoresidues is not readable\n" if !-r $somoresidues;

@l = `cat $somoresidues`;
die "$somoresidues is apparently empty\n" if !@l;
print sprintf( "$somoresidues line count %d\n", scalar @l );

sub nextline {
    die "incomplete residue desc at line $pos\n" if !@l;
    shift @l;
}

%skips = (
    "PBR-NO-OXT" => 1
    ,"PBR-OXT"   => 1
    ,"OXT"       => 1
    ,"OXT-P"     => 1
    ,"N1"        => 1
    ,"N1-"       => 1
    ,"PBR-N"     => 1
    ,"PBR-G"     => 1
    ,"PBR-P"     => 1
    ,"NH2"       => 1
);    


while ( $l = shift @l ) {
    ++$pos;
    my $resdesc = $l;
    $resdesc =~ s/alternate names/alternate atom names/;
    $l = nextline();
    my ( $resname, $atoms, $beads ) = $l =~ /^(\S+)\s+\S+\s+\S+\s+\S+\s+(\d+)\s+(\d+)\s+/;
    print "$resname has $atoms atoms and $beads beads\n";

    for ( my $i = 0; $i < $atoms + $beads; ++$i ) {
        shift @l;
    }

    if ( !$skips{$resname} ) {
        $summary{$resname} .= sprintf( "%-4s\t\t", $resname ) if $summary{$resname};
        $summary{$resname} .= $resdesc;
    }
}

$out =
    "Residue name    Description\n" .
    "------------    ----------------------------------------------------------------------\n"
    ;

for my $k ( sort { $a cmp $b } keys %summary ) {
    $out .= sprintf( "%-4s\t\t%s", $k, $summary{$k} );
}

open OUT, ">$residueshtml";
print OUT $out;
close OUT;
print "$residueshtml created\n";


    
