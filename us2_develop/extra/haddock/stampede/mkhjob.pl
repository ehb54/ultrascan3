#!/usr/bin/perl

# - user config for different systems -

$sys_ppn = 16;

use Cwd;

$notes = 
    "usage: $0 source.tgz nodes queue HH:MM:SS\n";

$tgz   = shift || die $notes;
$nodes = shift || die $notes;
$queue = shift || die $notes;
$time  = shift || die $notes;

die "$0: time must be in HH:MM:SS format\n" if $time !~ /^\d\d:\d\d:\d\d/;

$queue = "normal" if $q =~ /^n/;
$queue = "development" if $q =~ /^d/;

die "$0 $tgz does not exist\n" if !-e $tgz;

@l = `tar ztf $tgz`;
@l = grep chomp, @l;

$tag = $l[ 0 ];
$tag =~ s/\/$//;

{
    my @lt = grep /$tag\/run(\d+)\//, @l;
    $run = $lt[ 0 ];
}

$run =~ s/^$tag\///;
$run =~ s/\/$//;

die "$0: no run name found in $tgz\n" if !length( $run );

print "found $tag/$run\n";

$dir = cwd();

$scratch = $ENV{'SCRATCH'} . "/haddock";
$home    = $ENV{'HOME'};
$hhome    = "$home/haddock";

chdir $scratch || die "$0: could not chdir $scratch\n";

die "$0: please:\nrm -fr $scratch/$tag\n and try again\n" if -e $tag;

$cmd = "tar zxf $tgz";
print "$cmd\n" if $debug;
print `$cmd`;

$cmd = "ln -sf $hhome/cns/cns_solve_1.3/intel-x86_64bit-linux/bin/cns $tag/$run/protocols/cns1\n";
print $cmd if $debug;
print `$cmd`;

# find run.cns

$cns = "$tag/$run/run.cns";
die "$0: no $cns found\n" if !-e $cns;

$f = $cns;

open IN, $f || die "$0: $f $!\n";
@l = <IN>;
close IN;

$l = join '', @l;

$l =~ /structures_0=(\d+);/;
$oiter0 = $1;

$summ .= "total it0 pdbs $oiter0\n";

$procs     = $sys_ppn * $nodes;
$core_used = $procs - 1;  # 1st node leaves one for master

$jpn = sprintf( "%.3f", $oiter0 / $core_used );
print "jobs per node $jpn\n";

# replace queue info

$qi = `$home/bin/mkrunq.pl $nodes $sys_ppn`;

for ( $i = 0; $i < @l; ++$i )
{
    push @ol, $l[ $i ];
    if ( $l[ $i ] =~ /====== parallel jobs ======/ )
    {
        push @ol, $qi;
        ++$i;
        for ( ; $l[ $i ] !~ /======================/ && $i < @l ; ++$i ) {};
        push @ol, $l[ $i ];
    }
}

$bf = "$f.bak";
$cmd = "mv $f $bf\n";
print $cmd if $debug;
print `$cmd`;

$ofiles .= "$bf\n";
$ofiles .= "$f\n";
open OUT, ">$f" || die "$0: >$f $!\n";
print OUT join '', @ol;
close OUT;

# $cmd = "diff $bf $f\n";
# print $cmd;
# print `$cmd`;

$infos .= "to check differences:\ndiff $bf $f\n" if $debug;

$warn .= "jobs per node $jpn < 1\n" if $jpn < 1;

# -------------------- make sbatch file

$f = "$tag/job";
$jf = $f;

die "$0: unexpected: $f exists!\n" if -e $f;


$out = "#!/bin/csh
#SBATCH -J h$tag$run       # job name
#SBATCH -o job.out         # output and error file name (\%j expands to jobID)
#SBATCH -n $procs          # total number of mpi tasks requested
#SBATCH -p $queue          # queue (partition) -- normal, development, etc.
#SBATCH -t $time           # run time (hh:mm:ss) - 1.5 hours
#SBATCH --mail-user=emre\@biochem.uthscsa.edu
#SBATCH --mail-type=begin  # email me when the job starts
#SBATCH --mail-type=end    # email me when the job finishes

cd $scratch/$tag/$run
source $home/.cshrc_user
haddock2.1
";

$ofiles .= "$f\n";
open OUT, ">$f" || die "$0: >$f $!\n";
print OUT $out;
close OUT;

print "\n";

print "Notes:\n$infos\n" if length( $infos );

print "Files created in $scratch:\n$ofiles\n";

print "WARNINGS:\n$warn\n" if length( $warn );

print "To submit:

cd $scratch/$tag
sbatch job

monitor with 'showq -u' or 'squeue -u tg457210'
cancel with 'scancel job-id'
";



