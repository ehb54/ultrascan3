#!/usr/bin/perl

# user editable section

# WARNING: BEFORE YOU CHANGE THESE VALUES, MAKE SURE YOU CAN RUN :
#         root@bcf:$ us_check_tigre_np _system_ _no_procs_ 
# IF THIS FAILS FOR THE NUMBER OF PROCESSORS YOU HAVE SELECTED, TIGRE JOBS WILL FAIL!
# THIS IS NOT HOW MANY PROCS THE SYSTEM HAS, BUT HOW MANY TIGRE/PBS KNOW ABOUT!!!!

$MAXCPU = 8; # set this if you add cpus!

@gw33downcount = `pbsnodes -l`;
$gw33_no_procs = 8;

$our_default_system = 'gw33.quarry.iu.teragrid.org';

# email info

$from = 'gridcontrol@gw33.quarry.iu.teragrid.org';
$error_dest = 'emre@biochem.uthscsa.edu';
undef $smtp_server;
# END USER EDITABLE SECTION

$SIG{HUP} = 'IGNORE';

$startdate = `date +'\%Y-\%m-\%d \%T'`;
chomp $startdate;

use MIME::Lite;

$US = $ENV{'ULTRASCAN'};

# us_tigre_job.pl
# reads arguments, puts files to TIGRE client, creates xml, runs globusrun command, waits for return,
# gets results, sends email, cleanup on client

# these are the target processor utilizations for normal & jobs with time invariant noise removal
# lower utilizations will improve the speedup achieved at the expense
# of idle processors

$util = .1;
$util_ti = .1;

$execute = 1 ; # uncomment this line for live version, comment for listing commands to be executed

sub check_is_resubmit {
    $cmd = "perl $US/etc/us_gridpipe_is_resubmit.pl '$jid|$basedir/$id/$eprfile'\n";
    print $cmd;
    $is_resubmit = `$cmd`;
    die "tigre job was resubmitted\n" if $is_resubmit == 1;
}    

sub failmsg {
# email a message
    $msg = MIME::Lite->new(From    =>  $from,
			   To      =>  $error_dest,
			   Subject =>  "JOB COMPLETION FAILED on $default_system",
			   Type    =>  'multipart/mixed');
    $msg->attach(Type => 'TEXT',
		 Data => "
--------------------------------------------------------------------------------
jobtype    $jobtype
expname    $expname
gcfile     $gcfile
email      $email
timestamp  $timestamp
id         $id
basedir    $basedir
experiment $experiment
solutes    $solutes
--------------------------------------------------------------------------------
$_[0]
"
		 );

    if(-e "/var/tmp/gc_tigre_${id}.2") {
	$msg->attach(Type     => 'TEXT',
		     Path     => "/var/tmp/gc_tigre_${id}.2",
		     Filename => "gc_tigre_${id}.stderr");
    }

    if(-e "/var/tmp/gc_tigre_${id}.1") {
	$msg->attach(Type     => 'TEXT',
		     Path     => "/var/tmp/gc_tigre_${id}.1",
		     Filename => "gc_tigre_${id}.stdout");
    }

    if(-e "/var/tmp/us_job${id}.stderr") {
	$msg->attach(Type     => 'TEXT',
		     Path     => "/var/tmp/us_job${id}.stderr",
		     Filename => "us_job${id}.stderr");
    }

    if(-e "/var/tmp/us_job${id}.stdout") {
	$msg->attach(Type     => 'TEXT',
		     Path     => "/var/tmp/us_job${id}.stdout",
		     Filename => "us_job${id}.stdout");
    }

    if ( $smtp_server )
    {
	$msg->send('smtp', $smtp_server);
    } else {
	$msg->send();
    }

    $msg = MIME::Lite->new(From    => $from,
			   To      => "$email",
			   Cc      =>  $error_dest,
			   Subject =>  "Your job running on $default_system has failed",
			   Data => "
The $jobtype job using experiment $expname you recently submitted to $default_system has failed.  
Our staff has been informed of this incident and we will be looking into the cause of this error.

Some possibilities for this error are -
1. The job ran longer than expected and timed out
2. The computer(s) on the selected cluster had a systems failure
3. An unexpected error occured in the analysis software

If you are in a hurry to get results - we suggest you resubmit your job to a 
different cluster.

We apologize for any inconvience this may have caused.

jid $id
"
		 );
    
    if ( $smtp_server )
    {
	$msg->send('smtp', $smtp_server);
    } else {
	$msg->send();
    }
};

