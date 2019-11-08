#!/usr/bin/perl

$notes = "usage: $0 list-of-base-dirs
e.g. $0 try10 try11 try12 try13
& this will setup the runs to produce ene_rmsd.xmgr
for these files
";

$hr = $ENV{ 'hr' } || die "$0: environment variable hr must be defined\n";
$ht = $ENV{ 'ht' } || die "$0: environment variable ht must be defined\n";

die "$0: directory $hr does not exist\n" if !-d $hr;

$HS = "structures_haddock-sorted.stat";
$CHS = "clusters_haddock-sorted.stat";

while ( $fb = shift )
{
    $d = "$hr$fb/run1/structures/it1";

    die "$d does not exist or is not a directory\n" if !-d $d;
    chdir $d || die "$0: can't change to directory $d\n";

    $f = $HS;
    die "$f does not exist\n" if !-e $f;
    $cmd = "$ht/make_ene-rmsd_graph.csh 3 2 $f";
    print "$cmd\n";
    print `$cmd`;
    $best = `head -2 $f | tail -1 | awk '{ print \$2 \"||\" \$1 }'`;
    chomp $best;
    print `pwd; ls -l ene_rmsd.xmgr`;
    $out .= "xmgrace $d/ene_rmsd.xmgr\n";
    $out2 .= "||$best\n";

#--------- add error bar from cluster out

    $f = $CHS;
    die "$f does not exist\n" if !-e $f;
    $cdata = `awk '{ print \$6 " " \$2 " "\$7 " " \$3 }' $CHS | sed '1d'`;
    $f = "ene_rmsd.xmgr";
    open IN, $f || die "$0: open $f error $!\n";
    @l = <IN>;
    close IN;
    $f = "cluster_rmsd.xmgr";
    open OUT, ">$f" ||  die "$0: open $f error $!\n";
    print OUT join '', @l;
    print OUT "\n\@ TYPE xydxdy\n";
    print OUT
'@ s1 symbol 1
@ s1 symbol size 0.5
@ s1 symbol fill 1
@ s1 symbol color 2
@ s1 symbol linewidth 2
@ s1 linestyle 0
' ;
    print OUT $cdata;
    print OUT "\&\n";
    close OUT;
    $outc .= "xmgrace $d/cluster_rmsd.xmgr\n";

#--------- switch to water directory

    $d = "$hr$fb/run1/structures/it1/water";

    die "$d does not exist or is not a directory\n" if !-d $d;
    chdir $d || die "$0: can't change to directory $d\n";

    $f = $HS;
    die "$f does not exist\n" if !-e $f;
    $cmd = "$ht/make_ene-rmsd_graph.csh 3 2 $f";
    print "$cmd\n";
    print `$cmd`;
    $best = `head -2 $f | tail -1 | awk '{ print \$2 \"||\" \$1 }'`;
    chomp $best;
    print `pwd; ls -l ene_rmsd.xmgr`;
    $outw .= "xmgrace $d/ene_rmsd.xmgr\n";
    $out2w .= "||$best\n";

#--------- add error bar from cluster out

    $f = $CHS;
    die "$f does not exist\n" if !-e $f;
    $cdata = `awk '{ print \$6 " " \$2 " "\$7 " " \$3 }' $CHS | sed '1d'`;
    $f = "ene_rmsd.xmgr";
    open IN, $f || die "$0: open $f error $!\n";
    @l = <IN>;
    close IN;
    $f = "cluster_rmsd.xmgr";
    open OUT, ">$f" ||  die "$0: open $f error $!\n";
    print OUT join '', @l;
    print OUT "\n\@ TYPE xydxdy\n";
    print OUT
'@ s1 symbol 1
@ s1 symbol size 0.5
@ s1 symbol fill 1
@ s1 symbol color 2
@ s1 symbol linewidth 2
@ s1 linestyle 0
' ;
    print OUT $cdata;
    print OUT "\&\n";
    close OUT;
    $outcw .= "xmgrace $d/cluster_rmsd.xmgr\n";

}

print '-'x60 . "\n$out";
print '-'x60 . "\n$out2";
print '-'x60 . "\n$outw";
print '-'x60 . "\n$out2w";
print '-'x60 . "\n$outc";
print '-'x60 . "\n$outcw";

