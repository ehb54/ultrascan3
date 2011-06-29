#!/usr/bin/perl

$min_np = 2;  # minimum no of procs
$max_np = 128; # max
$gran = 1;    

#$debug++;
$target_util = shift;
$ti_noise_util = shift;
die "usage us_parse_gc.pl util ti_noise_util gcfile\n" if !$ti_noise_util || $ti_noise_util <= 0;

@lines = <>;
@lines=grep(chomp,@lines);


$exps = $lines[5];
$sa2dbase = 6 + $exps * 5;

for($i = 0; $i < @lines - $sa2dbase; $i++) {
    print "sa2d line $i <$lines[$i + $sa2dbase]>\n" if $debug;
}

if($lines[0] =~ /^GA$/) {
    print "GA using demes + 1\n" if $debug;
    $demes = $sa2dbase;
    $np = $lines[$demes] + 1;
    $np = $max_np if $np > $max_np;
    print "$np\n";
    exit;
}


$ffres = 2 + $sa2dbase;
$sres = 5 + $sa2dbase;
$gridrep = 7 + $sa2dbase;
$tinoise = 11 + $sa2dbase;

$m = $lines[$ffres] * $lines[$sres];
$k = $lines[$gridrep] * $lines[$gridrep];
$k++ if $k <= 1;
$n = $m * $k;
$target_util = $ti_noise_util if $lines[$tinoise] > 0;
print "using utilization $target_util\n" if $debug;

print "n $n k $k n $n\n" if $debug;

for($p = $min_np; $p <= $max_np; $p += $gran) {
    print "\t$p\t" if $debug;
}
print "\n" if $debug;

    
for($x = .01; $x < .1; $x += .02) {
    print "x $x:" if $debug;
    $r = int(log($k)/(-log($x)) + .9999999);
    for($p = $min_np; $p <= $max_np; $p += $gran) {
	$l = int(log($k/$p)/(-log($x)));
	$speedup = (($k)/(1-$x))/(($k/$p)/(1-$x) +  $r - $l);
	$util = (($k/$p)/(1-$x))/(($k/$p)/(1-$x) +  $r - $l);
	print sprintf("\t%.4f(%d,%d,%.1f)",$util,$r,$l,$speedup) if $debug;
    }
    print "\n" if $debug;
}

$x = .03;
$r = int(log($k)/(-log($x)) + .9999999);
@utils = (.1, .2, .3, .4, .50, .55, .60, .65, .70, .75, .80, .85, .90, .95);
foreach $i (@utils) {
    $pmax{$i} = 1;
    $pmaxs{$i} = 1;
}

for($p = $min_np; $p <= $max_np; $p += $gran) {
    $l = int(log($k/$p)/(-log($x)));
    $speedup = (($k)/(1-$x))/(($k/$p)/(1-$x) +  $r - $l);
    $util = (($k/$p)/(1-$x))/(($k/$p)/(1-$x) +  $r - $l);
    foreach $i (@utils) {
	if($util >= $i) {
	    $pmax{$i} = $p;
	    $pmaxs{$i} = sprintf("%.2f",$speedup);
	}
    }
    if($util >= $target_util) {
	$pmax{'target'} = $p;
	$pmaxs{'target'} = sprintf("%.2f",$speedup);
    }
}

foreach $i (@utils) {
    print "recommended p for utilization $i $pmax{$i}\t speedup $pmaxs{$i}\n" if $debug;
#    print "$i $pmax{$i} $pmaxs{$i}\n" if $debug;
}

if($debug2) {
    $r = 4;
    $l = 1;
    $p = 2;
    $k = 30 * $p;
    $x = .1;
    $util = (($k/$p)/(1-$x))/(($k/$p)/(1-$x) +  $r - $l);
    print "test util $util\n";
}

$pmax{'target'} = $min_np if $pmax{'target'} < $min_np;
$pmax{'target'} = $max_np if $pmax{'target'} > $max_np;

print "$pmax{'target'}\n";
