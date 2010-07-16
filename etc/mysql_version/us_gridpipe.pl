#!/usr/bin/perl

$us = $ENV{'ULTRASCAN'} || die "The environment variable ULTRASCAN must be set.  Terminating\n";
# ----------- user configuration area
$debug++;
#$debugdb++;
$debug_cmds++;
$globustimeout = 15; # seconds to wait for globusrun-ws commands to succeed or timeout
$statusupdate = 10;  # seconds to wait for status update
$globus_statusupdate = 60;  # seconds to wait for status update
$killupdate = 60;  # seconds to wait for rekilling
$maxkillreps = 5;  # number of times to try to kill before giving up
$logfiledir = "/opt/tmp";    # reset to /opt/tmp for godzilla
# ----------- end user configuration area

use FileHandle;
# use IO::Handle;

$us = $ENV{'ULTRASCAN'} || die "The environment variable ULTRASCAN must be set.  Terminating\n";
open(SYS_LOCK, "$us/etc/us_gridpipe.lock") || die "$0: lockfile $us/etc/us_gridpipe.lock error: $!.  Terminating\n";
flock(SYS_LOCK, 6) || die "$0: lockfile $us/etc/us_gridpipe.lock is already in use ($!).  Terminating\n";

$seqlock = "$us/etc/us_gridpipe_seq.lock";
`touch $seqlock` if ! -e $seqlock;

require "$us/etc/us_gcfields.pl";

$US = $us;
require "$US/etc/us_gridpipe_db.pl";

$status_file = "$us/etc/mpi_queue_status";
$status_detail_file = "$us/etc/queue_status_detail";

$pipe = "$us/etc/us_gridpipe";

$ENV{'DISPLAY'}='';

## @fn $ dbnxtseq()
# returns a unique increasing #
# @param nothing
# @return a unique sequence #
sub LOCK_EX { 2 }
sub LOCK_UN { 8 }

sub dbnxtseq() {
    open(SEQLOCK, $seqlock);
    flock(SEQLOCK, LOCK_EX) || print STDERR "$0: Warning can not flock $seqlock proceeding (possible status file mangling!)\n";
    
    print "dbnxtseq\n" if $debugdb;
    my $seq = dbread("next_seq");
    print "seq was $seq\n" if $debugdb;
    $seq++;
    dbwrite("next_seq",$seq);
    flock(SEQLOCK, LOCK_UN);
    close SEQLOCK;
    print "seq is $seq $db{'next_seq'}\n" if $debugdb;
    print DBSEQ "dbnxtseq: $seq\n" if $debug_cmds;
    $seq;
}

## @fn $ timedexec($timeout, $command)
# runs a shell command for timeout seconds and if it has not completed, it returns with 'non-responsive'
# @param the timeout in seconds and the command
# @return the results from the command or 'non-responsive'

sub timedexec {

    my $return;
    my $pid = open(PIPE, "$_[1] |") or die $!;

    eval {
	local $SIG{ALRM} = sub { die "timeout" };
	alarm($_[0]);
	my @results = <PIPE>;
	$return = join '', @results;
	close(PIPE);
	alarm(0);
    };

    if($@) {
	alarm(0);
	if($@ =~ /timeout/) {
	    $return = "Current job state: Non-responsive";
	    kill 9, $pid;
	    close(PIPE); 
	    print "timed out\n";
	    # timed out;
	} else {
	    print "success\n";
	    die;
	}
    }
    $return;
}

sub startjob {
    print STDERR "$0: start job $queue[0]\n" if $debug;
    my $started = `date '+%D %T'`; chomp $started;
    my $child;
    if(!($child = fork)) {
	print PROCS "$$ startjob $queue[0]\n" if $debug_cmds;
	print STDERR "$0: child started job\n" if $debug;
	`$queue[0]`;
	exit;
    }
    print STDERR "$0: child pid is $child\n" if $debug;
}