sub cancelmsg 
{
    $msg = MIME::Lite->new(From    =>  $from,
			   To      => "$email",
			   Cc      =>  $error_dest,
			   Subject =>  "Your job running on $default_system has been canceled.",
			   Data => "
It appears the $jobtype job using experiment $expname you recently submitted to $default_system has been canceled.  
It is also possible that this job failed for some other reason, and if you believe this is the case, please inform
our staff by replying to the addresses CC'd in this email.  

Do not reply to $from, as that email address will bounce.

jid $id
"
		 );
    
    if ( $smtp_server )
    {
	$msg->send('smtp', $smtp_server);
    } else {
	$msg->send();
    }
};

# end user configuration

$|=1;

print "us_tigre_job.pl\n";
$date = `date +'%D %T'`;
chomp $date;

$gcfile = shift;
$total_points = shift;
$email = shift;
$timestamp = shift;
$id = $timestamp;
$basedir = shift;
$experiment = shift;
$solutes = shift;

require "$US/etc/us_gcfields.pl";
&parsegc($gcfile);

$eprfile = "us_tigre_epr${id}.xml";

$cmd = "$US/etc/us_gridpipe_my_jobid.pl $basedir/$id/$eprfile\n";
print $cmd;
$jid = `$cmd`;
chomp $jid;
print "gridpipe jid is <$jid>\n";

$default_system = shift;
$default_system = $our_default_system if !$default_system;
if($default_system eq 'meta') {
    $grms_loc = $ENV{'GRMS_CLIENT_LOCATION'};
    if(!$grms_loc) {
	&failmsg('ERROR: meta requested and no GRMS_CLIENT_LOCATION');
    }
    die "ERROR: meta requested and no GRMS_CLIENT_LOCATION\n";
}

print "cmdline: $0 $gcfile $total_points $email $timestamp $basedir $experiment $solutes $default_system\n";

if($default_system =~ /hlrb2/) {
    $util = .1;
    $util_ti = .1;
    print "setting target utilization to .1 for hlrb\n";
}

if($default_system && 0) {
    $id = $default_system;
    $id =~ s/\..*//;
    $cmd = "rm -fr $id";
    print $cmd;
    `$cmd`;
}

$ugcfile = $gcfile;
if(!-e $gcfile) {
    $ugcfile = "$basedir/$gcfile";
} 

$jobtype = `head -1 $ugcfile`;
chomp $jobtype;
$expname = `head -7 $ugcfile | tail -1`;
chomp $expname;
$jobtype = "2DSA" if $jobtype =~ /^SA2D$/;

$analysis_type = "2DSA" if $analysis_type =~ /^SA2D$/;
$newjobtype = $analysis_type;

if($monte_carlo) {
    $newjobtype .= "-MC-$monte_carlo";
}
$jobtype = $newjobtype if length($analysis_type) && length($newjobtype);

# spec gw33 ppn

$us_ppn = 16;
$us_ppn = 8 if $jobtype =~ /^(2DSA|SA2D)/;
$us_ppn = 8; # until we get node exclusivity

print "
jobtype    $jobtype
expname    $expname
gcfile     $gcfile
email      $email
timestamp  $timestamp
id         $id
basedir    $basedir
experiment $experiment
solutes    $solutes
";


$ULTRASCAN = $ENV{'ULTRASCAN'};

@systems = (
	    'gw33.quarry.iu.teragrid.org' ,
            'queenbee.loni-lsu.teragrid.org' ,
            'gatekeeper.ranger.tacc.teragrid.org' ,
	    );

for($i = 0; $i < @systems; $i++) {
    $reversesystems{$systems[$i]} = $i;
}

@ports_globus = (
		 8443 ,
		 0 ,
		 0
		 );

@ports_ssh = (
	      22 ,
	      22 ,
	      22
	      );

@work = (
	 '/var/tmp/gridjobs' ,
	 '/work/ultrasca' ,
	 '/work/01314/ultrasca' ,
	 ''
	 );

@factorytypes = (
		 'PBS', # gw33
		 '' ,   # queenbee
		 '' ,   # ranger
	       );

