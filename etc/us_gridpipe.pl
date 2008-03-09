#!/usr/bin/perl -w
use FileHandle;

$|=1;

undef $debug;
$debug++;


$globustimeout = 15; # seconds to wait for globusrun-ws commands to succeed or timeout
$statusupdate = 60; # seconds to wait for status update

$us = $ENV{'ULTRASCAN'};

require "$us/etc/us_gcfields.pl";

$status_file = "$us/etc/mpi_queue_status";

$pipe = "$us/etc/us_gridpipe";

$ENV{'DISPLAY'}='';

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
#	`export DISPLAY=bcf:99; us_gridcontrol $_[0] > /lustre/tmp/us_gridcontrol.stdout 2> /lustre/tmp/us_gridcontrol.stderr`;
	`us_gridcontrol_t $_[0] > /lustre/tmp/us_gridcontrol.stdout 2> /lustre/tmp/us_gridcontrol.stderr`;
	exit;
    }
    print STDERR "$0: gc child pid is $child\n" if $debug;
}

sub startjob_gc_tigre {
    print STDERR "$0: start gc tigre process $_[0]\n" if $debug;
    my $child;
    if(!($child = fork)) {
	print STDERR "$0: child started gc process job\n" if $debug;
#	`export DISPLAY=bcf:99; us_gridcontrol $_[0] TIGRE > /lustre/tmp/us_gridcontrol.stdout 2> /lustre/tmp/us_gridcontrol.stderr`;
	`us_gridcontrol_t $_[0] TIGRE $_[1] > /lustre/tmp/us_gridcontrol.stdout 2> /lustre/tmp/us_gridcontrol.stderr`;
	exit;
    }
    print STDERR "$0: gc tigre child pid is $child\n" if $debug;
}

sub startjob_tigre {
    print STDERR "$0: start tigre job process $_[0]\n" if $debug;
    my $child;
    if(!($child = fork)) {
	print STDERR "$0: child started tigre job\n" if $debug;
	`$_[0]`;
	exit;
    }
    print STDERR "$0: tigre job child pid is $child\n" if $debug;
    $child;
}

## @fn $ status_daemon()
# forks off a process to ping the gridpipe every $statusupdate seconds to rewrite the $statusfile
# @param nothing
# @return nothing

sub status_daemon {
    print STDERR "$0: start status_daemon\n" if $debug;
    my $child;
    if(!($child = fork)) {
	while(1) {
	    print STDERR "$: internal_status_update\n" if $debug;
	    &timedexec(10, "echo internal_status_update > $pipe");
	    sleep $statusupdate;
	}
	exit; # should never get here!
    }
    print STDERR "$0: status_daemon started as pid $child\n" if $debug;
}

## @fn $ write_status($filename)
# forks off a process to write the status to $filename.new and when it is done, locks & copys $filename.new to $filename
# @param the filename
# @return nothing