sub startjob_gc {
    print STDERR "$0: start gc process $_[0]\n" if $debug;
    my $child;
    if(!($child = fork)) {
	print PROCS "$$ startjob_gc $_[0]\n" if $debug_cmds;
	print STDERR "$0: child started gc process job\n" if $debug;
	`us_gridcontrol_t $_[0] > $logfiledir/us_gridcontrol.stdout 2> $logfiledir/us_gridcontrol.stderr`;
	exit;
    }
    print STDERR "$0: gc child pid is $child\n" if $debug;
}

sub startjob_gc_tigre {
    print STDERR "$0: start gc tigre process $_[0]\n" if $debug;
    my $child;
    if(!($child = fork)) {
	printf PROCS "$$ startjob_gc_tigre $_[0]\n" if $debug_cmds;
	print STDERR "$0: child started gc process job\n" if $debug;
	`us_gridcontrol_t $_[0] TIGRE $_[1] > $logfiledir/us_gridcontrol.stdout 2> $logfiledir/us_gridcontrol.stderr`;
	exit;
    }
    print STDERR "$0: gc tigre child pid is $child\n" if $debug;
}

sub startjob_tigre {
    print STDERR "$0: start tigre job process $_[0]\n" if $debug;
    my $child;
    if(!($child = fork)) {
	printf PROCS "$$ startjob_tigre $_[0]\n" if $debug_cmds;
	$SIG{HUP} = 'IGNORE';
	print STDERR "$0: child started tigre job\n" if $debug;
	`$_[0]`;
	exit;
    }
    print STDERR "$0: tigre job child pid is $child\n" if $debug;
    $child;
}

sub tigre_del {
    my $eprfile = $_[0];
    print "tigre_del <$eprfile>\n" if $debugdb;
    dbdel('epr|'. $eprfile);
    dbdel('eprsystem|' . $eprfile);
    dbdel($db{'eprseq|' . $eprfile});
    dbdel('eprseq|' . $eprfile);
    dbdel('eprstatus|' . $eprfile);
    print STDERR "$0: tigre_del $eprfile\n" if $debug;
}
    
sub startjob_tigre_status {
    print STDERR "$0: start tigre status job process $_[0]\n" if $debug;
    my $child;
    if(!($child = fork)) {
	printf PROCS "$$ startjob_tigre_status\n" if $debug_cmds;
	dbopen();
	print STDERR "$0: child status started tigre job\n" if $debug;
	sleep $globus_statusupdate;
	while (1) {
	    if (length(dbread('epr|' . $_[0])) == 0) {
		print STDERR "$0: child status exited job disappeared $_[0]\n" if $debug;
		exit;
	    }
	    print "startjob_tigre_status: <$_[0]>\n";
	    my $status = &timedexec($globustimeout, "globusrun-ws -status -subject /O=Grid/OU=GlobusTest/OU=simpleCA-godzilla.allergan.com/CN=host/godzilla.allergan.com -job-epr-file $_[0]");
	    chomp $status;
	    if(length($status)) {
		dbwrite('eprstatus|' . $_[0], $status);
		if ($status =~ /Done/) {
		    &tigre_del($_[0]);
		    print STDERR "$0: child status exited job Done $_[0]\n" if $debug;
		    exit;
		}
	    }
	    sleep $globus_statusupdate;
	}
	exit;
    }
    print STDERR "$0: tigre status job child pid is $child\n" if $debug;
    $child;
}

## @fn $ write_status($filename)
# forks off a process to write the status to $filename.new and when it is done, locks & copys $filename.new to $filename
# @param the filename
# @return nothing

