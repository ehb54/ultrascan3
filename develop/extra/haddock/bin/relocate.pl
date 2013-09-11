#!/usr/bin/perl

$notes =
"usage: $0 orgdir rundir run cpu iter0 iter1 iter2 
env HADDOCK & CNS_SOLVE must be defined
";

$haddock  = $ENV{'HADDOCK'  } || die $notes;
$cnssolve = $ENV{'CNS_SOLVE'} || die $notes;

$orgdir = shift || die $notes;
$rundir = shift || die $notes;
$run    = shift || die $notes;
$cpu    = shift || die $notes;
$iter0  = shift || die $notes;
$iter1  = shift || die $notes;
$iter2  = shift || die $notes;

$orgrun  = "${orgdir}/run${run}";
$runrun  = "${rundir}/run${run}";
$rundata = "${runrun}/data";

$rundir =~ s/\/$//;
( $newbase ) = $rundir =~ /^.*\/(\w*)$/;

$f = "${orgrun}/run.cns";

die "$0: $f not found\n" if !-e $f;

open IN, $f || die "$0: $f $!\n";
@l = <IN>;
close IN;

$l = join '', @l;

$l =~ /structures_0=(\d+);/;
$oiter0 = $1;

$l =~ /structures_1=(\d+);/;
$oiter1 = $1;

$l =~ /waterrefine=(\d+);/;
$oiter2 = $1;

$l =~ /cpunumber_1=(\d+);/;
$ocpu= $1;

print "org iters $oiter0 $oiter1 $oiter2 $ocpu\n";

$l =~ s/structures_0=(\d+);/structures_0=${iter0};/;
$l =~ s/structures_1=(\d+);/structures_1=${iter1};/;
$l =~ s/anastruc_1=(\d+);/anastruc_1=${iter1};/;
$l =~ s/waterrefine=(\d+);/waterrefine=${iter2};/;
$l =~ s/how_many_pdb=(\d+);/how_many_pdb=${iter2};/;
$l =~ s/ run_dir="\S+";/ run_dir="$runrun";/;
$l =~ s/ haddock_dir="\S+";/ haddock_dir="$haddock";/;
$l =~ s/ temptrash_dir="\S+";/ temptrash_dir="$runrun";/;
$l =~ s/ cpunumber_1=\d+;/ cpunumber_1=$cpu;/;
$l =~ s/ cns_exe_1="\S+";/ cns_exe_1="${cnssolve}\/intel-x86_64bit-linux\/bin\/cns";/;
$l =~ s/ fileroot="\S+";/ fileroot="$newbase";/;

$l =~ s/5\.3.pro/5.\4.pro/g;


if ( !-d $rundir )
{
    print "making $rundir\n";
    mkdir $rundir;
}

if ( !-d $runrun )
{
    print "making $runrun\n";
    mkdir $runrun;
}

if ( !-d $rundata )
{
    print "making $rundata\n";
    mkdir $rundata;
}

chdir $runrun || die "$0: $runrun $!\n";

$of = "$runrun/run.cns";

die "$0: $of exists, please remove first\n" if -e $of;

print ">$of\n";

open OUT, ">$of";
print OUT $l;
close OUT;

print `diff $f $of`;

# -------- new.html -----------

$f = "${orgrun}/data/new.html";

die "$0: $f not found\n" if !-e $f;

open IN, $f || die "$0: $f $!\n";
@l = <IN>;
close IN;

$l = join '', @l;

for ( $i = 0; $i < @l; $i++ )
{
    next if $l[ $i ] !~ /^(\S+)=(\S+)<BR>$/;
    $tag = $1;
    $val = $2;
    print "tag <$tag> val <$val>\n";
    $ref{ $tag } = $val;
}

die "run inconsistancy $run != " . $ref{ 'RUN_NUMBER' } . "\n" if $run != $ref{ 'RUN_NUMBER' };

for ( $i = 1; $i <= 6; ++$i )
{
    $tag = "PDB_FILE${i}";
    next if !$ref{ $tag };
    $val = $ref{ $tag };
    $targ = $val;
    $targ =~ s/^.*\///g;
    print "$val -> $targ\n";

    $dest = "${rundir}/$targ";
    die "$0: rm $dest and try again\n" if -e $dest;
    die "$0: $val does not exist\n" if !-e $val;
    `cp $val $dest`;
    $l =~ s/${tag}=$val/${tag}=$dest/;
}