sub write_status {
    print STDERR "$0: write_status $_[0]\n" if $debug;
    my $child;
    if(!($child = fork)) {
	print STDERR "$0: child started write_status\n" if $debug;
	my $outfile = $_[0];
	print STDERR "$0: write statusqueue status into $outfile\n" if $debug;
	`touch ${outfile}.lock` if ! -e "${outfile}.lock";
	open(LOCK, "${outfile}.lock");
	flock(LOCK, 2) || print STDERR "$0: Warning can not flock ${outfile}.lock, proceeding (possible status file mangling!)\n";
	if(open(OUT, ">${outfile}.new")) {
	    flock(OUT, 2) || print STDERR "$0: Warning can not flock ${outfile}.new, proceeding (possible status file mangling!)\n";
	    my $date = `date`;
	    print OUT "Queue status snapshot as of $date\n";
	    my $tjobs = 0;
	    foreach $i (keys %tigre) {
		$tjobs++;
	    }
	    if(@queue == 0 && $tjobs == 0) {
		print OUT "No jobs are currently queued.\n";
		close OUT;
	    } else {
		my @unsorted;
		for($i = 0; $i < @queue; $i++) {
		    my $status = "Current job state: ";
		    if($i) {
			$status .= "Waiting";
		    } else {
			$status .= "Active";
		    }
		    push @unsorted, "$mpi_status_lines[$i] $status\n";
		}
		foreach $i (keys %tigre) {
#			    if($tigre{$i} =~ /[ meta]/) {
#				$status = `grms-client job_info $i 2> /dev/null | grep Status | awk '{ print \$3 }'`;
#			    } else {
		    my $status = &timedexec($globustimeout, "globusrun-ws -status -job-epr-file $i");
#			    }
		    chomp $status;
		    $status = "Starting" if !length($status);
		    push @unsorted, "$tigre{$i} $status\n";
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
	    }
	    open(FH, $outfile);
	    if(flock(FH, 2)) {
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
	my $status = `globusrun-ws -kill -job-epr-file $_[0] 2>&1`;
	print STDERR "$0: tigre_kill $_[0] returned <$status>\n" if $debug;
	exit;
    }
}

## @fn $ mpi_restart()
# forks off a process to execute mpi_restart if the active mpi job was cancelled
# @param nothing
# @return nothing

sub mpi_restart {
    print STDERR "$0: mpi_restart\n" if $debug;
    if (!(my $child = fork)) 
    {
	&timedexec(3600, "mpi_killall_apache > /dev/null 2> /dev/null");
	print STDERR "$0: mpi_restart killall 1\n" if $debug;
	sleep 2;
	&timedexec(3600, "mpi_killall_apache > /dev/null 2> /dev/null");
	print STDERR "$0: mpi_restart killall 2\n" if $debug;
	sleep 1;
	print STDERR "$0: mpi_restart restarting\n" if $debug;
	`echo mpi_job_restart > $pipe`;
	print STDERR "$0: mpi_restart child exits\n" if $debug;
	exit;
    }
}

$SIG{CHLD} = "IGNORE";

$seq = 0;

&status_daemon();

while(1) {
	open(PIPE, $pipe) or die "$0: pipe $pipe open failure\n";
	print STDERR "$0: Pipe open\n" if $debug;
	while($line = <PIPE>) {
#	    waitpid -1, 1;
	    chomp $line;
#	    close(PIPE);
	    print STDERR "$0: received $line\n" if $debug;
	    undef $valid;
	    if($line =~ /^internal_status_update$/) {
		$valid++;
		print STDERR "$0: queue status into $status_file\n" if $debug;
		&write_status($status_file);
	    }
		
	    if($line =~ /^status (\S*)$/) {
		close PIPE;
		$valid++;
		$outfile = $1;
		print STDERR "$0: queue status into $outfile\n" if $debug;
		if(open(OUT, ">$outfile")) {
		    $tjobs = 0;
		    foreach $i (keys %tigre) {
			$tjobs++;
		    }
		    if(@queue == 0 && $tjobs == 0) {
			print OUT "No jobs are currently queued.\n";
			close OUT;
		    } else {
			if(@queue != 0) {
			    print OUT "id   started             submitted           email\t\texperiment\tanalysis type\n";
			}
			for($i = 0; $i < @queue; $i++) {
			    print OUT sprintf("%-4d ",$ids[$i]);
			    if($i) {
				print OUT "waiting          ";
			    } else {
				print OUT "$started";
			    }
			    print OUT " : $submitted[$i] : $emails[$i]\t$experiments[$i]\t$types[$i]\n";
			}
			print OUT $last_status;
			close OUT;
			undef $last_status;
			foreach $i (keys %tigre) {
#			    if($tigre{$i} =~ /[ meta]/) {
#				$status = `grms-client job_info $i 2> /dev/null | grep Status | awk '{ print \$3 }'`;
#			    } else {
				$status = &timedexec($globustimeout, "globusrun-ws -status -job-epr-file $i");
#			    }
			    chomp $status;
			    $status = "Starting" if !length($status);
			    $last_status .= "$tigre{$i} $status\n";
			}
		    }
		} else {
		    print STDERR "$0: could not open \"$outfile\" for writing.\n";
		}
		open(PIPE, $pipe) or die "$0: pipe $pipe open failure\n";
	    }
	    if($line =~ /^status_full (\S*)$/) {
		close PIPE;
		$valid++;
		$outfile = $1;
		print STDERR "$0: queue status into $outfile\n" if $debug;
		if(open(OUT, ">$outfile")) {
		    $tjobs = 0;
		    foreach $i (keys %tigre) {
			$tjobs++;
		    }
		    if(@queue == 0 && $tjobs == 0) {
			print OUT "No jobs are currently queued.\n";
			close OUT;
		    } else {
			if(@queue != 0) {
			    print OUT "id   started             submitted           email\t\texperiment\tanalysis type\tjob\n";
			}
			for($i = 0; $i < @queue; $i++) {
			    print OUT sprintf("%-4d ",$ids[$i]);
			    if($i) {
				print OUT "waiting          ";
			    } else {
				print OUT "$started";
			    }
			    print OUT " : $submitted[$i] : $emails[$i]\t$experiments[$i]\t$types[$i]\t$queue[$i]\n";
			}
			print OUT $last_status_full;
			close OUT;
			undef $last_status_full;
			foreach $i (keys %tigre) {
#			    if($tigre{$i} =~ /[ meta]/) {
#				$status = `grms-client job_info $i 2> /dev/null | grep Status | awk '{ print \$3 }'`;
#			    } else {
				$status = &timedexec($globustimeout, "globusrun-ws -status -job-epr-file $i");
#			    }
			    chomp $status;
			    $status = "Starting" if !length($status);
			    $last_status_full .= "$tigre{$i} $status $i\n";
			}
		    }
		} else {
		    print STDERR "$0: could not open \"$outfile\" for writing.\n";
		}
		open(PIPE, $pipe) or die "$0: pipe $pipe open failure\n";
	    }
	    if($line =~ /^mpi_job_run (.*)$/) {
		# add to queue
		$seq++;
		$valid++;
		$job = $1;
		$job =~ /^\[(.*)\]\[(.*)\]\[(.*)\]\[(.*)\](.*)$/;
		$experiment = $1;
		$email = $2;
		$analysis_type = $3;
		$gcfile = $4;
		$job = $5;
		print STDERR "$0: email $email analysis_type $analysis_type seq $seq\n";
		print STDERR "$0: add job $job\n" if $debug;
		&parsegc($gcfile);
		$analysis_type =~ s/SA2D/2DSA/;
		if($monte_carlo) {
		    $analysis_type .= "-MC-$monte_carlo";
		}
		push @ids, $seq;
		push @queue, $job;
		push @experiments, $experiment;
		push @emails, $email;
		push @types, $analysis_type;
		$date = `date '+%D %T'`; chomp $date; push @submitted, $date;
		$mpi_status_line = sprintf("%d %s %s mpi %s %-26s %-25s %-20s %-4s",
				   $seq, $db_login_database, $HPCAnalysisID, $date, "bcf.uthscsa.edu", $email, $exp_file_info[0], $analysis_type);
		push @mpi_status_lines, $mpi_status_line;
		if(@queue == 1) {
		    # no jobs are running, so start this one
		    &startjob;
		}
	    }
	    if($line =~ /^mpi_job_complete$/) {
		# pop from queue
		$valid++;
		if(@queue > 0) {
		    $job = shift @queue;
		    shift @submitted;
		    shift @experiments;
		    shift @emails;
		    shift @types;
		    shift @ids;
		    shift @mpi_status_lines;
		    print STDERR "$0: shift off job $job\n" if $debug;
		    if(@queue > 0) {
			&startjob;
		    }
		} else {
		    print STDERR "$0: complete but job queue empty\n" if $debug;
		}
	    }
	    if($line =~ /^mpi_job_cancel/) {
		# pop from queue
		( $id ) = $line =~ /^mpi_job_cancel (\d*)/;
		$valid++;
		$mark = -1;
		for($i = 0; $mark == -1 && $i < @queue; $i++) {
		    if($ids[$i] == $id) {
			&remove_status($status_file, $id);
			$mark = $i;
			print STDERR "$0: shift off job $queue[$i]\n" if $debug;
			splice @ids, $i, 1;
			splice @queue, $i, 1;
			splice @submitted, $i, 1;
			splice @experiments, $i, 1;
			splice @emails, $i, 1;
			splice @types, $i, 1;
			splice @mpi_status_lines, $i, 1;
			&mpi_restart() if !$i;
		    }
		} 
		if($mark == -1) {
		    print STDERR "$0: mpi_job_cancel could not find id $id\n" if $debug;
		}
	    }
	    if($line =~ /^mpi_job_to_end/) {
		# pop from queue
		( $id ) = $line =~ /^mpi_job_to_end (\d*)/;
		$valid++;
		$mark = -1;
		for($i = 0; $mark == -1 && $i < @queue; $i++) {
		    if($ids[$i] == $id) {
			$mark = $i;
			print STDERR "$0: mpi_job_to_end job $queue[$i]\n" if $debug;
			@org_mpi_status_lines = @mpi_status_lines;
			@org_ids = @ids;
			@org_queue = @queue;
			@org_submitted = @submitted;
			@org_experiments = @experiments;
			@org_emails = @emails;
			@org_types = @types;
			splice @mpi_status_lines, $i, 1;
			splice @ids, $i, 1;
			splice @queue, $i, 1;
			splice @submitted, $i, 1;
			splice @experiments, $i, 1;
			splice @emails, $i, 1;
			splice @types, $i, 1;
		    }
		} 
		if($mark == -1) {
		    print STDERR "$0: mpi_to_end could not find id $id\n" if $debug;
		} else {
		    push @mpi_status_lines, $org_mpi_status_lines[$mark];
		    push @ids, $org_ids[$mark];
		    push @queue, $org_queue[$mark];
		    push @submitted, $org_submitted[$mark];
		    push @experiments, $org_experiments[$mark];
		    push @emails, $org_emails[$mark];
		    push @types, $org_types[$mark];
		}
	    }
	    if($line =~ /^mpi_job_restart$/) {
		# restart 1st entry
		$valid++;
		print STDERR "$0: restarting\n" if $debug;
		if(@queue > 0) {
		    &startjob;
		}
	    }
	    if($line =~ /^gc (.*)$/) {
		# process us_gridcontrol
		$valid++;
		$file = $1;
		print STDERR "$0: process us_gridcontrol $file\n" if $debug;
		&startjob_gc($file);
	    }
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
		$analysis_type =~ s/SA2D/2DSA/;
		print STDERR "$0: email $email analysis_type $analysis_type\n";
		print STDERR "$0: add job $job\n" if $debug;
		&startjob_tigre($job,$system);
	    }
	    if($line =~ /^tigre_job_start (.*)$/) {
		# just run this one
		$valid++;
		$seq++;
		$job = $1;
		$job =~ /^\[(.*)\]\[(.*)\]\[(.*)\]\[(.*)\]\[(.*)\]\[(.*)\]\[(.*)\]\[(.*)\]$/;
		$experiment = $1;
		$analysis_type = $2;
		$system = $3;
		$email = $4;
		$date = $5;
		$eprfile = $6;
		$db = $7;
 		$analysisid = $8;
		$analysis_type =~ s/SA2D/2DSA/;
		$pr_line = sprintf("%d %s %s tigre %s %-26s %-25s %-20s %-4s",
				   $seq, $db, $analysisid, $date, $system, $email, $experiment, $analysis_type);
		$tigre{$eprfile} = $pr_line;
		$tigre_system{$eprfile} = $system;
		$tigre_seq{$eprfile} = $seq;
		$tigre_epr{$seq} = $eprfile;

		print STDERR "$0: tigre_job_start SEQ=$seq $eprfile\n" if $debug;
		foreach $i (keys %tigre) {
#		    if($tigre{$i} =~ /[ meta]/) {
#			print STDERR $tigre{$i} . `grms-client job_info $i 2> /dev/null | grep Status | awk '{ print \$3 }'`;
#		    } else {
		        $status = &timedexec($globustimeout, "globusrun-ws -status -job-epr-file $i");
			print STDERR $tigre{$i} . $status;
#		    }
		}
	    }
	    if($line =~ /^tigre_job_end (.*)$/) {
		# just run this one
		$valid++;
		$eprfile = $1;
		chomp $eprfile;
		foreach $i (keys %tigre) {
#		    if($tigre{$i} =~ /[ meta]/) {
#			print STDERR $tigre{$i} . `grms-client job_info $i 2> /dev/null | grep Status | awk '{ print \$3 }'`;
#		    } else {
			print STDERR $tigre{$i} . &timedexec($globustimeout, "globusrun-ws -status -job-epr-file $i");
#		    }
		}
		delete $tigre{$eprfile};
		delete $tigre_epr{$tigre_seq{$eprfile}};
		delete $tigre_seq{$eprfile};
		delete $tigre_system{$eprfile};
		print STDERR "$0: tigre_job_end $eprfile\n" if $debug;
	    }
	    if($line =~ /^tigre_job_clear$/) {
		# just run this one
		$valid++;
		foreach $i (keys %tigre) {
#		    if($tigre{$i} =~ /[ meta]/) {
#			$status = `grms-client job_info $i 2> /dev/null | grep Status | awk '{ print \$3 }'`;
#		    } else {
		        $status = &timedexec($globustimeout, "globusrun-ws -status -job-epr-file $i");
#		    }
		    chomp $status;
		    if(!length($status) || $status eq 'FINISHED' || $status eq 'FAILED') {
			delete $tigre{$i};
			delete $tigre_epr{$tigre_seq{$i}};
			delete $tigre_seq{$i};
			delete $tigre_system{$i};
			print STDERR "$0: tigre_job_clear removed $tigre{$i}\n" if $debug;
		    }
		}
	    }
	    if($line =~ /^tigre_job_cancel (.*)$/) {
		# cancel one job
		my $cancel_seq = $1;
		$valid++;
		$eprfile = $tigre_epr{$cancel_seq};
		print STDERR "$0: tigre_job_cancel eprfile $eprfile\n";
		if($eprfile) {
		    &remove_status($status_file, $cancel_seq);
		    &tigre_kill($eprfile);
		    delete $tigre{$eprfile};
		    delete $tigre_epr{$tigre_seq{$eprfile}};
		    delete $tigre_seq{$eprfile};
		    delete $tigre_system{$eprfile};
		}
	    }
	    if(!$valid) {
		print STDERR "$0: unknown request \"$line\"\n";
	    }
	    print STDERR "$0: try reading again\n" if $debug;
	}
	close(PIPE);
	print STDERR "$0: Pipe closed\n" if $debug;
}
exit;
