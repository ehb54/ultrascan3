#!/usr/bin/perl

$hr = $ENV{ 'hr' } || die "$0: environment variable hr must be defined\n";
$ht = $ENV{ 'ht' } || die "$0: environment variable ht must be defined\n";

while ( $fb = shift )
{
    $cmd = "pushd
cd $hr$fb/run1/structures/it1
zip /haddock/tmp/$fb.zip structures_haddock-sorted.stat ${fb}_*.pdb
cd water
zip /haddock/tmp/${fb}w.zip structures_haddock-sorted.stat ${fb}_*w.pdb
popd
"
;
    print $cmd;
}