@bins = (
       'bin' , # gw33
       'bin64' , # queenbee
       'bin64' , # ranger
       );
@executable = (
       'us_fe_nnls_t_mpi',     #gw33
       'us_fe_nnls_t_mpi.sh',  # queenbee
       'us_fe_nnls_t_mpi.sh',  # ranger
       );

@queues = (
	   '', # gw33
	   '', # queenbee
	   '', # queenbee
	   );

@ld_xml = (
	   "<environment>
  <name>US_PPN</name>
  <value>$us_ppn</value>
  </environment>
" , # gw33
	   "", # queenbee
	   "", # ranger
	   );

@max_np = (
	   $gw33_no_procs,    # gw33
	   64,                # queenbee
	   64,                # ranger
	   );

@max_time = (
	     60000,   # gw33
	     60, # 2880,    # queenbee
	     60, # 2880,    # ranger
	   );

@gsi = (
	'',     # gw33
	'',     # queenbee
	'',     # ranger
	);

@gfac = (
	 0,     # gw33
	 1,     # queenbee
	 1,     # ranger
	 );

# here is where we would select the system to run on via load levels
# for debugging, otherwise, parse $gcfile for utilization to get $np

print "Maximum time[0]=$max_time[0]\n";

$cmd = "perl $ENV{'ULTRASCAN'}/etc/us_parse_gc.pl $util $util_ti $gcfile\n";
print $cmd;
$np = `$cmd`;
chomp $np;
$np = 2 if $np < 2;


$esttime = 120; # we should do better
$hostfile = "$ENV{'ULTRASCAN'}/etc/us_tigre_hosts";

$usesys = $reversesystems{$default_system};

if( $systems[$usesys] ne $default_system) {
    print "warning: system mismatch $systems[$usesys] != $default_system\n" ;
    $default_system = $systems[$usesys];
}

print "using system $default_system\n";

if($default_system ne 'meta' && !$gfac[$usesys]) {
    do {
	$result = `perl $ULTRASCAN/etc/check_tigre.pl $default_system`;
	if($result =~ /ERROR/) {
	    print STDERR "$result";
	    if(!$messagesent) {
		$msg = MIME::Lite->new(From    => $from,
				       To      =>  $error_dest,
				       Subject =>  "Globus ERROR on $default_system",
				       Data    => "There is an error when trying to initialize a job on $default_system
--------------------------------------------------------------------------------
jobtype    $jobtype
expname    $expname
gcfile     $gcfile
email      $email
timestamp  $timestamp
id         $id
basedir    $basedir
experiment $experiment
solutes    $solutes
--------------------------------------------------------------------------------
$result
"
				       );
		if ( $smtp_server )
		{
		    $msg->send('smtp', $smtp_server);
		} else {
		    $msg->send();
		}
		$messagesent++;
	    }
	    sleep 120;
	}
    } while($result =~ /ERROR/);
    if($messagesent) {
	$msg = MIME::Lite->new(From    => $from,
			       To      =>  $error_dest,
			       Subject =>  "Grid Job service restored on $default_system",
			       Data    => "Grid Job service restored on $default_system
--------------------------------------------------------------------------------
jobtype    $jobtype
expname    $expname
gcfile     $gcfile
email      $email
timestamp  $timestamp
id         $id
basedir    $basedir
experiment $experiment
solutes    $solutes
--------------------------------------------------------------------------------
"
				   );
	if ( $smtp_server )
	{
	    $msg->send('smtp', $smtp_server);
	} else {
	    $msg->send();
	}
    }
}


print "max np $max_np[$usesys]\n";
$np = $max_np[$usesys] if $np > $max_np[$usesys];

print "systems (use_sys) $systems[$usesys]\n";

print "np is $np\n";

$SYSTEM = $systems[$usesys];
$GSI_SYSTEM = $SYSTEM;
$PORT_GLOBUS = $ports_globus[$usesys];
$PORT_SSH = $ports_ssh[$usesys];
$WORK = $work[$usesys];
$FACTORYTYPE = $factorytypes[$usesys];
$BIN = $bins[$usesys];
$LD_XML = $ld_xml[$usesys];
$LD_QUEUE = $queues[$usesys];

$WORKTMP = "${WORK}/tmp";
$WORKRUN = "${WORK}/tmp/$id";

