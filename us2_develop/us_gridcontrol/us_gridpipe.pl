#!/usr/bin/perl -w
use FileHandle;

$|=1;

undef $debug;
$debug++;

$pipe = "$ENV{'ULTRASCAN'}/etc/us_gridpipe";

$ENV{'DISPLAY'}='';

sub startjob {
    print STDERR "$0: start job $queue[0]\n" if $debug;
    $started = `date '+%D %T'`; chomp $started;
    if(!($child = fork)) {
	print STDERR "$0: child started job\n" if $debug;
	`$queue[0]`;
	exit;
    }
    print STDERR "$0: child pid is $child\n" if $debug;
}

sub startjob_gc {
    print STDERR "$0: start gc process $_[0]\n" if $debug;
    if(!($child = fork)) {
	print STDERR "$0: child started gc process job\n" if $debug;
#	`export DISPLAY=bcf:99; us_gridcontrol $_[0] > /tmp/us_gridcontrol.stdout 2> /tmp/us_gridcontrol.stderr`;
	`us_gridcontrol_t $_[0] > /tmp/us_gridcontrol.stdout 2> /tmp/us_gridcontrol.stderr`;
	exit;
    }
    print STDERR "$0: gc child pid is $child\n" if $debug;
}

sub startjob_gc_tigre {
    print STDERR "$0: start gc tigre process $_[0]\n" if $debug;
    if(!($child = fork)) {
	print STDERR "$0: child started gc process job\n" if $debug;
#	`export DISPLAY=bcf:99; us_gridcontrol $_[0] TIGRE > /tmp/us_gridcontrol.stdout 2> /tmp/us_gridcontrol.stderr`;
	`us_gridcontrol_t $_[0] TIGRE > /tmp/us_gridcontrol.stdout 2> /tmp/us_gridcontrol.stderr`;
	exit;
    }
    print STDERR "$0: gc tigre child pid is $child\n" if $debug;
}

sub startjob_tigre {
    print STDERR "$0: start tigre job process $_[0]\n" if $debug;
    if(!($child = fork)) {
	print STDERR "$0: child started tigre job\n" if $debug;
	`$_[0]`;
	exit;
    }
    print STDERR "$0: tigre job child pid is $child\n" if $debug;
}

$SIG{CHLD} = "IGNORE";

while(1) {
	open(PIPE, $pipe) or die "$0: pipe $pipe open failure\n";
	print STDERR "$0: Pipe open\n" if $debug;
	while($line = <PIPE>) {
#	    waitpid -1, 1;
	    chomp $line;
#	    close(PIPE);
	    print STDERR "$0: received $line\n" if $debug;
	    undef $valid;
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
			print OUT "No jobs waiting to complete.\n";
		    } else {
			if(@queue != 0) {
			    print OUT "started             submitted           email\t\texperiment\tanalysis type\n";
			}
			for($i = 0; $i < @queue; $i++) {
			    if($i) {
				print OUT "waiting          ";
			    } else {
				print OUT "$started";
			    }
			    print OUT " : $submitted[$i] : $emails[$i]\t$experiments[$i]\t$types[$i]\n";
			}

			foreach $i (keys %tigre) {
			    $status = `globusrun-ws -status -job-epr-file $i`;
			    chomp $status;
			    $status = "Starting" if !length($status);
			    print OUT "$tigre{$i} $status\n";
			}
		    }
		    close OUT;
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
			print OUT "No jobs waiting to complete.\n";
		    } else {
			if(@queue != 0) {
			    print OUT "started             submitted           email\t\texperiment\tanalysis type\tjob\n";
			}
			for($i = 0; $i < @queue; $i++) {
			    if($i) {
				print OUT "waiting          ";
			    } else {
				print OUT "$started";
			    }
			    print OUT " : $submitted[$i] : $emails[$i]\t$experiments[$i]\t$types[$i]\t$queue[$i]\n";
			}
			foreach $i (keys %tigre) {
			    $status = `globusrun-ws -status -job-epr-file $i`;
			    chomp $status;
			    $status = "Starting" if !length($status);
			    print OUT "$tigre{$i} $status $i\n";
			}
		    }
		    close OUT;
		} else {
		    print STDERR "$0: could not open \"$outfile\" for writing.\n";
		}
		open(PIPE, $pipe) or die "$0: pipe $pipe open failure\n";
	    }
	    if($line =~ /^mpi_job_run (.*)$/) {
		# add to queue
		$valid++;
		$job = $1;
		$job =~ /^\[(.*)\]\[(.*)\]\[(.*)\](.*)$/;
		$experiment = $1;
		$email = $2;
		$analysis_type = $3;
		$job = $4;
		print STDERR "$0: email $email analysis_type $analysis_type\n";
		print STDERR "$0: add job $job\n" if $debug;
		push @queue, $job;
		push @experiments, $experiment;
		push @emails, $email;
		push @types, $analysis_type;
		$date = `date '+%D %T'`; chomp $date; push @submitted, $date;
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
		    print STDERR "$0: shift off job $job\n" if $debug;
		    if(@queue > 0) {
			&startjob;
		    }
		} else {
		    print STDERR "$0: complete but job queue empty\n" if $debug;
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
	    if($line =~ /^gc_tigre (.*)$/) {
		# process us_gridcontrol
		$valid++;
		$file = $1;
		print STDERR "$0: process us_gridcontrol $file\n" if $debug;
		&startjob_gc_tigre($file);
	    }
	    if($line =~ /^tigre_job_run (.*)$/) {
		# just run this one
		$valid++;
		$job = $1;
		$job =~ /^\[(.*)\]\[(.*)\]\[(.*)\](.*)$/;
		$experiment = $1;
		$email = $2;
		$analysis_type = $3;
		$job = $4;
		print STDERR "$0: email $email analysis_type $analysis_type\n";
		print STDERR "$0: add job $job\n" if $debug;
		&startjob_tigre($job);
	    }
	    if($line =~ /^tigre_job_start (.*)$/) {
		# just run this one
		$valid++;
		$job = $1;
		$job =~ /^\[(.*)\]\[(.*)\]\[(.*)\]\[(.*)\]\[(.*)\]\[(.*)\]$/;
		$experiment = $1;
		$analysis_type = $2;
		$system = $3;
		$email = $4;
		$date = $5;
		$eprfile = $6;
		$pr_line = sprintf("%s %-26s %-25s %-20s %-4s",
				   $date, $system, $email, $experiment, $analysis_type);
		$tigre{$eprfile} = $pr_line;
		print STDERR "$0: tigre_job_start $eprfile\n" if $debug;
		foreach $i (keys %tigre) {
		    print STDERR $tigre{$i} . `globusrun-ws -status -job-epr-file $i`;
		}
	    }
	    if($line =~ /^tigre_job_end (.*)$/) {
		# just run this one
		$valid++;
		$eprfile = $1;
		chomp $eprfile;
		foreach $i (keys %tigre) {
		    print STDERR $tigre{$i} . `globusrun-ws -status -job-epr-file $i`;
		}
		delete $tigre{$eprfile};
		print STDERR "$0: tigre_job_end $eprfile\n" if $debug;
	    }
	    if($line =~ /^tigre_job_clear$/) {
		# just run this one
		$valid++;
		foreach $i (keys %tigre) {
		    $status = `globusrun-ws -status -job-epr-file $i`;
		    chomp $status;
		    if(!length($status)) {
			delete $tigre{$i};
			print STDERR "$0: tigre_job_clear removed $tigre{$i}\n" if $debug;
		    }
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