$tag = 'PROJECT_DIR';
die "no $tag in $f\n" if !$ref{ $tag };
$l =~ s/${tag}=$ref{ $tag }/${tag}=${rundir}/;

$tag = 'HADDOCK_DIR';
die "no $tag in $f\n" if !$ref{ $tag };
$l =~ s/${tag}=$ref{ $tag }/${tag}=${haddock}/;

$tag = 'AMBIG_TBL';
die "no $tag in $f\n" if !$ref{ $tag };
$val = $ref{ $tag };
$targ = $val;
$targ =~ s/^.*\///g;
print "$val -> $targ\n";
$dest = "${rundir}/$targ";
die "$0: rm $dest and try again\n" if -e $dest;
die "$0: $val does not exist\n" if !-e $val;
`cp $val $dest`;
$l =~ s/$tag=$ref{ $tag }/$tag=$dest/;

$of = "${rundata}/new.html";

die "$0: $of exists, please remove first\n" if -e $of;

print ">$of\n";

open OUT, ">$of";
print OUT $l;
close OUT;

print `diff $f $of`;

# ----- protocols, toppar

`cp -r $haddock/protocols $runrun/`;
`cp -r $haddock/toppar $runrun/`;
`cp -r $haddock/tools $runrun/`;

# mk other dirs

@d = 
    (
       'data/dihedrals',
       'data/rdcs',
       'data/distances',
       'data/jcouplings',
       'data/dani',
       'data/sequence',
       'data/hbonds',
       'structures',
       'structures/it1',
       'structures/it1/analysis',
       'structures/it1/water',
       'structures/it1/water/analysis',
       'structures/it0',
       'begin',
       'packages'
    );

for ( $i = 0; $i < @d; ++$i )
{
    $tmpd = "${runrun}/$d[$i]";
    if ( !-d $tmpd )
    {
        print "making $tmpd\n";
        mkdir $tmpd;
    }
}    

# cp more stuff
@cp = `cd $orgrun; find . -name \"iteration.cns\" -or -name \"*.tbl\" -or -name \"*.py\"`;
@cp = grep chomp, @cp;
for ( $i = 0; $i < @cp; $i++ )
{
    `cp $orgrun/$cp[ $i ] $runrun/$cp[ $i ]`;
}


#pdbs to data/sequence
for ( $i = 1; $i <= 6; ++$i )
{
    $tag = "PDB_FILE${i}";
    next if !$ref{ $tag };
    $val = $ref{ $tag };
    $targ = $val;
    $targ =~ s/^.*\///g;
    print "$val -> $targ\n";

    $dest = "${rundata}/sequence/$targ";
    die "$0: rm $dest and try again\n" if -e $dest;
    die "$0: $val does not exist\n" if !-e $val;
    `cp $val $dest`;
    $l =~ s/${tag}=$val/${tag}=$dest/;
}

# the .list files

@lf = `cd ${orgrun}/data/sequence; ls -1 *.list`;
@lf = grep chomp, @lf;

for ( $i = 0; $i < @lf; $i++ )
{
    $f = "${orgrun}/data/sequence/$lf[ $i ]";
    $of = "${runrun}/data/sequence/$lf[ $i ]";
    open IN, $f || die "$0 $f $!\n";
    @l = <IN>;
    close IN;
    for ( $j = 0; $j < @l; ++$j )
    {
        $l[ $j ] =~ s/^.*\///g;
        $l[ $j ] = "\"$runrun/data/sequence/" . $l[ $j ];
    }
    print ">$of\n";
    open OUT, ">$of" || die "$0 $of $!\n";
    print OUT join '', @l;
    close OUT;
}
    

$readme =
"Make sure:
1. cns1 is symlinked correctly: e.g.
 ln -s $cnssolve/intel-x86_64bit-linux/bin/cns $runrun/protocols/cns1
---- if you have identical paths setup, this could be ok
2. make sure the .list files in data/sequence are correct
3. make sure run.cns: run_dir, cns_exe_1, temptrash_dir are correct
4. make sure data/new.html: PDB_FILE*, PROJECT_DIR, AMBIG_TBL, HADDOCK_DIR are correct
";

$f = "$rundir/README";
open OUT, ">$f" || die "$0: >$f $!\n";
print OUT $readme;
close OUT;
print `cat $f`;