$cwd = "$basedir/$id";

$xmlfile = "us_tigre_job_desc${id}.xml";

if($default_system ne 'meta') {
    $pr_line = sprintf("[%20s][%4s][%26s][%25s][%s][%s][%s][%s][%s]",
		       $expname,
		       $jobtype,
		       $default_system,
		       $email,
		       $date,
		       "$cwd/$eprfile",
		       $db_login_database,
		       $HPCAnalysisID,
		       $gcfile);
    
    print `echo tigre_job_start $pr_line > $ENV{'ULTRASCAN'}/etc/us_gridpipe`;
}

# put files to TIGRE client
if($default_system ne 'meta') {
    &check_is_resubmit();
    $cmd = 
	"$gsi[$usesys]ssh -p $PORT_SSH $GSI_SYSTEM rm -fr $WORKRUN
$gsi[$usesys]ssh -p $PORT_SSH $GSI_SYSTEM mkdir -p $WORKRUN
$gsi[$usesys]scp -P $PORT_SSH $experiment $GSI_SYSTEM:${WORKRUN}/experiments${timestamp}.dat
$gsi[$usesys]scp -P $PORT_SSH $solutes $GSI_SYSTEM:${WORKRUN}/solutes${timestamp}.dat\n";
    print $cmd;
    print `$cmd` if $execute;
}
# create xml

chdir $basedir;
`mkdir $id 2> /dev/null`;
if($default_system eq 'meta') {
    print `chmod g+w $id`;
}

chdir $id;

if(!open(JD, ">$xmlfile")) {
    &failmsg("xml output file create error! $xmlfile");
    die "couldn't xml open output file $xmlfile\n";
 }

if($default_system eq 'meta') {
    print JD
"<grmsJob appid=\"ap_grms_1\">
  <task taskid=\"ultrascan\" crucial=\"true\" persistent=\"true\">
     <executable type=\"mpi\" count=\"$np\">
        <execfile name=\"us_fe_nnls_t_mpi\">
           <url>file:///\${HOME}/ultrascan/bin64/us_fe_nnls_t_mpi</url>
        </execfile>
        <arguments>
           <file name=\"exp.dat\" type=\"in\">
              <url>gsiftp://bcf.biochemistry.uthscsa.edu/$experiment</url>
           </file>
           <value>exp.dat</value>
           <file name=\"sol.dat\" type=\"in\">
              <url>gsiftp://bcf.biochemistry.uthscsa.edu/$solutes</url>
           </file>
           <value>sol.dat</value>
           <directory name=\".\" type=\"out\">
              <url>gsiftp://bcf.biochemistry.uthscsa.edu/$cwd</url>
           </directory>
        </arguments>
       <stdout>
          <url>gsiftp://bcf.biochemistry.uthscsa.edu/$cwd/us_job${id}.stdout</url>
       </stdout>
       <stderr>
          <url>gsiftp://bcf.biochemistry.uthscsa.edu/$cwd/us_job${id}.stderr</url>
       </stderr>
     </executable>
     <executionTime>
        <execDuration>PT$max_time[$usesys]M</execDuration>
     </executionTime>
  </task>
</grmsJob>
";
} else {
    $jid_in_xml = "  <argument>$jid</argument>\n" ;

    print JD
"<job>
  <executable>/home/ultrascan/ultrascan/${BIN}/$executable[$usesys]</executable>
  <directory>$WORKRUN</directory>
  <argument>$WORKRUN/experiments${timestamp}.dat</argument>
  <argument>$WORKRUN/solutes${timestamp}.dat</argument>
$jid_in_xml
$LD_XML
  <stdout>$WORKRUN/us_job${id}.stdout</stdout>
  <stderr>$WORKRUN/us_job${id}.stderr</stderr>
  <count>$np</count>
$PROJECT
$LD_QUEUE
  <maxWallTime>$max_time[$usesys]</maxWallTime>
$MAXMEM
  <jobType>mpi</jobType>
</job>
" ;
}
close JD;