sub write_status {
    print STDERR "$0: write_status $_[0]\n" if $debug;
    my $child;
    if(!($child = fork)) {
	printf PROCS "$$ write_status $_[0]\n" if $debug_cmds;
	dbopen();
	print STDERR "$0: child started write_status\n" if $debug;
	my $outfile = $_[0];
	print STDERR "$0: write statusqueue status into $outfile\n" if $debug;
	`touch ${outfile}.lock` if ! -e "${outfile}.lock";
	open(LOCK, "${outfile}.lock");
	flock(LOCK, 2) || print STDERR "$0: Warning can not flock ${outfile}.lock, proceeding (possible status file mangling!)\n";
	if(open(OUT, ">${outfile}.new")) {
	    open(OUT_DETAIL, ">${status_detail_file}.new");
	    flock(OUT, 2) || print STDERR "$0: Warning can not flock ${outfile}.new, proceeding (possible status file mangling!)\n";
	    my $date = `date`;
	    print OUT "Queue status snapshot as of $date\n";
	    my $tjobs = 0;
	    %tigre = dbrocopy();
	    foreach $i (keys %tigre) {
		if ($i =~ /^epr\|/) {
		    $tjobs++;
		}
	    }
	    if(@queue == 0 && $tjobs == 0) {
		print OUT "No jobs are currently queued.\n";
		close OUT;
	    } else {
		my @unsorted;
		foreach $i (keys %tigre) {
		    if ($i =~ /^epr\|/) {
			( $use_i ) = $i =~ /^epr\|(.*)$/;
			if (!$tigre{'kill|' . $use_i}) {
			    ( $jid ) = $tigre{$i} =~ /^(\d+) /;
			    ( $mci ) = $tigre{"js|$jid"} =~ /MC iteration (\d+)/;
			    $status = $tigre{'eprstatus|' . $use_i};
			    if ($mci) {
				$status .= " $mci";
			    }
			    if ( $tigre{"js|$jid"} ) {
				print OUT_DETAIL "$jid " . $tigre{"js|$jid"} . "\n";
			    }
			    push @unsorted, "$tigre{$i} $status\n";
			}
		    }
		}
		print OUT sort 
		{ 
		    my $a1;
		    my $b1;
		    ( $a1 ) = $a =~ /(\d*)/;
		    ( $b1 ) = $b =~ /(\d*)/;
		    $a1 <=> $b1;
		} @unsorted;
		close OUT;
		close OUT_DETAIL;
	    }
	    open(FH, $outfile);
	    if(flock(FH, 2)) {
		system("mv -f ${outfile}.new $outfile;mv -f ${status_detail_file}.new $status_detail_file");
	    } else {
		close(FH);
		print STDERR "$0: Warning can not flock $outfile\n";
	    }
	} else {
	    print STDERR "$0: write_status could not open \"$outfile\" for writing.\n";
	}
	close(LOCK);
	exit;
    }
    print STDERR "$0: write status child pid is $child\n" if $debug;
}

## @fn $ status_daemon()
# forks off a process to ping the gridpipe every $statusupdate seconds to rewrite the $statusfile
# @param nothing
# @return nothing

sub status_daemon {
    print STDERR "$0: start status_daemon\n" if $debug;
    my $child;
    if(!($child = fork)) {
	printf PROCS "$$ status_daemon\n" if $debug_cmds;
	dbopen();
	while(1) {
	    print STDERR "$: internal_status_update\n" if $debug;
	    &write_status($status_file);
	    sleep $statusupdate;
	}
	exit; # should never get here!
    }
    print STDERR "$0: status_daemon started as pid $child\n" if $debug;
}

## @fn $ remove_status($filename, $remove)
# forks to remove a job from the status line
# @param the filename and the sequence # to remove
# @return nothing

