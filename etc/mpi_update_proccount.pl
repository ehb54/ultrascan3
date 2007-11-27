#!/usr/bin/perl

$cmd = q[grep -v '^#' $ULTRASCAN/etc/mpi_sa2d_machines  | awk -F= '{ x+=$2 } END { print x }' > $ULTRASCAN/etc/mpi_sa2d_proccount];
print `$cmd\n`;
$cmd = q[grep -v '^#' $ULTRASCAN/etc/mpi_ga_machines  | awk -F= '{ x+=$2 } END { print x }' > $ULTRASCAN/etc/mpi_ga_proccount];
print `$cmd\n`;

use Net::Telnet;
$t = new Net::Telnet(Timeout=>.1, Errmode=>'return', Port=>22);

$US = $ENV{'ULTRASCAN'};
open(SA2D, "awk '{ print \$1 }' $US/etc/mpi_sa2d_machines |");
open(GA, "awk '{ print \$1 }' $US/etc/mpi_ga_machines |");

$first = 1;

sub checkthis {
    chomp;
    return if /^#/;
    if(!$machines{$_}) {
	$machines{$_}++;
	print "checking $_ : ";
	$t->open($_);
	if(length($t->errmsg)) {
	    print "ERROR: can not connect\n";
	    return;
	}
	    
	$cmd = "su apache -c 'ssh $_ ldd $US/bin64/us_fe_nnls_t_mpi 2>&1 | grep -i \"not found
no such file\"'\n";
#	print "$cmd";
	$result = `$cmd`;
#	print "$result";
	if($result =~ /(not found|No such file)/) {
	    print "ERROR: $result";
	    return;
	}
	$cmd = "su apache -c 'ssh $_ which mpirun'";
	$result = `$cmd`;
	chomp $result;
	if($first) {
	    $lastres = $result;
	    $baseres = $_;
	    undef $first;
	} else {
	    if($lastres ne $result) {
		print "ERROR : differing versions of mpirun $result != $lastres (from $baseres)\n";
		return;
	    } 	
	}	
	print "ok\n";
    }
}

while(<SA2D>) {
    &checkthis($_);
}

while(<GA>) {
    &checkthis($_);
}

    
$cmd = q[echo '2DSA ' `cat $ULTRASCAN/etc/mpi_sa2d_proccount`];
print `$cmd\n`;
$cmd = q[echo 'GA   ' `cat $ULTRASCAN/etc/mpi_ga_proccount`];
print `$cmd\n`;
