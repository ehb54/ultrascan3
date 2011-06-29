#!/usr/bin/perl

$us = $ENV{'ULTRASCAN'} || die "The environment variable ULTRASCAN must be set.  Terminating\n";
# ----------- user configuration area
#$debug++;
$debugdb++;
$dbname = "$us/etc/us_gridpipe_db";
$globustimeout = 15; # seconds to wait for globusrun-ws commands to succeed or timeout
$statusupdate = 10;  # seconds to wait for status update
$globus_statusupdate = 60;  # seconds to wait for status update
$killupdate = 60;  # seconds to wait for rekilling
$maxkillreps = 120;  # number of times to try to kill before giving up
$logfiledir = "/lustre/tmp";    # reset to /lustre/tmp
# ----------- end user configuration area

use DB_File;
use FileHandle;

$|=1;

$us = $ENV{'ULTRASCAN'} || die "The environment variable ULTRASCAN must be set.  Terminating\n";
open(SYS_LOCK, "$us/etc/us_gridpipe.lock") || die "$0: lockfile $us/etc/us_gridpipe.lock error: $!.  Terminating\n";
flock(SYS_LOCK, 6) || die "$0: lockfile $us/etc/us_gridpipe.lock is already in use ($!).  Terminating\n";

require "$us/etc/us_gcfields.pl";

$status_file = "$us/etc/mpi_queue_status";
$status_detail_file = "$us/etc/queue_status_detail";

$pipe = "$us/etc/us_gridpipe";

$ENV{'DISPLAY'}='';

$dbfile = "$dbname.db";
$dblock = "$dbname.lock";
`touch $dblock` if ! -e $dblock;
die "couldn't create $dblock\n" if ! -e $dblock;

sub LOCK_EX { 2 }
sub LOCK_UN { 8 }

## @fn $ dblock()
# exclusively locks the database, failure to lock retries
# @param nothing
# @return nothing

sub dblock {
    print "dblock\n" if $debugdb;
    if (!flock(DBLOCK, LOCK_EX)) {
	do {
	    print STDERR "$0: warning: error trying to lock file $dblock <$!>\n";
	    close DBLOCK;
	    open(DBLOCK, $dblock) || die "couldn't access dblock file $dblock <$!>\n";
	    sleep 5;
	} while (!flock(DBLOCK, LOCK_EX));
    }
}

## @fn $ dbunlock()
# unlocks the database, failure to lock retries
# @param nothing
# @return nothing

sub dbunlock {
    print "dbunlock\n" if $debugdb;
    if (!flock(DBLOCK, LOCK_UN)) {
	do {
	    print STDERR "$0: warning: error trying to unlock file $dblock<$!>\n";
	    sleep 5;
	} while (!flock(DBLOCK, LOCK_UN));
    }
}

## @fn $ dbopen()
# opens the database
# @param nothing
# @return nothing

sub dbopen {
    print "dbopen\n" if $debugdb;
    undef $DB;
    undef %db;
    open(DBLOCK, $dblock) || die "couldn't access dblock file $dblock <$!>\n";
    dblock();
    $DB = tie %db, "DB_File", $dbfile || die "can not tie $dbfile: $!\n";
    dbunlock();
}

## @fn $ dbclose()
# closed the database
# @param nothing
# @return nothing

sub dbclose {
    print "dbclose\n" if $debugdb;
    dblock();
    untie %db;
    dbunlock();
    close(DBLOCK);
}

## @fn $ dbwrite($key, $val)
# set $db{$key} = $val
# @param key
# @return value

sub dbwrite {
    print "dbwrite $_[0]\n" if $debugdb;
    dblock();
    $db{$_[0]} = $_[1];
    $DB->sync;
    dbunlock();
}

## @fn $ dbread($key)
# returns $db{$key}
# @param key
# @return $db{$key}

sub dbread {
    print "dbread $_[0]\n" if $debugdb;
    dblock();
    my $return = $db{$_[0]};
    dbunlock();
    return $return;
}

## @fn $ dbnxtseq()
# returns a unique increasing #
# @param nothing
# @return a unique sequence #

sub dbnxtseq() {
    print "dbnxtseq\n" if $debugdb;
    dblock();
    my $seq = $db{"next_seq"};
    print "seq was $seq\n" if $debugdb;
    $seq++;
    $db{"next_seq"} = $seq;
    print "seq is $seq $db{'next_seq'}\n" if $debugdb;
    $DB->sync;
    dbunlock();
    $seq;
}

## @fn $ dbdel($key)
# deletes $db{$key}
# @param key
# @return nothing