sub remove_status 
{
    print STDERR "$0: remove_status $_[0]\n" if $debug;
    my $child;
    if (!($child = fork)) 
    {
	printf PROCS "$$ remove_status\n" if $debug_cmds;
	dbopen();
	print STDERR "$0: child started remove_status\n" if $debug;
	my $outfile = $_[0];
	my $remove = $_[1];
	print STDERR "$0: remove_status $remove from $outfile\n" if $debug;
	open(LOCK, "${outfile}.lock");
	flock(LOCK, 2) || print STDERR "$0: Warning can not flock ${outfile}.lock, proceeding (possible status file mangling!)\n";
	if (open(IN, "$outfile")) 
	{
	    open(OUT, ">$outfile.new");
	    flock(OUT, 2) || print STDERR "$0: Warning can not flock ${outfile}.new, proceeding (possible status file mangling!)\n";
	    print STDERR "remove status processing\n";
	    my $date = `date`;
	    print OUT "Queue status snapshot as of $date\n";
	    <IN>; <IN>;
	    my $line;
	    my $count;
	    while ($line = <IN>) 
	    {
		if (!($line =~ /^$remove /))
		{
		    $count++;
		    print OUT $line;
		    print STDERR "remove status printing line <$line>\n";
		} else {
		    print STDERR "skipped status printing line <$line>\n";
		}
	    }
	    close(IN);
	    if (!$count) 
	    {
		print OUT "No jobs are currently queued.\n";
	    } 
	    close OUT;
	    print STDERR "cat $outfile.new : " . `cat ${outfile}.new`;
	    open(FH, $outfile);
	    if (flock(FH, 2)) 
	    {
		system("mv -f ${outfile}.new $outfile");
	    } else {
		close(FH);
		print STDERR "$0: Warning can not flock $outfile\n";
	    }
	} else {
	    print STDERR "$0: write_status could not open \"$outfile\" for writing.\n";
	}
	close(LOCK);
	exit;
    }
    print STDERR "$0: write status child pid is $child\n" if $debug;
}

## @fn $ tigre_kill($eprfile)
# forks off a process to kill the tigre job
# @param the eprfile
# @return nothing

sub tigre_kill {
    print STDERR "$0: tigre_kill $_[0]\n" if $debug;
    if(!(my $child = fork)) {
	printf PROCS "$$ tigre_kill $_[0]\n" if $debug_cmds;
	my $reps = 0;
	do {
	    my $status = &timedexec($globustimeout, "globusrun-ws -kill -subject /O=Grid/OU=GlobusTest/OU=simpleCA-godzilla.allergan.com/CN=host/godzilla.allergan.com -job-epr-file $_[0] 2>&1");
	    print STDERR "$0: tigre_kill $_[0] returned <$status>\n" if $debug;
	    exit if $status =~ /Destroying job\.\.\.Done./;
	    sleep $killupdate;
	} while($reps++ < $maxkillreps);
	exit;
    }
}

## @fn $ handle_request($line)
# forks off a process to handle the request
# @param line
# @return nothing