# submit the job
if($default_system eq 'meta') {
    $cmd = "grms-client submit_job $xmlfile | grep JobId | awk -F= '{ print \$2 }'";
    print $cmd;
    print $grms_id=`$cmd` if $execute;
    chomp $grms_id;
    print "grms_id = $grms_id\n";
    $pr_line = sprintf("[%20s][%4s][%26s][%25s][%s][%s]",
		       $expname,
		       $jobtype,
		       $default_system,
		       $email,
		       $date,
		       $grms_id);
    
    print `echo tigre_job_start $pr_line > $ENV{'ULTRASCAN'}/etc/us_gridpipe`;
} else {
    &check_is_resubmit();
    if ( !$gfac[$usesys] ) {
	$cmd = 
	    "globusrun-ws -submit -batch -F https://${SYSTEM}:${PORT_GLOBUS}/wsrf/services/ManagedJobFactoryService -factory-type $FACTORYTYPE -f $xmlfile > $eprfile\n";
	print $cmd;
	print `$cmd` if $execute;
    } else {
	$cmd = "perl $US/etc/us_asta_run.pl $SYSTEM ${WORKRUN}/experiments${timestamp}.dat ${WORKRUN}/solutes${timestamp}.dat $jid $np $max_time[$usesys]\n";
	print $cmd;
	if ( $execute ) {
	    $result = `$cmd`;
	    print $result;
	    ( $exp_tag ) = $result =~ /(Experiment-.*)( |$)/;
   	    print "experiment tag is <$exp_tag>\n";
            $cmd = "echo 'gfac~$exp_tag' > $eprfile\n";
            print $cmd;
            print `$cmd`;
        }
    }
}

$dups_blank = 0;
do {
    sleep 10;
    if($default_system eq 'meta') {
	$status = `grms-client job_info $grms_id 2> /dev/null | grep Status | awk '{ print \$3 }'`;
    } else {
	if ( !$gfac[$usesys] ) {
	    $status = `globusrun-ws -status -job-epr-file $eprfile 2> /var/tmp/gc_tigre_${id}_globus_stderr`;
	    $stderr = `cat /var/tmp/gc_tigre_${id}_globus_stderr`;
	    print STDERR $stderr;
	    if($stderr =~ /SOAP Fault/ &&
	       $status != /^$/) {
		print "Soap Fault: ignoring bogus status <$status>\n";
		$status = '';
	    }
	} else {
            $cmd = "$US/etc/us_asta_status.pl $exp_tag\n";
            print $cmd;
            $status = `$cmd`;
            $status = "FINISHED\n" if $status =~ /COMPLETED/;
	}
    }
    print $status;
    chomp $status;
    if($default_system ne 'meta') {
	if($status =~ /.* (\w*)/) {
	    $status = $1;
	}
    }

    if($status =~ /^$/) {
	$dups_blank++;
	&check_is_resubmit();
	if($dups_blank < 10) {
	    print "bad response from checking status, sleep, retry $dups_blank of 10\n";
	    print STDERR "bad response from checking status, sleep, retry $dups_blank of 10\n";
	    sleep ($dups_blank * 20);
	} else {
	    &cancelmsg("Job was canceled or terminated abnormally\n");
	    die "Job was canceled or abnormally terminated\n";
	}
    } else {
	$dups_blank = 0;
    }
    if($status eq 'Failed' ||
       $status eq 'FAILED') {
	if($default_system ne 'meta') {
	    $cmd = 
"$gsi[$usesys]scp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/us_job${id}.stderr /var/tmp/
$gsi[$usesys]scp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/us_job${id}.stdout /var/tmp/
";
	    print $cmd;
	    print `$cmd` if $execute;
	}
	&failmsg("Globus Status returned 'Failed'");
	if($default_system eq 'meta') {
	    print `echo tigre_job_end $grms_id > $ENV{'ULTRASCAN'}/etc/us_gridpipe`;
	} else {
	    print `echo tigre_job_end $cwd/$eprfile > $ENV{'ULTRASCAN'}/etc/us_gridpipe`;
	}
	die "globus job failed submission on $default_system\n";
    }
} while($status ne 'Done' && $status ne 'FINISHED');
# get results (gsiscp models & emails)

