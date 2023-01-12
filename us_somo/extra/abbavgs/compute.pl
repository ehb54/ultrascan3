#!/usr/bin/perl

use File::Basename;
$scriptdir = dirname(__FILE__);

## user config

$etc         = "$scriptdir/../../etc";
$somoatom    = "$etc/somo.atom.new";
$somohybrid  = "$etc/somo.hybrid.new";
$somoresidue = "$etc/somo.residue.new";
$onlyatoms   = "^(C|N|O|S|P)\\d+";
#$onlyatoms   = "^.*\$";

## end user config

$notes = "usage: $0 

computes averages for ABB based upon $somoatom, $somoresidue & $somohybrid
only selecting atoms with hybridnames matching regex $onlyatoms

";

print $notes;

die "$somoatom does not exist\n"     if !-e $somoatom;
die "$somoatom is not readable\n"    if !-r $somoatom;
die "$somohybrid does not exist\n"   if !-e $somohybrid;
die "$somohybrid is not readable\n"  if !-r $somohybrid;
die "$somoresidue does not exist\n"  if !-e $somoresidue;
die "$somoresidue is not readable\n" if !-r $somoresidue;

## first load somohybrid for hybridization

open IN, $somohybrid || die "$somohybrid open error $!\n";
while ( my $l = <IN> ) {
    my ( $saxs_name, $name, $mw, $radius, $scat_len, $exch_prot, $num_elect ) = split /\s+/, $l;
    next if $name !~ /$onlyatoms/;
    my ( $net_charge ) = $name =~ /((?:\+|-)\d*)$/;
    my $protons = $num_elect + $net_charge;
    print "name $name protons $protons electrons $num_elect\n";
    $hybrid_to_electrons{$name} = $num_elect;
    $hybrid_to_protons  {$name} = $protons;
    $hybrid_to_scat_len {$name} = $scat_len;
}
close IN;


## scan somoatom for hybridization, mw, atomic radius
         
open IN, $somoatom || die "$somoatom open error $!\n";
while ( my $l = <IN> ) {
    my ( $atomname, $hybridname, $hybridmw, $hybridradius, $saxs_excl_vol ) = split /\s+/, $l;
    next if $hybridname !~ /$onlyatoms/;
    die "$hybridname missing : $somoatom : $l\n" if !exists $hybrid_to_electrons{$hybridname} || !exists $hybrid_to_protons{$hybridname};
    ++$tot{count};
    $tot{mw}         += $hybridmw;
    $tot{radius}     += $hybridradius;
    $tot{scat_len}   += $hybrid_to_scat_len{$hybridname};
    $tot{electrons}  += $hybrid_to_electrons{$hybridname};
    $tot{protons}    += $hybrid_to_protons{$hybridname};
}
close IN;

die "no usable lines found in $somoatom!\n" if !$tot{count};

$tot{mw}        /= $tot{count};
$tot{radius}    /= $tot{count};
$tot{scat_len}  /= $tot{count};
$tot{electrons} /= $tot{count};
$tot{protons}   /= $tot{count};

print sprintf( 
    "atoms from $somoatom considered: %d\n"   
    . "average mw:                    %.1f\n"
    . "average radius:                %.2f\n"
    . "average scat_len:              %.4f\n"
    . "average electrons:             %.1f\n"
    . "average protons:               %.1f\n"
    ,$tot{count}
    ,$tot{mw}
    ,$tot{radius}
    ,$tot{scat_len}
    ,$tot{electrons}
    ,$tot{protons}
    );