sub handle_request {
    print STDERR "$0: handle request $_[0]\n" if $debug;
    if(!(my $child = fork)) {
	printf PROCS "$$ handle_request $_[0]\n" if $debug_cmds;
	dbopen();
	print STDERR "$0: received $line\n" if $debug;
	undef $valid;
	if($line =~ /^gc_tigre (.*) (.*)$/) {
	    # process us_gridcontrol
	    $valid++;
	    $file = $1;
	    $system = $2;
	    print STDERR "$0: process us_gridcontrol $file $system\n" if $debug;
	    &startjob_gc_tigre($file, $system);
	}
	if($line =~ /^tigre_job_run (.*)$/) {
	    # just run this one
	    $valid++;
	    $job = $1;
	    $job =~ /^\[(.*)\]\[(.*)\]\[(.*)\]\[(.*)\](.*)$/;
	    $experiment = $1;
	    $email = $2;
	    $analysis_type = $3;
	    $gcfile = $4;
	    $job = $5;
	    @f = split / /, $job;
	    $gc = $gcfile;
	    $timedate = $f[8];
	    $eprfile = $gc;
	    $eprfile =~ s/\/\d+\.gc//;
	    $eprfile .= "/$timedate\/us_tigre_epr${timedate}.xml";
	    $system = $f[12];
	    &parsegc($gcfile);
	    print "eprfile <$eprfile> system <$system>\n";
	    $analysis_type =~ s/SA2D/2DSA/;
	    if($monte_carlo) {
		$analysis_type .= "-MC-$monte_carlo";
	    }
	    $date = `date +'%D %T'`;
	    chomp $date;
	    $seq = dbnxtseq();
	    $pr_line = sprintf("%d %s %s tigre %s %-26s %-25s %-20s %-4s %s",
			       $seq, $db_login_database, $HPCAnalysisID, $date, $system, $email, $exp_file_info[0], $analysis_type, $gcfile);
	    print "prline <$pr_line>\n";

	    print STDERR "$0: email $email analysis_type $analysis_type\n";
	    print STDERR "$0: add job $job\n" if $debug;
	    undef $mcnt;
	    my %vdb;
	    do {
		$mcnt++;
		print "updating db $mcnt with epr|$eprfile\n";
		dbwrite('epr|'. $eprfile, $pr_line);
		dbwrite('eprsystem|' . $eprfile, $system);
		dbwrite('eprseq|' . $eprfile, $seq);
		dbwrite('eprstatus|' . $eprfile,"Current job state: Initialized");
		dbwrite($seq, $eprfile);
		undef %vdb;
		%vdb = dbrocopy();
		my $ky = 'eprseq|' . $eprfile;
		print "org: vdb{$ky} = $vdb{$ky} vs $seq\n" if $vdb{'eprseq|' . $eprfile} ne $seq;
	    } while ($vdb{'eprseq|' . $eprfile} ne $seq);
	    &startjob_tigre($job);
	    print STDERR "$0: tigre_job_run SEQ=$seq $eprfile\n" if $debug;
	}
	if($line =~ /^tigre_job_start (.*)$/) {
	    # just run this one
	    $valid++;
	    $job = $1;
	    $job =~ /^\[(.*)\]\[(.*)\]\[(.*)\]\[(.*)\]\[(.*)\]\[(.*)\]\[(.*)\]\[(.*)\]\[(.*)\]$/;
	    $experiment = $1;
	    $analysis_type = $2;
	    $system = $3;
	    $email = $4;
	    $date = $5;
	    $eprfile = $6;
	    $db = $7;
	    $analysisid = $8;
	    $gcfile = $9;
	    $analysis_type =~ s/SA2D/2DSA/;
	    if (dbread('kill|' . $eprfile)) {
		print STDERR "$0: tigre_job_start, already killed $eprfile\n" if $debug;
		exit;
	    }
	    $seq = dbread('eprseq|' . $eprfile);
	    undef $rewrite;
	    if (!$seq) {
		$seq = dbnxtseq();
		$rewrite++;
	    }
	    $pr_line = sprintf("%d %s %s tigre %s %-26s %-25s %-20s %-4s %s",
			       $seq, $db, $analysisid, $date, $system, $email, $experiment, $analysis_type, $gcfile);
	    print "pr_line <$pr_line>\n";
	    undef $mcnt;
	    my %vdb;
	    do {
		$mcnt++;
		print "updating db $mcnt with epr|$eprfile rewrite $rewrite\n";
		if ($rewrite) {
		    dbwrite('epr|'. $eprfile, $pr_line);
		    dbwrite('eprsystem|' . $eprfile, $system);
		    dbwrite('eprseq|' . $eprfile, $seq);
		    dbwrite($seq,$eprfile);
		} 
		dbwrite('eprstatus|' . $eprfile,"Current job state: Initializing_globus");
		undef %vdb;
		%vdb = dbrocopy();
		my $ky = 'eprseq|' . $eprfile;
		print "rew: vdb{$ky} = $vdb{$ky} vs $seq\n" if $vdb{'eprseq|' . $eprfile} ne $seq;
	    } while ($vdb{'eprseq|' . $eprfile} ne $seq);
	    &startjob_tigre_status($eprfile);

	    print STDERR "$0: tigre_job_start SEQ=$seq $eprfile\n" if $debug;
	}
	if($line =~ /^tigre_job_end (.*)$/) {
 	    # just run this one
	    $valid++;
	    $eprfile = $1;
	    chomp $eprfile;
	    if (length(dbread('epr|' . $eprfile)) > 0) {
		&tigre_del($eprfile);
		print STDERR "$0: tigre_job_end $eprfile\n" if $debug;
	    } 
	    print STDERR "$0: tigre_job_end $eprfile <not exist>\n" if $debug;
	}
	if($line =~ /^tigre_job_clear$/) {
	    # checks all jobs and clears if necessary
	    $valid++;
	    %tigre = dbrocopy();
	    foreach $i (keys %tigre) {
		if ($i =~ /^epr\|/) {
		    ( $use_i ) = $i =~ /^epr\|(.*)$/;
		    $status = &timedexec($globustimeout, "globusrun-ws -status -subject -subject /O=Grid/OU=GlobusTest/OU=simpleCA-godzilla.allergan.com/CN=host/godzilla.allergan.com -job-epr-file $use_i");
		    chomp $status;
		    if(!length($status) || $status eq 'FINISHED' || $status eq 'FAILED') {
			&tigre_del($use_i);
		    }
		    print STDERR "$0: tigre_job_clear removed $tigre{$i}\n" if $debug;
		}
	    }
	}
	if($line =~ /^tigre_job_cancel (.*)$/) {
	    # cancel one job
	    my $cancel_seq = $1;
	    $valid++;
	    %tigre = dbrocopy();
	    $eprfile = $tigre{$cancel_seq};
	    print STDERR "$0: tigre_job_cancel eprfile $eprfile\n";
	    if($eprfile) {
		dbwrite('kill|' . $eprfile, 1);
		&remove_status($status_file, $cancel_seq);
		&tigre_kill($eprfile);
		&tigre_del($eprfile);
	    }
	}
	if($line =~ /^tigre_job_resubmit (.*)$/) {
	    # cancel one job
	    my $cancel_seq = $1;
	    $valid++;
	    %tigre = dbrocopy();
	    $eprfile = $tigre{$cancel_seq};
	    print STDERR "$0: tigre_job_resubmit eprfile $eprfile\n";
	    if($eprfile) {
		dbwrite('kill|' . $eprfile, 1);
		dbwrite('resubmit|' . "$cancel_seq|$eprfile", 1);
		&remove_status($status_file, $cancel_seq);
		&tigre_kill($eprfile);
		&tigre_del($eprfile);
	    }
	}
	if(!$valid) {
	    print STDERR "$0: unknown request \"$line\"\n";
	}
	exit;
    }
}