if ( $gfac[$usesys] ) {
    $cmd = "$US/etc/us_asta_dir.pl $exp_tag\n";
    print $cmd;
    $WORKRUN = `$cmd`;
    chomp $WORKRUN;
    print "workrun now <$WORKRUN>\n";
}
if($default_system eq 'meta') {
    $cmd = 
"mv us_job${id}.stderr /var/tmp/us_job${id}.stderr
mv us_job${id}.stdout /var/tmp/us_job${id}.stdout
";
} else {
    if ( $gfac[$usesys] ) {
        $cmd = "$gsi[$usesys]scp -C -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/UltraScan_MPI_Program.stderr /var/tmp/us_job${id}.stderr 
$gsi[$usesys]scp -C -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/UltraScan_MPI_Program.stdout /var/tmp/us_job${id}.stdout
$gsi[$usesys]scp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/email_* .
$gsi[$usesys]scp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/*.model* .
$gsi[$usesys]scp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/*noise* .
$gsi[$usesys]scp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/*.simulation_parameters .
";
    } else {
        $cmd = "$gsi[$usesys]scp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/us_job${id}.stderr /var/tmp/
$gsi[$usesys]scp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/us_job${id}.stdout /var/tmp/
$gsi[$usesys]scp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/email_* .
$gsi[$usesys]scp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/*.model* .
$gsi[$usesys]scp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/*noise* .
$gsi[$usesys]scp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/*.simulation_parameters .
";
    }
}
print $cmd;
print `$cmd` if $execute;
print "----tail us_job${id}.stderr --- from $default_system ---\n";
print `tail /var/tmp/us_job${id}.stderr` if $execute;
print "----end us_job${id}.stderr ---\n";

# email results
print "emailing results\n";
print `cp email_text_* email_msg
echo "Results processed on $default_system" >> email_msg`;
print `perl $ENV{'ULTRASCAN'}/bin64/us_email.pl email_list_* email_msg` if $execute;


# cleanup


if($default_system eq 'meta') {
    print `echo tigre_job_end $grms_id > $ENV{'ULTRASCAN'}/etc/us_gridpipe`;
} else {
    $cmd = "$gsi[$usesys]ssh -p $PORT_SSH ${GSI_SYSTEM} rm -fr $WORKRUN\n";
    print "not run: $cmd";
# print `$cmd` if $execute;
    print `echo tigre_job_end $cwd/$eprfile > $ENV{'ULTRASCAN'}/etc/us_gridpipe`;
}

# check for good completion
$good_completion = `grep '^0: finalizing' /var/tmp/us_job${id}.stdout`;
if(!($good_completion =~ '0: finalizing')) {
    &failmsg("Job completed successfully, but results indicate job failed");
}

# accumulate statistics
$US = $ENV{'ULTRASCAN'};
require "$US/etc/us_extract_usage.pl";
&extract_usage("/var/tmp/us_job${id}.stdout");
$extra_fields = "$util|$util_ti|$default_system|$np|$total_points|$esttime|$date|$id";
$extra_fields_title = "util|util_ti|default_system|np|total_points|esttime|date|id";

print "Writing to $US/etc/tigre_db\n";
$lock = "$US/etc/tigre_db_lock";
open(FH, $lock) || print STDERR "$0: WARNING can't open $lock!\n";
flock(FH, 2)  || print STDERR "$0: WARNING can't flock $lock!\n";
open(DB, ">>$US/etc/tigre_db");
print DB "$dbstring|$extract_usage|$extra_fields\n";
close DB;
open(DBS, ">>$US/etc/tigre_short_db");
print DBS "$db_minstring|$extract_usage|$extra_fields\n";
close DBS;
close FH;

use Time::Local;
sub diffdates 
{
    my $date1 = $_[0];
    my $date2 = $_[1];
    my $y1, $y2, $m1, $m2, $d1, $d2, $h1, $h2, $mm1, $mm2, $s1, $s2;
    ( $y1, $m1, $d1, $h1, $mm1, $s1 ) = $date1 =~ /(\d\d\d\d)-(\d\d)-(\d\d) (\d\d):(\d\d):(\d\d)/;
    $y1 -= 1900;
    $m1 -= 1;
    my $time1 = timegm($s1, $mm1, $h1, $d1, $m1, $y1);
    ( $y2, $m2, $d2, $h2, $mm2, $s2 ) = $date2 =~ /(\d\d\d\d)-(\d\d)-(\d\d) (\d\d):(\d\d):(\d\d)/;
    $y2 -= 1900;
    $m2 -= 1;
    my $time2 = timegm($s2, $mm2, $h2, $d2, $m2, $y2);
    return $time2 - $time1;
}
    
if ($HPCAnalysisID > 0) 
{
    print "Updating MySql database $db_login_database for id $HPCAnalysisID\n";
    $enddate = `date +'\%Y-\%m-\%d \%T'`;
    chomp $enddate;
    $walltime = &diffdates($startdate, $enddate);
    use DBI;

    my $dbh = DBI->connect("DBI:mysql:database=$db_login_database;host=$db_login_host",
			   "$db_login_user", "$db_login_password",
			   {'RaiseError' => 1});
    $sql = "update tblHPCAnalysis set\n" .
	"CPU_Number=$np, Datapoints=$total_points, Cluster_Name=\"$default_system\", CPUTime=$jobtime,\n" .
	"EndDateTime=\"$enddate\",max_rss=$maxrss, Walltime=$walltime " .
	"where HPCAnalysis_ID=$HPCAnalysisID;\n";
    print "SQL $sql";
    $dbh->do($sql);

# update tblHPCModel also

    $tid = `echo email_list*`;
    chomp $tid;
    ( $tid ) = $tid =~ /email_list_(\d*)/;

    print "tid is $tid\n";

    @list = `cat email_list_$tid`;
    grep(chomp,@list);

    @models = grep(/$tid\.model\./, @list);
    @tinoise = @models;
    @rinoise = @models;
    grep(s/$tid\.model\./$tid\.ti_noise\./,@tinoise);
    grep(s/$tid\.model\./$tid\.ri_noise\./,@rinoise);

    @text = `cat email_text_$tid`;
    grep(chomp,@text);
    @text = grep(/^Experiment /, @text);

    if ($models[0] =~ /_global_/) 
    {
	print "model global <$models[0]> ti <> ri <> meniscus <> rmsd <>\n";
	undef $model;
	$model = `cat $models[0]` if -e $models[0];
	$sql = "insert into tblHPCModel values " . 
	    "(NULL, 0, \"$model\", NULL, NULL, \"$models[0]\"," . 
	    " \"\", \"\", 0, $HPCAnalysisID);\n";
	print "SQL $sql";
	$dbh->do($sql);
	shift @models;
	shift @tinoise;
	shift @rinoise;
    }
	
    for ($i = 0; $i < @models; $i++) 
    {
	( $rmsd[$i] ) = $text[$i] =~ /rmsd (.*?),/;
	( $meniscus[$i] ) = $text[$i] =~ /meniscus (.*?),/;
	undef $tinoise[$i] if ! -e $tinoise[$i];
	undef $rinoise[$i] if ! -e $rinoise[$i];
	print "model $i <$models[$i]> ti <$tinoise[$i]> ri <$rinoise[$i]> " . 
	    " meniscus <$meniscus[$i]> rmsd <$rmsd[$i]>\n";

	undef $model;
	undef $ti;
	undef $ri;
	$model = `cat $models[$i]` if -e $models[$i];
	$ti = `cat $tinoise[$i]` if $tinoise[$i] && -e $tinoise[$i];
	$ri = `cat $rinoise[$i]` if $rinoise[$i] && -e $rinoise[$i];
	$meniscus[$i] = 0 if !length($meniscus[$i]);
	$sql = "insert into tblHPCModel values " . 
	    "(NULL, $meniscus[$i], \"$model\", \"$ti\", \"$ri\", \"$models[$i]\", \"$tinoise[$i]\", " . 
	    " \"$rinoise[$i]\", $rmsd[$i], $HPCAnalysisID);\n";
	print "SQL $sql";
	$dbh->do($sql);
    }

    $dbh->disconnect();
}


print "Finished\n";

__END__

nohup perl $ULTRASCAN/etc/us_tigre_job.pl /var/www/html/cauma/data/47a5253a5aff13184a456a803d59d60d/1154573785.gc emre@flash.net 060802223315 /var/www/html/cauma/data/47a5253a5aff13184a456a803d59d60d /var/www/html/cauma/data/47a5253a5aff13184a456a803d59d60d/experiments060802223315.dat /var/www/html/cauma/data/47a5253a5aff13184a456a803d59d60d/solutes060802223315-0.dat > /tmp/gc_tigre_060802223315.1 2> /tmp/gc_tigre_060802223315.2 
