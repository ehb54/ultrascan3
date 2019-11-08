#!/usr/bin/perl

$notes = "usage: $0 queues ppn\n";

$q = shift || die $notes;
$p = shift || die $notes;

$uq = $q;
$uq = 10 if $uq < 10;

print "\n{+ table: rows=$uq";

for ( $i = 1; $i <= $uq; ++$i )
{
    print " \"$i\"";
}

print '
 cols=3 "queue command" "cns executable" "number of jobs" +}
';


for ( $i = 1; $i <= $uq; ++$i )
{
    if ( $i <= $q )
    {
        if ( $i == 1 && $p == 16 )
        {
            $up = 15;
        } else {
            $up = $p;
        }

        print "\n{===>} queue_$i=\"/home1/00451/tg457210/bin/runonnode.pl $i\";
{===>} cns_exe_$i=\"/home1/00451/tg457210/haddock/cns/cns_solve_1.3/intel-x86_64bit-linux/bin/cns\";
{===>} cpunumber_$i=$up;
";
    } else {
        print "\n{===>} queue_$i=\"\";
{===>} cns_exe_$i=\"\";
{===>} cpunumber_$i=0;
";
    }        
}

print "\n";