$SIG{CHLD} = "IGNORE";

$seq = 0;

if($debug_cmds)
{
    open(CMDS, ">$us/etc/us_gridpipe_cmds.txt");
    $old_fh = select(CMDS);
    $|=1;
    open(DBSEQ, ">$us/etc/us_gridpipe_dbseq.txt");
    select(DBSEQ);
    $|=1;
    open(PROCS, ">$us/etc/us_gridpipe_procs.txt");
    select(PROCS);
    $|=1;
    select($old_fh);
}

&dbopen();
&status_daemon();
%tigre = dbrocopy();

foreach $i (keys %tigre) {
    print "keys <$i>\n" if $debugdb;
    if ($i =~ /^epr\|/) {
	( $use_i ) = $i =~ /^epr\|(.*)$/;
	print "i<$i> use_i<$use_i>\n";
	if (!-e $use_i) {
	    print "Deleting job: " . $tigre{'epr|' . $use_i} . "\n";
	    &tigre_del($use_i);
	} else {
	    &startjob_tigre_status($use_i);
	    print "Monitoring job: " . $tigre{'epr|' . $use_i} . "\n";
	}
    }
}	    

while(1) {
    open(PIPE, $pipe) or die "$0: pipe $pipe open failure\n";
    print STDERR "$0: Pipe open\n" if $debug;
    while($line = <PIPE>) {
#	    waitpid -1, 1;
	chomp $line;
#	    close(PIPE);
	print CMDS $line if $debug_cmds;
	&handle_request($line);
	print STDERR "$0: try reading again\n" if $debug;
    }
    close(PIPE);
    print STDERR "$0: Pipe closed\n" if $debug;
}
exit;
