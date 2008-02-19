#!/usr/bin/perl

# user editable section

# WARNING: BEFORE YOU CHANGE THESE VALUES, MAKE SURE YOU CAN RUN :
#         root@bcf:$ us_check_tigre_np _system_ _no_procs_ 
# IF THIS FAILS FOR THE NUMBER OF PROCESSORS YOU HAVE SELECTED, TIGRE JOBS WILL FAIL!
# THIS IS NOT HOW MANY PROCS THE SYSTEM HAS, BUT HOW MANY TIGRE/PBS KNOW ABOUT!!!!


$bcf_no_procs = 42;
$alamo_no_procs = 31;
$laredo_no_procs = 36;

# END USER EDITABLE SECTION

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

$util = .5;
$util_ti = .5;

$execute = 1 ; # uncomment this line for live version, comment for listing commands to be executed

sub failmsg {
# email a message
    $msg = MIME::Lite->new(From    => 'gridcontrol@ultrascan.uthscsa.edu',
			   To      =>  'ebrookes@cs.utsa.edu, jeremy@biochem.uthscsa.edu, demeler@biochem.uthscsa.edu',
#			   To      =>  'ebrookes@cs.utsa.edu',
			   Subject =>  "TIGRE JOB COMPLETION FAILED on $default_system",
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

    if(-e "/lustre/tmp/us_job${id}.stderr") {
	$msg->attach(Type     => 'TEXT',
		     Path     => "/lustre/tmp/us_job${id}.stderr",
		     Filename => "us_job${id}.stderr");
    }

    if(-e "/lustre/tmp/us_job${id}.stdout") {
	$msg->attach(Type     => 'TEXT',
		     Path     => "/lustre/tmp/us_job${id}.stdout",
		     Filename => "us_job${id}.stdout");
    }
    
    $msg->send('smtp', 'smtp.uthscsa.edu');

    $msg = MIME::Lite->new(From    => 'gridcontrol@ultrascan.uthscsa.edu',
			   To      => "$email",
			   Cc      =>  'ebrookes@cs.utsa.edu, jeremy@biochem.uthscsa.edu, demeler@biochem.uthscsa.edu',
#			   To      =>  'ebrookes@cs.utsa.edu',
			   Subject =>  "Your TIGRE job running on $default_system has failed",
			   Data => "
The TIGRE $jobtype job using experiment $expname you recently submitted to $default_system has failed.  
Our staff has been informed of this incident and we will be looking into the cause of this error.

Some possibilities for this error are -
1. The job ran longer than expected and timed out
2. The computer(s) on the selected TIGRE cluster had a systems failure
3. An unexpected error occured in the analysis software

If you are in a hurry to get results - we suggest you resubmit your job to a 
different TIGRE cluster.

We apologize for any inconvience this may have caused.
"
		 );
    
    $msg->send('smtp', 'smtp.uthscsa.edu');
};

sub cancelmsg 
{
    $msg = MIME::Lite->new(From    => 'gridcontrol@ultrascan.uthscsa.edu',
			   To      => "$email",
			   Cc      =>  'ebrookes@cs.utsa.edu, jeremy@biochem.uthscsa.edu, demeler@biochem.uthscsa.edu',
#			   To      =>  'ebrookes@cs.utsa.edu',
			   Subject =>  "Your TIGRE job running on $default_system has been canceled.",
			   Data => "
It appears the TIGRE $jobtype job using experiment $expname you recently submitted to $default_system has been canceled.  
It is also possible that this job failed for some other reason, and if you believe this is the case, please inform
our staff by replying to the addresses CC'd in this email.  

Do not reply to gridcontrol\@ultrascan.uthscsa.edu, as that email address will bounce.
"
		 );
    
    $msg->send('smtp', 'smtp.uthscsa.edu');
};

# end user configuration

$|=1;

if($default_system eq 'meta') {
    $grms_loc = $ENV{'GRMS_CLIENT_LOCATION'};
    if(!$grms_loc) {
	&failmsg('ERROR: meta requested and no GRMS_CLIENT_LOCATION');
    }
    die "ERROR: meta requested and no GRMS_CLIENT_LOCATION\n";
}

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

$default_system = shift;
$default_system = "bcf.uthscsa.edu" if !$default_system;
    
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
if($default_system ne 'meta') {
 do {
    $result = `perl $ULTRASCAN/etc/check_tigre.pl $default_system`;
    if($result =~ /ERROR/) {
	print STDERR "$result";
	if(!$messagesent) {
	    $msg = MIME::Lite->new(From    => 'gridcontrol@ultrascan.uthscsa.edu',
				   To      =>  'ebrookes@cs.utsa.edu, jeremy@biochem.uthscsa.edu, demeler@biochem.uthscsa.edu',
				   Subject =>  "TIGRE ERROR on $default_system",
				   Data    => "There is an error when trying to initialize a TIGRE job on $default_system
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
	    $msg->send('smtp', 'smtp.uthscsa.edu');
	    $messagesent++;
	}
	sleep 120;
    }
} while($result =~ /ERROR/);
if($messagesent) {
    $msg = MIME::Lite->new(From    => 'gridcontrol@ultrascan.uthscsa.edu',
			   To      =>  'ebrookes@cs.utsa.edu, jeremy@biochem.uthscsa.edu, demeler@biochem.uthscsa.edu',
			   Subject =>  "TIGRE service restored on $default_system",
			   Data    => "TIGRE service restored on $default_system
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
    $msg->send('smtp', 'smtp.uthscsa.edu');
 }
}


@systems = (
	    'lonestar.tacc.utexas.edu' , 
	    'cosmos.tamu.edu' ,
	    'antaeus.hpcc.ttu.edu' ,
	    'gridgate.rtc.rice.edu' ,
	    'eldorado.acrl.uh.edu' ,
	    'bcf.uthscsa.edu' ,
	    'alamo.uthscsa.edu' ,
	    'laredo.uthscsa.edu' ,
	    'meta'
	    );

for($i = 0; $i < @systems; $i++) {
    $reversesystems{$systems[$i]} = $i;
}

@ports_globus = (
		 8443 ,
		 8443 ,
		 9443 ,
		 9443 ,
		 8443 ,
		 9443 ,
		 9443 ,
		 9443 ,
		 0
		 );

@ports_ssh = (
	      22 ,
	      2222 ,
	      49922 ,
	      22 ,
	      22 ,
	      22 ,
	      22 ,
	      22 ,
	      0
	      );

@work = (
	 '/work/teragrid/tg457210' ,
	 '/home/ehb1056' ,
	 '/home/tigrepool0003' ,
	 '/users/eb4' ,
	 '/home/b/btigre01' ,
	 '/home/tigre' ,
	 '/home/tigre' ,
	 '/home/tigre' ,
	 ''
	 );

@factorytypes = (
	       'LSF' ,
	       'PBS' ,
	       'LSF' ,
	       'PBS' ,
	       'PBS' ,
	       'PBS' ,
	       'PBS' ,
	       'PBS' ,
	       'LSF' ,
		 ''
	       );

@bins = (
       'bin64' ,
       'bin'   ,
       'bin64' ,
       'bin'   ,
       'bin'   ,
       'bin64' ,
       'bin' ,
       'bin'   ,
	 'bin64'
       );

@queues = (
#	   '<queue>normal</queue>' , # lonestar
	   '<queue>high</queue>' , # lonestar
	   '<queue>normal</queue>' , # cosmos
	   '<queue>tigre</queue>' , #antaeus
	   '' , # gridgate
	   '' , # eldorado
	   '' , #bcf
	   '' , #alamo
	   '' ,  #laredo
	   '' #meta
	   );



@ld_xml = (
	   '  <environment>
    <name>LD_LIBRARY_PATH</name>
    <value>/work/teragrid/tg457210/qt/lib:/work/teragrid/tg457210/qwt/lib:/work/teragrid/tg457210/qwt3d/lib:/work/teragrid/tg457210/ultrascan/lib64:/opt/MPI/intel9/mvapich/0.9.7/lib:/opt/intel/compiler9.1/cc/lib:/opt/intel/compiler9.1/fc/lib:/data/TG/globus-4.0.1-r3/lib:/data/TG/globus-4.0.1-r3/myproxy-3.4/lib:/data/TG/gsi-openssh-3.6-r1/lib:/usr/lib:/usr/X11R6/lib:/usr/local/lib:/data/TG/lib:/data/TG/srb-client-3.4.1-r1/lib:/data/TG/64bit/globus-4.0.1-r3/lib:/data/TG/hdf4-4.2r1-r1/lib:/opt/apps/hdf5/hdf5-1.6.5/lib:/data/TG/phdf5-1.6.5/lib:</value>
  </environment>
'   , # lonestar
	   '  <environment>
    <name>LD_LIBRARY_PATH</name>
    <value>/opt/intel/9.1-20060523/lib:/opt/intel/mkl/6.1.1.004/mkl61/lib/64:/usr/local/tigre/globus/lib:/usr/local/tigre/globus/lib:/usr/X11R6/lib:/usr/X11R6/lib/modules</value>
  </environment>
'   , # cosmos
	   '' , # antaeus
#	   '  <environment>
#    <name>LD_LIBRARY_PATH</name>
#    <value>/home/tigrepool0003/qt/lib:/home/tigrepool0003/lib:/opt/intel/fce/9.1.039/lib:/opt/intel/cce/9.1.044/lib</value>
#  </environment>
#'   , # antaeus

	   '' , #gridgate
	   '' , #eldorado
	   '' , # bcf
	   '' , # alamo
	   '' , # laredo
	   '' # meta
	   );

#	   '  <environment>
#    <name>LD_LIBRARY_PATH</name>
#    <value>/share/apps64/ultrascan/lib64:/share/apps64/qwt/lib64:/share/apps64/qwtplot3d/lib64:/opt/openmpi/1.2.1/lib:/share/apps/tigre/globus/lib</value>
#    <name>PATH</name>
#    <value>/home/tigre/ultrascan/bin64:/opt/openmpi/1.2.1/bin:/usr/kerberos/bin:/usr/java/jre1.6.0/bin:/share/apps/tigre/globus/bin:/share/apps/tigre/globus/sbin:/usr/local/bin:/bin:/usr/bin:/usr/X11R6/bin:/usr/local/Adobe/Acrobat7.0/bin:/share/apps/pedsys/bin:/share/apps/miRanda/bin:/share/apps/scilab/bin:/usr/local/blast:/share/apps/pedsys/bin:/share/apps/phylip-3.6a:/share/apps/grasp/bin:/share/apps/modeller6v2/bin:/opt/Bio/ncbi/bin:/opt/Bio/mpiblast/bin/:/opt/Bio/hmmer/bin:/opt/Bio/EMBOSS/bin:/opt/Bio/clustalw/bin:/opt/Bio/t_coffee/bin:/opt/Bio/phylip/exe:/opt/Bio/mrbayes:/opt/Bio/fasta:/opt/Bio/glimmer/bin://opt/Bio/glimmer/scripts:/opt/Bio/gromacs/bin:/opt/eclipse:/opt/ganglia/bin:/opt/ganglia/sbin:/share/apps/gasbor:/share/apps/hydropro/:/opt/maven/bin:/opt/maui/bin:/opt/torque/bin:/opt/torque/sbin:/opt/rocks/bin:/opt/rocks/sbin:/opt/condor/bin:/opt/condor/sbin:/share/apps64/ultrascan/bin64:/home/tigre/bin</value>
#  </environment>
#'             ,
#	   ''  ,
#  <environment>
#    <name>LD_LIBRARY_PATH</name>
#    <value>/opt/openmpi/lib:/share/apps/ultrascan/lib:/share/apps/qwt/lib:/share/apps/qwtplot3d/lib:/share/apps/tigre/globus/lib:/share/apps/tigre/globus/lib:/share/apps/ultrascan/lib:/share/apps/qwt/lib:/share/apps/qwtplot3d/lib:/share/apps/tigre/globus/lib:/home/tigre/qwt3d/lib</value>
#    <name>PATH</name>
#    <value>/home/tigre/bin:/opt/openmpi/bin:/opt/torque/bin:/usr/kerberos/bin:/usr/java/jdk1.5.0_07/bin:/usr/bin:/bin:/usr/sbin:/sbin:/share/apps/tigre/globus/bin:/usr/X11R6/bin:/opt/Bio/ncbi/bin:/opt/Bio/mpiblast/bin/:/opt/Bio/hmmer/bin:/opt/Bio/Emboss/bin:/opt/Bio/clustalw/bin:/opt/Bio/t_coffee/bin:/opt/Bio/phylip/exe:/opt/Bio/mrbayes:/opt/Bio/fasta:/opt/Bio/glimmer/bin://opt/Bio/glimmer/scripts:/opt/Bio/gromacs/bin:/opt/eclipse:/opt/ganglia/bin:/opt/ganglia/sbin:/opt/maven/bin:/opt/openmpi/bin/:/opt/maui/bin:/opt/torque/bin:/opt/torque/sbin:/usr/share/pvm3/lib:/usr/share/pvm3/lib/LINUX:/usr/share/pvm3/bin/LINUX:/opt/rocks/bin:/opt/rocks/sbin:/opt/condor/bin:/opt/condor/sbin:/share/apps/ultrascan/bin</value>
#  </environment>
#'
#	   '' #laredo
#	 );

@max_np = (
	   64 ,
	   4 ,
	   16 ,
	   64 ,
	   64 ,
	   $bcf_no_procs ,
	   $alamo_no_procs ,
	   $laredo_no_procs ,
	   64 #meta
	   );

@max_time = (
	   30 ,
	   120 ,
	   120 ,
	   120 ,
	   120 ,
	   60000 ,
	   60000 ,
	   60000 ,
	   1440
	   );


# here is where we would select the system to run on via load levels
# for debugging, otherwise, parse $gcfile for utilization to get $np


if($analysis_type =~ /^GA/) {
    if($fit_ti_noise || $fit_ri_noise) {
	$max_time[0] = (($ga_params_initial_solutes ** 2) * $ga_params_generations * $ga_params_genes) / 7403;
    } else {
	$max_time[0] = (($ga_params_initial_solutes ** 1.3) * $ga_params_generations * $ga_params_genes) / 7403;
    }
    print "GA time $max_time[0]\n";
    $max_time[0] = 30 if $max_time[0] < 30;
    $max_time[0] *= 1.5;
}

if($analysis_type =~ /^2DSA/) {
    $max_time[0] = 10;
    print "2DSA time $max_time[0]\n";
}

if($monte_carlo) {
    $max_time[0] *= $monte_carlo;
};

if($sa2d_params_use_iterative &&
   $sa2d_max_iterations > 1) {
    $max_time[0] *= $sa2d_max_iterations;
}

if($meniscus_gridpoints) {
    $max_time[0] *= $meniscus_gridpoints;
}

if($fit_ti_noise || $fit_ri_noise) {
    $max_time[0] *= 2;
}


$max_time[0] = 5 if $max_time[0] <= 5;
if($max_time[0] > 2880 &&
   $default_system eq 'lonestar.tacc.utexas.edu') {
    $msg = MIME::Lite->new(From    => 'gridcontrol@ultrascan.uthscsa.edu',
			   To      =>  "$email, ebrookes@cs.utsa.edu",
			   Subject =>  "TIGRE job on $default_system set to maximum time",
			   Data    => 
"Your TIGRE job on $default system estimated has an estimated run time of " . ($max_time[0]/60) . " which exceeds the maximum 48 hour limit.  
It has been set to this limit, but the job may terminate prematurely, losing all results.  
The job info is as follows:
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
    $msg->send('smtp', 'smtp.uthscsa.edu');
}    
    

$max_time[0] = 2880 if $max_time[0] > 2880;
$max_time[4] = $max_time[0];
$max_time[2] = $max_time[0];
$max_time[1] = $max_time[0];
$max_time[8] = $max_time[0];

print "Maximum time[0]=$max_time[0]\n";

$cmd = "perl $ENV{'ULTRASCAN'}/etc/us_parse_gc.pl $util $util_ti $gcfile\n";
print $cmd;
$np = `$cmd`;
chomp $np;
$np = 2 if $np < 2;

$esttime = 120; # we should do better
$hostfile = "$ENV{'ULTRASCAN'}/etc/us_tigre_hosts";

if(!$default_system) {
    use SelectResource;
    ($default_system, $np) = SelectResource::select_tigre($hostfile, $np, $max_time[$usesys]);
}

$usesys = $reversesystems{$default_system};

if( $systems[$usesys] ne $default_system) {
    print "warning: system mismatch $systems[$usesys] != $default_system\n" ;
    $default_system = $systems[$usesys];
}

print "using tigre system $default_system\n";
print "max np $max_np[$usesys]\n";
$np = $max_np[$usesys] if $np > $max_np[$usesys];

print "np is $np\n";

$SYSTEM = $systems[$usesys];
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
$eprfile = "us_tigre_epr${id}.xml";

if($default_system ne 'meta') {
    $pr_line = sprintf("[%20s][%4s][%26s][%25s][%s][%s][%s][%s]",
		       $expname,
		       $jobtype,
		       $default_system,
		       $email,
		       $date,
		       "$cwd/$eprfile",
		       $db_login_database,
		       $HPCAnalysisID);
    
    print `echo tigre_job_start $pr_line > $ENV{'ULTRASCAN'}/etc/us_gridpipe`;
}

# put files to TIGRE client
if($default_system ne 'meta') {
    $cmd = 
	"gsissh -p $PORT_SSH $SYSTEM rm -fr $WORKRUN
gsissh -p $PORT_SSH $SYSTEM mkdir -p $WORKRUN
gsiscp -P $PORT_SSH $experiment $SYSTEM:${WORKRUN}/experiments${timestamp}.dat
gsiscp -P $PORT_SSH $solutes $SYSTEM:${WORKRUN}/solutes${timestamp}.dat\n";
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
    print JD
"<job>
  <executable>${WORK}/ultrascan/${BIN}/us_fe_nnls_t_mpi</executable>
  <directory>$WORKRUN</directory>
  <argument>$WORKRUN/experiments${timestamp}.dat</argument>
  <argument>$WORKRUN/solutes${timestamp}.dat</argument>
$LD_XML
  <stdout>$WORKRUN/us_job${id}.stdout</stdout>
  <stderr>$WORKRUN/us_job${id}.stderr</stderr>
  <count>$np</count>
$LD_QUEUE
  <maxWallTime>$max_time[$usesys]</maxWallTime>
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
    $cmd = "globusrun-ws -submit -batch -term 12/31/2099 -F https://${SYSTEM}:${PORT_GLOBUS}/wsrf/services/ManagedJobFactoryService -factory-type $FACTORYTYPE -f $xmlfile > $eprfile\n";
    print $cmd;
    print `$cmd` if $execute;
}

do {
    sleep 10;
    if($default_system eq 'meta') {
	$status = `grms-client job_info $grms_id 2> /dev/null | grep Status | awk '{ print \$3 }'`;
    } else {
	$status = `globusrun-ws -status -job-epr-file $eprfile`;
    }
    print $status;
    chomp $status;
    if($default_system ne 'meta') {
	$status =~ /.* (\w*)/;
	$status = $1;
    }
    if($status =~ /^$/) {
	&cancelmsg("Tigre job was canceled or terminated abnormally\n");
	die "tigre job was canceled or abnormally terminated\n";
    }
    if($status eq 'Failed' ||
       $status eq 'FAILED') {
	if($default_system ne 'meta') {
	    $cmd = 
"gsiscp -P $PORT_SSH ${SYSTEM}:${WORKRUN}/us_job${id}.stderr .
gsiscp -P $PORT_SSH ${SYSTEM}:${WORKRUN}/us_job${id}.stdout .
";
	    print $cmd;
	    print `$cmd` if $execute;
	}
	&failmsg("Tigre Status returned 'Failed'");
	if($default_system eq 'meta') {
	    print `echo tigre_job_end $grms_id > $ENV{'ULTRASCAN'}/etc/us_gridpipe`;
	} else {
	    print `echo tigre_job_end $cwd/$eprfile > $ENV{'ULTRASCAN'}/etc/us_gridpipe`;
	}
	die "tigre job failed submission on $default_system\n";
    }
} while($status ne 'Done' && $status ne 'FINISHED');
# get results (gsiscp models & emails)

if($default_system eq 'meta') {
    $cmd = 
"mv us_job${id}.stderr /lustre/tmp/us_job${id}.stderr
mv us_job${id}.stdout /lustre/tmp/us_job${id}.stdout
";
} else {
    $cmd = "gsiscp -P $PORT_SSH ${SYSTEM}:${WORKRUN}/us_job${id}.stderr /lustre/tmp/
gsiscp -P $PORT_SSH ${SYSTEM}:${WORKRUN}/us_job${id}.stdout /lustre/tmp/
gsiscp -P $PORT_SSH ${SYSTEM}:${WORKRUN}/email_* .
gsiscp -P $PORT_SSH ${SYSTEM}:${WORKRUN}/*.model* .
gsiscp -P $PORT_SSH ${SYSTEM}:${WORKRUN}/*noise* .
";
}
print $cmd;
print `$cmd` if $execute;
print "----tail us_job${id}.stderr --- from $default_system ---\n";
print `tail /lustre/tmp/us_job${id}.stderr` if $execute;
print "----end us_job${id}.stderr ---\n";

# email results
print "emailing results\n";
print `cp email_text_* email_msg
echo "Results processed by TIGRE on $default_system" >> email_msg`;
print `perl $ENV{'ULTRASCAN'}/bin64/us_email.pl email_list_* email_msg` if $execute;


# cleanup


if($default_system eq 'meta') {
    print `echo tigre_job_end $grms_id > $ENV{'ULTRASCAN'}/etc/us_gridpipe`;
} else {
    $cmd = "gsissh -p $PORT_SSH ${SYSTEM} rm -fr $WORKRUN\n";
    print "not run: $cmd";
# print `$cmd` if $execute;
    print `echo tigre_job_end $cwd/$eprfile > $ENV{'ULTRASCAN'}/etc/us_gridpipe`;
}

# check for good completion
$good_completion = `grep '^0: finalizing' /lustre/tmp/us_job${id}.stdout`;
if(!($good_completion =~ '0: finalizing')) {
    &failmsg("TIGRE job completed successfully, but results indicate job failed");
}

# accumulate statistics
$US = $ENV{'ULTRASCAN'};
require "$US/etc/us_extract_usage.pl";
&extract_usage("/lustre/tmp/us_job${id}.stdout");
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
	$sql = "insert into tblHPCModel values " . 
	    "(NULL, 0, \"$model\", \"$ti\", \"$ri\", \"$models[$i]\", \"$tinoise[$i]\", " . 
	    " \"$rinoise[$i]\", $rmsd[$i], $HPCAnalysisID);\n";
	print "SQL $sql";
	$dbh->do($sql);
    }

    $dbh->disconnect();
}


print "Finished\n";

__END__

nohup perl $ULTRASCAN/etc/us_tigre_job.pl /var/www/html/cauma/data/47a5253a5aff13184a456a803d59d60d/1154573785.gc emre@flash.net 060802223315 /var/www/html/cauma/data/47a5253a5aff13184a456a803d59d60d /var/www/html/cauma/data/47a5253a5aff13184a456a803d59d60d/experiments060802223315.dat /var/www/html/cauma/data/47a5253a5aff13184a456a803d59d60d/solutes060802223315-0.dat > /tmp/gc_tigre_060802223315.1 2> /tmp/gc_tigre_060802223315.2 
