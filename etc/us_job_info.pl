#!/usr/bin/perl

$US = $ENV{'ULTRASCAN'} || die "environ variable ULTRASCAN must be set\n";
# $debug++;

$| = 1;

sub pid_info {
    my $pid = $_[0];
    my $file = "/proc/$pid/cmdline";
    my $i;

    open(IN, $file) || die "file open error: $file $!\n";
    my $l = <IN>;
    close IN;
    print "l = <$l>\n" if $debug;
    @l = split /\s/, $l;
    for ( $i = 0; $i < @l; $i++ )
    {
	print "pos $i <$l[$i]>\n" if $debug;
    }
    $gcfile = $l[4];
    $email = $l[6];
    $timestamp = $l[7];
    $dir = $l[8];
    $sys = $l[11];
    $out1 = $l[13];
    $out2 = $l[14];
    # print out basic info
    # 
    $epr = "$dir/$timestamp/us_tigre_epr${timestamp}.xml";
    $cmd = "$US/etc/us_gridpipe_my_jobid.pl $epr 2> /dev/null\n";
    print $cmd if $debug;
    $jid = `$cmd`;
    chomp $jid;
    print "$jid $email $sys $out1 ";
    print "$epr ", (-e $epr ? "exists" : "does not exist"), " status:";
    $cmd = "sudo su - apache -c 'globusrun-ws -status -job-epr-file $epr'";
    print "$cmd" if $debug;
    my $res = `$cmd | grep state`;
    print $res;
}

sub list_tigre_jobs {
    my @l;
    my  $i;
    @l = `ps -ef | grep us_tigre_job | grep -v grep | grep 'nohup perl' | awk '{ print \$2 }'`;
    grep chomp, @l;
    for ( $i = 0; $i < @l; $i++ ) 
    {
	print "$i $l[$i]\n" if $debug || $_[0];
    }
    @l;
}


$help = "usage: $0 {-p pid|-l|-a|-d}
 -p pid      \tprint info for pid #
 -l          \tlist all pid's
 -a          \tprint info for all pids
";

while ( $ARGV[0] =~ /^-/ ) {
    $_ = shift;
    last if /^--/;
    $any = 0;
    if ( /d/ ) 
    {
	$debug++;
	print "debug mode\n" if $debug;
	$any++;
    }
    if ( /p/ ) 
    {
	&pid_info(shift);
	$any++;
    }
    if ( /a/ ) 
    {
	@j = &list_tigre_jobs();
	for ( $i = 0; $i < @j; $i++) {
	    &pid_info($j[$i]);	    
	}
	$any++;
    }
    if ( /l/ ) 
    {
	&list_tigre_jobs(1);
	$any++;
    }
    if ( /h/ ) 
    {
	print $help;
	exit;
    }
    if ( !$any ) 
    {
	die "I don't recognize this switch: $_\\n";
	print $help;
    }
}