sub dbdel {
    print "dbread $_[0]\n" if $debugdb;
    dblock();
    delete $db{$_[0]};
    $DB->sync;
    dbunlock();
}

## @fn $ dbdel($key)
# returns a list of the keys of the database
# @param key
# @return list of the keys

sub dbkeys {
    print "dbkeys\n" if $debugdb;
    dblock();
    my @keys = keys %db;
    dbunlock();
    return sort @keys;
}

## @fn $ dbdel($key)
# returns a local copy of the database
# @param key
# @return nothing

sub dbrocopy {
    print "dbrocopy\n" if $debugdb;
    dblock();
    my %mdb = %db;
    dbunlock();
    return %mdb;
}

## @fn $ dblist()
# returns a string listing the database contents
# @param nothing
# @return string

sub dblist {
    print "dblist\n" if $debugdb;
    my $out = '';
    my %mdb = dbrocopy();
    foreach my $i (keys %mdb) {
	$out .= "<$i>:<$mdb{$i}>\n";
    }
    return $out;
}

## @fn $ dbtest()
# a simple database test
# @param nothing
# @return nothing

sub dbtest {
    dbopen();
    dbwrite("key1", "key arg 1");
    dbwrite("key2", "key arg 2");
    dbwrite("key3", "key arg 3");
    print dblist();
    print "----------\n";
    dbdel("key2");
    print dblist();
    dbclose();
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
    dbopen();
    dblock();
    print "tigre_del <$eprfile>\n" if $debugdb;
    delete $db{'epr|'. $eprfile};
    delete $db{'eprsystem|' . $eprfile};
    delete $db{$db{'eprseq|' . $eprfile}};
    delete $db{'eprseq|' . $eprfile};
    delete $db{'eprstatus|' . $eprfile};
    $DB->sync;
#    dbdel('eprsystem|' . $eprfile);
#    dbdel('eprseq|' . $eprfile);
#    dbdel($tigre{'eprseq|' . $eprfile});
#    dbdel('eprstatus|' . $eprfile);
    dbunlock();
    dbclose();
    print STDERR "$0: tigre_del $eprfile\n" if $debug;
}
    
sub is_gfac {
    open(GIN, $_[0]);
    my $gfac = <GIN>;
    close GIN;
    chomp $gfac;
    $gfac =~ /^gfac~(.*)$/ ? $gfac = $1 : undef $gfac;
    print STDERR "epr $_[0]\n";
    print STDERR $gfac ? "gfac set\n" : "gfac not set\n";
    print STDERR `echo echo cat $_[0]; cat $_[0]`;
    return $gfac;
}
    

sub startjob_tigre_status {
    print STDERR "$0: start tigre status job process $_[0]\n" if $debug;
    my $child;
    if(!($child = fork)) {
	close DBLOCK;
	print STDERR "$0: child status started tigre job\n" if $debug;
	sleep $globus_statusupdate;
	while (1) {
            $gfac = is_gfac($_[0]);
	    dbopen();
	    $DB->sync;
	    if (length(dbread('epr|' . $_[0])) == 0) {
		print STDERR "$0: child status exited job disappeared $_[0]\n" if $debug;
		exit;
	    }
	    dbclose();
	    print "startjob_tigre_status: <$_[0]>\n";
	    my $status = &timedexec($globustimeout, 
                                    $gfac ? 
                                    "$us/etc/us_asta_status.pl $gfac" :
                                    "globusrun-ws -status -job-epr-file $_[0]"
                                    );
	    chomp $status;
	    if(length($status)) {
                $status = "Current job state: $status" if $gfac;
		dbopen();
		$DB->sync;
		dbwrite('eprstatus|' . $_[0], $status);
		dbclose();
		if ($status =~ /(Done|FINISHED|COMPLETED)/ ) {
		    &tigre_del($_[0]);
		    print STDERR "$0: child status exited job Done $_[0]\n" if $debug;
		    exit;
		}
	    }
	    sleep $globus_statusupdate;
	}
	dbclose();
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
	close DBLOCK;
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
	    dbopen();
	    $DB->sync;
	    %tigre = dbrocopy();
	    dbclose();
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
	close DBLOCK;
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
	close DBLOCK;
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
	close DBLOCK;
        my $gfac = is_gfac($_[0]);
	my $reps = 0;
        if ( $gfac ) {
            my $cmd = "$us/etc/us_asta_cancel.pl $gfac\n";
            print STDERR "$0: $cmd";
            print STDERR `$cmd`;
        } else {
            do {
                my $status = `globusrun-ws -kill -job-epr-file $_[0] 2>&1`;
                print STDERR "$0: tigre_kill $_[0] returned <$status>\n" if $debug;
                exit if $status =~ /Destroying job\.\.\.Done./;
                sleep $killupdate;
            } while($reps++ < $maxkillreps);
        }
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
	close DBLOCK;
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
	    dbopen();
	    $seq = dbnxtseq();
	    dbclose();
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
		dbopen();
		dblock();
#		$DB->sync;
		$db{'epr|'. $eprfile} =  $pr_line;
		$db{'eprsystem|' . $eprfile} =  $system;
		$db{'eprseq|' . $eprfile} =  $seq;
		$db{'eprstatus|' . $eprfile} = "Current job state: Initialized";
		$db{$seq} = $eprfile;
		$DB->sync;
		dbunlock();
		dbclose();
		dbopen();
		undef %vdb;
		%vdb = dbrocopy();
		dbclose();
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
	    dbopen();
	    if (dbread('kill|' . $eprfile)) {
		dbclose();
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
	    dbclose();
	    undef $mcnt;
	    my %vdb;
	    do {
		$mcnt++;
		print "updating db $mcnt with epr|$eprfile rewrite $rewrite\n";
		dbopen();
		dblock();
#		$DB->sync;
		if ($rewrite) {
		    $db{'epr|'. $eprfile} =  $pr_line;
		    $db{'eprsystem|' . $eprfile} =  $system;
		    $db{'eprseq|' . $eprfile} =  $seq;
		    $db{$seq} = $eprfile;
		} 
		$db{'eprstatus|' . $eprfile} = "Current job state: Initializing_globus";
		$DB->sync;
		dbunlock();
		dbclose();
		dbopen();
		undef %vdb;
		%vdb = dbrocopy();
		dbclose();
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
	    dbopen();
	    $DB->sync;
	    if (length(dbread('epr|' . $eprfile)) > 0) {
		dbclose();
		&tigre_del($eprfile);
		print STDERR "$0: tigre_job_end $eprfile\n" if $debug;
	    } else {
		dbclose();
	    }
	    print STDERR "$0: tigre_job_end $eprfile <not exist>\n" if $debug;
	}
	if($line =~ /^tigre_job_clear$/) {
	    # checks all jobs and clears if necessary
	    $valid++;
	    dbopen();
	    $DB->sync;
	    %tigre = dbrocopy();
	    dbclose();
	    foreach $i (keys %tigre) {
		if ($i =~ /^epr\|/) {
		    ( $use_i ) = $i =~ /^epr\|(.*)$/;
                    my $gfac = is_gfac($use_i);
                    my $status = &timedexec($globustimeout, 
                                            $gfac ? 
                                            "$us/etc/us_asta_status.pl $gfac" :
                                            "globusrun-ws -status -job-epr-file $use_i"
                                            );
		    chomp $status;
		    if( !length($status) || $status =~ 'FINISHED' || $status =~ 'FAILED' || $status =~ 'Done' ) {
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
	    dbopen();
	    $DB->sync;
	    %tigre = dbrocopy();
	    dbclose();
	    $eprfile = $tigre{$cancel_seq};
	    print STDERR "$0: tigre_job_cancel eprfile $eprfile\n";
	    if($eprfile) {
		dbopen();
		$DB->sync;
		dbwrite('kill|' . $eprfile, 1);
		dbclose();
		&remove_status($status_file, $cancel_seq);
		&tigre_kill($eprfile);
		&tigre_del($eprfile);
	    }
	}
	if($line =~ /^tigre_job_resubmit (.*)$/) {
	    # cancel one job
	    my $cancel_seq = $1;
	    $valid++;
	    dbopen();
	    $DB->sync;
	    %tigre = dbrocopy();
	    dbclose();
	    $eprfile = $tigre{$cancel_seq};
	    print STDERR "$0: tigre_job_resubmit eprfile $eprfile\n";
	    if($eprfile) {
		dbopen();
		$DB->sync;
		dbwrite('kill|' . $eprfile, 1);
		dbwrite('resubmit|' . "$cancel_seq|$eprfile", 1);
		dbclose();
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

&status_daemon();
&dbopen();
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

dbclose();

while(1) {
    open(PIPE, $pipe) or die "$0: pipe $pipe open failure\n";
    print STDERR "$0: Pipe open\n" if $debug;
    while($line = <PIPE>) {
#	    waitpid -1, 1;
	chomp $line;
#	    close(PIPE);
	&handle_request($line);
	print STDERR "$0: try reading again\n" if $debug;
    }
    close(PIPE);
    print STDERR "$0: Pipe closed\n" if $debug;
}
exit;
