#!/usr/bin/perl

# user editable section

# WARNING: BEFORE YOU CHANGE THESE VALUES, MAKE SURE YOU CAN RUN :
#         root@bcf:$ us_check_tigre_np _system_ _no_procs_ 
# IF THIS FAILS FOR THE NUMBER OF PROCESSORS YOU HAVE SELECTED, TIGRE JOBS WILL FAIL!
# THIS IS NOT HOW MANY PROCS THE SYSTEM HAS, BUT HOW MANY TIGRE/PBS KNOW ABOUT!!!!

@bcfdowncount = `/opt/torque/bin/pbsnodes -l`;
@laredodowncount = `ssh laredo.uthscsa.edu /opt/torque/bin/pbsnodes -l`;
@alamodowncount = `ssh alamo.uthscsa.edu /opt/torque/bin/pbsnodes -l`;

$bcf_no_procs = 32 -2 * @bcfdowncount;
$alamo_no_procs = 32 - 2 * @alamodowncount;
#$laredo_no_procs = 20;
#$laredo_no_procs = 20 - 4 * @laredodowncount;

$MAX_RETRIES = 11;
$MAX_RETRIES_SHORT = 2;

# END USER EDITABLE SECTION

$SIG{HUP} = 'IGNORE';

$startdate = `date +'\%Y-\%m-\%d \%T'`;
chomp $startdate;

use MIME::Lite;
use IO::CaptureOutput qw/capture/;

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

sub check_is_resubmit {
    $cmd = "perl $US/etc/us_gridpipe_is_resubmit.pl '$jid|$basedir/$id/$eprfile'\n";
    print $cmd;
    $is_resubmit = `$cmd`;
    die "tigre job was resubmitted\n" if $is_resubmit == 1;
}    

sub if_gfac_set_workrun {
    if ( $gfac[$usesys] ) {
	$cmd = "$US/etc/us_asta_dir.pl $exp_tag\n";
	my $tries = 0;
	do {
	    print $cmd;
	    $WORKRUN = `$cmd`;
	    chomp $WORKRUN;
	    print "workrun now <$WORKRUN>\n";
	    if ( !length($WORKRUN) )
	    {
		$tries++;
		if ( $tries <= $MAX_RETRIES_SHORT ) {
		    print "no directory yet (try $tries) from $cmd, sleeping " , ($tries * 30), "s\n";
		    sleep $tries * 30;
		} else {
		    print "couldn't get workrun directory & too many retries\n";
		}
	    } 
	} while (!length($WORKRUN) && $tries <= $MAX_RETRIES_SHORT);
    }
}

sub ifbigcompress {
    my $file = $_[0];
    my $limit = 500;
    return $file if !-e $file;

    my $c = `ls -s $file`;
    if ($c > $limit) {
	print "too big, need compressed version\n" if $debug;
	my $cfile = "$file.gz";
	if ( -e $cfile ) {
	    print "compressed version already exists\n" if $debug;
	    return $cfile;
	}
	print "compressing\n" if $debug;
	my $cmd = "gzip -c $file > $cfile\n";
	print $cmd if $debug;
	my $res = `$cmd`;
	if ( !-e $cfile ) {
	    print "tried to compress, didn't work!\n" if $debug;
	    return $file;
	}
	return $cfile;
    } 
    print "file size ok, leave alone\n" if $debug;
    $file;
}

sub failmsg {
# email a message
    $msg = MIME::Lite->new(From    => 'gridcontrol@ultrascan.uthscsa.edu',
			   To      => "$email",
			   Cc      =>  'emre@biochem.uthscsa.edu, jeremy@biochem.uthscsa.edu, demeler@biochem.uthscsa.edu',
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

    if(-e "/lustre/tmp/gc_tigre_${id}.2") {
	$msg->attach(Type     => 'TEXT',
		     Path     => "/lustre/tmp/gc_tigre_${id}.2",
		     Filename => "gc_tigre_${id}.stderr");
    }

    if(-e "/lustre/tmp/gc_tigre_${id}.1") {
	$msg->attach(Type     => 'TEXT',
		     Path     => "/lustre/tmp/gc_tigre_${id}.1",
		     Filename => "gc_tigre_${id}.stdout");
    }

    if(-e "/lustre/tmp/us_job${id}.stderr") {
	$fname = &ifbigcompress("/lustre/tmp/us_job${id}.stderr");
	if ( $fname =~ /\.gz$/ ) {
	    $msg->attach(Type     => 'application/x-gzip',
			 Path     => "/lustre/tmp/us_job${id}.stderr.gz",
			 Filename => "us_job${id}.stderr.gz");
	} else {
	    $msg->attach(Type     => 'TEXT',
			 Path     => "/lustre/tmp/us_job${id}.stderr",
			 Filename => "us_job${id}.stderr");
	}
    }

    if(-e "/lustre/tmp/us_job${id}.stdout") {
	$fname = &ifbigcompress("/lustre/tmp/us_job${id}.stdout");
	if ( $fname =~ /\.gz$/ ) {
	    $msg->attach(Type     => 'application/x-gzip',
			 Path     => "/lustre/tmp/us_job${id}.stdout.gz",
			 Filename => "us_job${id}.stdout.gz");
	} else {
	    $msg->attach(Type     => 'TEXT',
			 Path     => "/lustre/tmp/us_job${id}.stdout",
			 Filename => "us_job${id}.stdout");
	}
    }

    $msg->send('smtp', 'smtp.uthscsa.edu');

    $msg = MIME::Lite->new(From    => 'gridcontrol@ultrascan.uthscsa.edu',
			   To      => "$email",
			   Cc      =>  'emre@biochem.uthscsa.edu, jeremy@biochem.uthscsa.edu, demeler@biochem.uthscsa.edu',
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

jid $id
"
		 );
    
    $msg->send('smtp', 'smtp.uthscsa.edu');
};

sub cancelmsg 
{
    $msg = MIME::Lite->new(From    => 'gridcontrol@ultrascan.uthscsa.edu',
			   To      => "$email",
			   Cc      =>  'emre@biochem.uthscsa.edu, jeremy@biochem.uthscsa.edu, demeler@biochem.uthscsa.edu',
#			   To      =>  'ebrookes@cs.utsa.edu',
			   Subject =>  "Your TIGRE job running on $default_system has been canceled.",
			   Data => "
It appears the TIGRE $jobtype job using experiment $expname you recently submitted to $default_system has been canceled.  
It is also possible that this job failed for some other reason, and if you believe this is the case, please inform
our staff by replying to the addresses CC'd in this email.  

Do not reply to gridcontrol\@ultrascan.uthscsa.edu, as that email address will bounce.

jid $id
"
		 );
    
    $msg->send('smtp', 'smtp.uthscsa.edu');
};


sub gsiget {
    print "sub gsiget:\n";
    my $maxtry = 5;
    my $sys = $_[0];
    my $port = $_[1];
    my $sfile = $_[2];
    my $dfile = $_[3];
    my $cmd = "$gsi[$usesys]ssh -p $port $sys ls -l $sfile | awk '{ print \$5 }'";
    print "$cmd\n";
    my $retry;
    my $stderr;
    my $result;
    if ( $execute ) {
	$retry = 0;
	do {
	    my $tosleep = $retry * 30;
	    if ( $retry ) {
		print "can not run <$cmd>, retry $retry, sleeping $tosleep\n";
		sleep $tosleep;
	    }
	    capture sub {
		system($cmd);
	    } => \$result, \$stderr;
	    $stderr =~ s/\s*//g;
	    $stderr =~ s/Warning:Noxauthdata;usingfakeauthenticationdataforX11forwarding.//g;
	    $stderr =~ s/\s*//g;
	    $retry++;
	    print "$cmd results:\n\tstdout <$result>\n\tstderr <$stderr>\n";
	    if ( $retry > $MAX_RETRIES ) {
		print "ERROR too many retries\n";
	    }
	} while (length($stderr) && $retry <= $MAX_RETRIES);
    }

    my $res = $execute ? $result : '1234 not run';
    chomp $res;
    print "result <$res>\n";
    my $count = 0;
    do {
	$cmd = "$gsi[$usesys]scp -C -P $port $sys:$sfile $dfile";
	print "$cmd\n";
	if ( $execute ) {
	    $retry = 0;
	    do {
		my $tosleep = $retry * 30;
		if ( $retry ) {
		    print "can not run <$cmd>, retry $retry, sleeping $tosleep\n";
		    sleep $tosleep;
		}
		capture sub {
		    system($cmd);
		} => \$result, \$stderr;
		$stderr =~ s/\s*//g;
		$stderr =~ s/Warning:Noxauthdata;usingfakeauthenticationdataforX11forwarding.//g;
		$stderr =~ s/\s*//g;
		$retry++;
		print "$cmd results:\n\tstdout <$result>\n\tstderr <$stderr>\n";
		if ( $retry > $MAX_RETRIES ) {
		    print "ERROR too many retries\n";
		}
	    } while (length($stderr) && $retry <= $MAX_RETRIES);
	}
	my $newres = $execute ? $result : '4321 not run';
	print "result <$newres>\n";
	chomp $newres;
	$cmd = "ls -l $dfile | awk '{ print \$5 }'\n";
	print "$cmd\n";
	$newres = $execute ? `$cmd` : '4321 not run';
	chomp $newres;
	print "result <$newres>\n";
	if ( $newres != $res ) {
	    $count++;
	    print "$gsi[$usesys]scp failed try $count of $maxtry\n";
	    return if $count >= $maxtry;
	} else {
	    return;
	}
    } while (1);
}

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
$cwd = "$basedir/$id";

require "$US/etc/us_gcfields.pl";
&parsegc($gcfile);

$eprfile = "us_tigre_epr${id}.xml";

$cmd = "$US/etc/us_gridpipe_my_jobid.pl $basedir/$id/$eprfile\n";
print $cmd;
$jid = `$cmd`;
chomp $jid;
print "gridpipe jid is <$jid>\n";

$default_system = shift;
$default_system = "bcf.uthscsa.edu" if !$default_system;
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

$cmd = "us_cmdline_t check_limits $experiment";
print "$cmd\n";
$results = `$cmd`;

if ( $results =~ /error/ ) {
    print "error: $results";
    $msg = MIME::Lite->new(From    => 'gridcontrol@ultrascan.uthscsa.edu',
			   To      => "$email",
			   Cc      =>  'emre@biochem.uthscsa.edu, jeremy@biochem.uthscsa.edu, demeler@biochem.uthscsa.edu',
			   Subject =>  "GRID JOB COMPLETION FAILED on $default_system / DATA RANGE ERROR",
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
$results
"
		 );
    $msg->send('smtp', 'smtp.uthscsa.edu');
    $cmd = "echo tigre_job_end $cwd/$eprfile > $ENV{'ULTRASCAN'}/etc/us_gridpipe";
    print "$cmd\n";
    print `$cmd`;
    exit;
} else {
    print "ok\n";
}

$ULTRASCAN = $ENV{'ULTRASCAN'};


@systems = (
	    'lonestar.tacc.utexas.edu' , 
	    'ranger.tacc.utexas.edu' , 
	    'cosmos.tamu.edu' ,
	    'antaeus.hpcc.ttu.edu' ,
	    'gridgate.rtc.rice.edu' ,
	    'eldorado.acrl.uh.edu' ,
	    'bcf.uthscsa.edu' ,
	    'alamo.uthscsa.edu' ,
	    'laredo.uthscsa.edu' ,
	    'a01.hlrb2.lrz-muenchen.de' ,
	    'ng2.vpac.monash.edu.au' ,
            'queenbee.loni-lsu.teragrid.org' ,
            'gatekeeper.bigred.iu.teragrid.org' ,
            'gatekeeper.ranger.tacc.teragrid.org' ,
            'lonestar.tacc.teragrid.org' ,
            'bcf.biochemistry.uthscsa.edu' ,
	    'meta' 
	    );

for($i = 0; $i < @systems; $i++) {
    $reversesystems{$systems[$i]} = $i;
}
$home[$reversesystems{'ng2.vpac.monash.edu.au'}] = "/home/grid-ultrascan/";

@ports_globus = (
		 8443 ,
		 8443 ,
		 8443 ,
		 9443 ,
		 9443 ,
		 8443 ,
		 9443 ,
		 9443 ,
		 9443 ,
		 8443 ,
		 8443 ,
		 8443 , # queenbee
		 8443 , # bigred
		 0, # gatekeeper.ranger
		 0, # new lonestar 
		 0, # bcf gram5 
		 0 
		 );

@ports_ssh = (
	      22 ,
	      22 ,
	      2222 ,
	      49922 ,
	      22 ,
	      22 ,
	      22 ,
	      22 ,
	      22 ,
	      2222 ,
	      22 ,
	      22 , # queenbee
	      22 , # bigred
	      22 , # ranger
	      22 , # new lonestar
	      22 , # bcf gram5
	      0 ,
	      );

@work = (
	 '/work/teragrid/tg457210' ,
	 '/work/00451/tg457210' ,
	 '/home/ehb1056' ,
	 '/home/tigrepool0003' ,
	 '/users/eb4' ,
	 '/home/b/btigre01' ,
	 '/home/tigre' ,
	 '/home/tigre' ,
	 '/home/tigre' ,
	 '/home/hlrb2/pr28ci/lu65cen' ,
	 '/home/grid-ultrascan' ,
	 '/work/brookes' ,
	 '/N/dc/scratch/tg-ebrookes' , # bigred
	 '/work/01314/ultrasca' , # ranger
	 '/work/01314/ultrasca' , # new lonestar
	 '/work/ultrasca' , # bcf gram5
	 '' 
	 );

@factorytypes = (
	       'LSF' ,
	       'LSF' ,
	       'PBS' ,
	       'LSF' ,
	       'PBS' ,
	       'PBS' ,
	       'PBS' ,
	       'PBS' ,
	       'PBS' ,
	       'PBS' ,
	       'PBS' ,
	       'PBS' ,
	       'Loadleveler' , # bigred
	       '' , # ranger
	       '' , # new lonestar
	       '' , # gram5 bcf
               'PBS' ,  # ?
	       '' 
	       );

@bins = (
         'bin64' , #lonestar
         'bin64' , #ranger
         'bin'   , #cosmos
         'bin64' , #antaeus
         'bin'   , #gridgate?
         'bin'   , #eldorado
         'bin64' , #bcf
         'bin' ,   #alamo
         'bin64'   , #laredo
         'bin'   , #a01.hlrb2
         'bin'   , #ng2.vpac.monash
	 'bin64' ,  #queenbee
	 'bin' ,  # bigred
	 'bin64' ,  # ranger
	 'bin64' ,  # new lonestar
	 'bin64' ,  # gram5 bcf
	 'bin64'   #meta
       );
@executable = (
       'us_fe_nnls_t_mpi' , #lonestar
       'us_fe_nnls_t_mpi' , #ranger
       'us_fe_nnls_t_mpi' , #cosmos
       'us_fe_nnls_t_mpi' , #antaeus
       'us_fe_nnls_t_mpi' , #gridgate
       'us_fe_nnls_t_mpi' , #eldorado
       'us_fe_nnls_t_mpi' , #bcf
       'us_fe_nnls_t_mpi' , #alamo
       'us_fe_nnls_t_mpi' , #laredo
       'us_fe_nnls_t' , #a01.hlrb2
       'us_fe_nnls_t_mpi' , #ng2.vpac.monash
       'us_fe_nnls_t.sh' , #queenbee
       'us_fe_nnls_t_mpi' , #bigred
       'us_fe_nnls_t_mpi.sh' , #ranger
       'us_fe_nnls_t_mpi' , #new lonestar
       'us_fe_nnls_t_mpi' , #gram5 bcf
       'us_fe_nnls_t_mpi' , #meta
       );

@queues = (
#	   '<queue>normal</queue>' , # lonestar
	   '<queue>high</queue>' , # lonestar
	   '<queue>normal</queue>' , # lonestar
	   '<queue>normal</queue>' , # cosmos
	   '<queue>tigre</queue>' , #antaeus
	   '' , # gridgate
	   '' , # eldorado
	   '' , #bcf
	   '' , #alamo
	   '' ,  #laredo
	   '' ,  #hlrb2
	   '' ,  #ng2.vpac.monash
	   '' ,  #queenbee
	   '<queue>NORMAL</queue>' , # bigred
	   '' , #ranger / determined based upon runtime
	   '' , # new lonestar / determined based upon runtime
	   'default' , # gram5 bcf
	   '' #meta
	   );

# /work/teragrid/tg457210/qt/lib:/work/teragrid/tg457210/qwt/lib:/work/teragrid/tg457210/qwt3d/lib:/work/teragrid/tg457210/ultrascan/lib64:/opt/MPI/intel9/mvapich/0.9.7/lib:/opt/intel/compiler9.1/cc/lib:/opt/intel/compiler9.1/fc/lib:/data/TG/globus-4.0.1-r3/lib:/data/TG/globus-4.0.1-r3/myproxy-3.4/lib:/data/TG/gsi-openssh-3.6-r1/lib:/usr/lib:/usr/X11R6/lib:/usr/local/lib:/data/TG/lib:/data/TG/srb-client-3.4.1-r1/lib:/data/TG/64bit/globus-4.0.1-r3/lib:/data/TG/hdf4-4.2r1-r1/lib:/opt/apps/hdf5/hdf5-1.6.5/lib:/data/TG/phdf5-1.6.5/lib:</value>

@ld_xml = (
	   '  <environment>
    <name>LD_LIBRARY_PATH</name>
    <value>/work/teragrid/tg457210/qt/lib:/work/teragrid/tg457210/qwt/lib:/work/teragrid/tg457210/qwt3d/lib:/work/teragrid/tg457210/ultrascan//lib64:/opt/apps/intel10/hdf5/1.6.5/lib:/opt/apps/intel10/mvapich/1.0.1/lib:/opt/apps/intel10/mvapich/1.0.1/lib/shared:/opt/apps/intel/10.1//cc/lib:/opt/apps/intel/10.1//fc/lib:/opt/apps/gsi-openssh-3.9/lib:/usr/lib:/usr/X11R6/lib:/usr/local/lib:.:/import/data2/TG/globus-4.0.7-r1/lib:/data/TG/tgcp-1.0.0-r2/lib:</value>
  </environment>
  <environment>
    <name>ULTRASCAN</name>
    <value>/work/teragrid/tg457210/ultrascan</value>
  </environment>
'   , # lonestar
	   '  <environment>
    <name>LD_LIBRARY_PATH</name>
    <value>/opt/apps/intel10_1/mvapich/1.0.1/lib:/opt/apps/intel10_1/mvapich/1.0.1/lib/shared:/opt/apps/intel/10.1/cc/lib:/opt/apps/intel/10.1/fc/lib:/share/apps/teragrid/globus-4.0.7-r1/lib:/share/apps/teragrid/globus-4.0.7-r1/myproxy-3.4/lib:/share/apps/teragrid/globus-4.0.1-r3/lib:/share/apps/teragrid/globus-4.0.1-r3/myproxy-3.4/lib:/share/apps/teragrid/srb-client-3.4.1-r1/lib:/opt/gsi-openssh-4.1/lib:/opt/gsi-openssh-4.1/lib:/opt/apps/binutils-amd/070220/lib64
  </environment>
  <environment>
    <name>ULTRASCAN</name>
    <value>/work/00451/tg457210/ultrascan</value>
  </environment>
'   , # ranger
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
	   '' , # hlrb2
	   '  <environment>
    <name>ULTRASCAN</name>
    <value>/home/grid-ultrascan/ultrascan</value>
  </environment>
' , # ng2.vpac.monash
	   ' 
 <environment>
    <name>ULTRASCAN</name>
    <value>/home/brookes/ultrascan</value>
  </environment>
' , # queenbee
	   '', # bigred
	   '', # ranger
	   '', # new lonestar
	   '', # gram5 bcf
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
	   128 ,
	   64 ,
	   4 ,
	   16 ,
	   64 ,
	   64 ,
	   $bcf_no_procs ,
	   $alamo_no_procs ,
	   $laredo_no_procs ,
	   2048 , # hlrb2
	   32 , # ng2.vpac.monash
	   64 , # queenbee
	   64 , # bigred
	   128 , # ranger
	   144 , # new lonestar
	   16 , # gram5 bcf
	   64 #meta
	   );

@max_time = (
	   30 ,    # lonestar
	   2880 ,  # ranger
	   120 ,   # cosmos
	   120 ,   # antaeus
	   120 ,   # gridgate
	   120 ,   # eldorado
	   60000 , # bcf
	   60000 , # alamo
	   60000 , # laredo
	   2880 ,  # a01.hlrb2
	   10000 , # ng2.monash
	   2880 ,  # queenbee
	   2880 ,  # bigred
	   2880 ,  # ranger
	   1440 ,  # new lonestar
	   60000 , # bcf
	   2880    # meta
	   );
@gsi = (
	'gsi' ,    # lonestar
	'gsi' ,  # ranger
	'gsi' ,   # cosmos
	'gsi' ,   # antaeus
	'gsi' ,   # gridgate
	'gsi' ,   # eldorado
	'' , # bcf
	'' , # alamo
	'' , # laredo
	'gsi' ,  # a01.hlrb2
	'gsi' , # ng2.monash
	'gsi' ,  # queenbee
	'gsi' ,  # bigred
	'' ,  # ranger
	'' ,  # new lonestar
	'' ,  # gram5 bcf
	'gsi'    # meta
	);

@gfac = (
	 0 ,    # lonestar
	 0 ,  # ranger
	 0 ,   # cosmos
	 0 ,   # antaeus
	 0 ,   # gridgate
	 0 ,   # eldorado
	 0 , # bcf
	 0 , # alamo
	 0 , # laredo
	 0 ,  # a01.hlrb2
	 0 , # ng2.monash
	 0 ,  # queenbee
	 0 ,  # bigred
	 1 ,  # ranger
	 1 ,  # new lonestar
	 1 ,  # gram5 bcf
	 0    # meta
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

if($analysis_type =~ /^GA_SC/) {
    $max_time[0] *= 4;
}

if($analysis_type =~ /^2DSA/) {
    $max_time[0] = 10;
    print "2DSA time $max_time[0]\n";
}

$max_time[0] *= 8 if $default_system =~ /hlrb2/;
$max_time[0] *= 4 if $default_system =~ /lonestar/;
$max_time[0] *= 4 if $default_system =~ /ranger/;
$max_time[0] *= 8 if $default_system =~ /queenbee/;
$max_time[0] *= 4 if $default_system =~ /bigred/;

if($monte_carlo) {
    $max_time[0] *= $monte_carlo;
};

if($sa2d_params_use_iterative &&
   $sa2d_max_iterations > 1) {
    $max_time[0] *= 1.5 * $sa2d_max_iterations;
}

if($meniscus_gridpoints) {
    $max_time[0] *= $meniscus_gridpoints;
}

if($fit_ti_noise || $fit_ri_noise) {
    $max_time[0] *= 2;
}

# $max_time[0] = 7 * 60 if $max_time[0] > 7 * 60;

$max_time[0] = 5 if $max_time[0] <= 5;
if($max_time[0] > 2880 &&
   ($default_system eq 'lonestar.tacc.utexas.edu' ||
    $default_system =~ /queenbee/ || 
    $default_system =~ /bigred/ # bigred
    )) {
    $msg = MIME::Lite->new(From    => 'gridcontrol@ultrascan.uthscsa.edu',
			   To      =>  "$email, emre\@biochem.uthscsa.edu",
			   Subject =>  "TIGRE job on $default_system set to maximum time",
			   Data    => 
"Your TIGRE job on $default_system estimated has an estimated run time of " . ($max_time[0]/60) . " which exceeds the maximum 48 hour limit.  
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
    

$max_time[0] = int($max_time[0] + .5);
$max_time[0] = 2880 if $max_time[0] > 2880;
$max_time[5] = $max_time[0];
$max_time[3] = $max_time[0];
$max_time[2] = $max_time[0];
$max_time[1] = $max_time[0];
$max_time[9] = $max_time[0];
$max_time[11] = $max_time[0];  # queenbee
$max_time[12] = $max_time[0];  # bigred
$max_time[13] = $max_time[0];  # ranger
$max_time[14] = $max_time[0];  # new lonestar


print "Maximum time[0]=$max_time[0]\n";

$cmd = "perl $ENV{'ULTRASCAN'}/etc/us_parse_gc.pl $util $util_ti $gcfile\n";
print $cmd;
$np = `$cmd`;
chomp $np;
$np = 2 if $np < 2;

# $np = 31;

$esttime = 120; # we should do better
$hostfile = "$ENV{'ULTRASCAN'}/etc/us_tigre_hosts";

if(!$default_system) {
#    use SelectResource;
#    ($default_system, $np) = SelectResource::select_tigre($hostfile, $np, $max_time[$usesys]);
}

$usesys = $reversesystems{$default_system};

if( $systems[$usesys] ne $default_system) {
    print "warning: system mismatch $systems[$usesys] != $default_system\n" ;
    $default_system = $systems[$usesys];
}

print "using tigre system $default_system\n";

if($default_system ne 'meta' && !$gfac[$usesys]) {
    do {
	$result = `perl $ULTRASCAN/etc/check_tigre.pl $default_system`;
	if($result =~ /ERROR/) {
	    print STDERR "$result";
	    if(!$messagesent) {
		$msg = MIME::Lite->new(From    => 'gridcontrol@ultrascan.uthscsa.edu',
				       To      =>  'emre@biochem.uthscsa.edu, jeremy@biochem.uthscsa.edu, demeler@biochem.uthscsa.edu',
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
			       To      =>  'emre@biochem.uthscsa.edu, jeremy@biochem.uthscsa.edu, demeler@biochem.uthscsa.edu',
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

print "max np $max_np[$usesys]\n";
$np = $max_np[$usesys] if $np > $max_np[$usesys];

print "systems (use_sys) $systems[$usesys]\n";
if( $systems[$usesys] =~ /^(bcf|alamo|laredo)/) {
    print "overriding recommended max_np of $np\n" if $max_np[$usesys] != $np;
    $np = $max_np[$usesys];
}

undef $hc;
if ( $default_system =~ /bigred/ )
{
    $hcn = int(($np / 4) + .9999999);
    $hc = "\n<hostCount>$hcn</hostCount>\n";
    $np = 4 * $hcn;
    print $hc;
}

if( $default_system =~ /gatekeeper.ranger/ )
{
    $np = 16 * ( int( $np / 16 ) + 1);
    $np = $max_np[$usesys] if $np > $max_np[$usesys];
}

$gfac_hc = 0;
if( $default_system =~ /lonestar.tacc.teragrid.org/ )
{
    $np = 12 * ( int( $np / 12 ) + 1);
    $np = $max_np[$usesys] if $np > $max_np[$usesys];
    $gfac_hc = int($np / 12);
}

if( $default_system =~ /bcf.biochemistry.uthscsa.edu/ )
{
    $np = 2 * ( int( $np / 2 ) + 1);
    $np = $max_np[$usesys] if $np > $max_np[$usesys];
    $gfac_hc = int($np / 2);
}

print "np is $np gfac_hc is $gfac_hc\n";
if( $default_system =~ /gatekeeper.ranger/ )
{
    if ( $max_time[$usesys] > 1440 )
    {
	$queues[$usesys] = 'long';
    } else {
	$queues[$usesys] = 'normal';
    }
    print "submitting to $default_system via '$queues[$usesys]' queue\n";
}

if( $default_system =~ /lonestar.tacc.teragrid.org/ )
{
    $queues[$usesys] = 'normal';
    print "submitting to $default_system via '$queues[$usesys]' queue\n";
}
    
$SYSTEM = $systems[$usesys];
$GSI_SYSTEM = $SYSTEM;
$GSI_SYSTEM = "brecca.vpac.monash.edu.au" if $SYSTEM =~ /ng2.vpac.monash.edu.au/;
$GSI_SYSTEM = "gatekeeper.iu.teragrid.org" if $SYSTEM =~ /bigred/;
$GSI_SYSTEM = "tg-login.ranger.tacc.teragrid.org" if $SYSTEM =~ /ranger/;
$PORT_GLOBUS = $ports_globus[$usesys];
$PORT_SSH = $ports_ssh[$usesys];
$WORK = $work[$usesys];
$FACTORYTYPE = $factorytypes[$usesys];
$BIN = $bins[$usesys];
$LD_XML = $ld_xml[$usesys];
$LD_QUEUE = $queues[$usesys];
$PROJECT = "<project>TG-MCB070039N</project>" if
    $default_system eq 'lonestar.tacc.utexas.edu' || 
    $default_system eq 'queenbee.loni-lsu.teragrid.org' ||
    $default_system eq 'gatekeeper.bigred.iu.teragrid.org' 
    ;

$MAXMEM = "<maxMemory>2000</maxMemory>" if $SYSTEM =~ /ng2.vpac.monash.edu.au/;

$WORKTMP = "${WORK}/tmp";
$WORKRUN = "${WORK}/tmp/$id";

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

sub check_staged_files {
    $check_cmd = "$gsi[$usesys]ssh -p $PORT_SSH $GSI_SYSTEM ls ${WORKRUN}/\n"; # experiments${timestamp}.dat ${WORKRUN}/solutes${timestamp}.dat\n";
    print $check_cmd;
    my $retry = 0;
    if ( $execute ) {
	$result = `$check_cmd`;
	print $result;
	while ( 
		! ( $result =~ /experiments${timestamp}.dat/ ) ||
		! ( $result =~ /solutes${timestamp}.dat/ ) )
        {
	    $retry++;
	    my $tosleep = $retry * 30;
	    print "Files not properly staged, retry $retry, sleeping $tosleep\n";
	    sleep $tosleep;
	    $cmd = 
		"$gsi[$usesys]ssh -p $PORT_SSH $GSI_SYSTEM rm -fr $WORKRUN
$gsi[$usesys]ssh -p $PORT_SSH $GSI_SYSTEM mkdir -p $WORKRUN
$gsi[$usesys]scp -P $PORT_SSH $experiment $GSI_SYSTEM:${WORKRUN}/experiments${timestamp}.dat
$gsi[$usesys]scp -P $PORT_SSH $solutes $GSI_SYSTEM:${WORKRUN}/solutes${timestamp}.dat\n";
	    print $cmd;
	    print `$cmd`;
	    print $check_cmd;
	    $result = `$check_cmd`;
	    print $result;
	}
    }
}
    
if($default_system ne 'meta') {
    &check_is_resubmit();
    $cmd = 
	"$gsi[$usesys]ssh -p $PORT_SSH $GSI_SYSTEM rm -fr $WORKRUN
$gsi[$usesys]ssh -p $PORT_SSH $GSI_SYSTEM mkdir -p $WORKRUN
$gsi[$usesys]scp -P $PORT_SSH $experiment $GSI_SYSTEM:${WORKRUN}/experiments${timestamp}.dat
$gsi[$usesys]scp -P $PORT_SSH $solutes $GSI_SYSTEM:${WORKRUN}/solutes${timestamp}.dat\n";
    print $cmd;
    print `$cmd` if $execute;
    &check_staged_files();
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
  <executable>${WORK}/ultrascan/${BIN}/$executable[$usesys]</executable>
  <directory>$WORKRUN</directory>
  <argument>$WORKRUN/experiments${timestamp}.dat</argument>
  <argument>$WORKRUN/solutes${timestamp}.dat</argument>
$jid_in_xml
$LD_XML
  <stdout>$WORKRUN/us_job${id}.stdout</stdout>
  <stderr>$WORKRUN/us_job${id}.stderr</stderr>
  <count>$np</count>$hc
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
	$cmd = "globusrun-ws -submit -batch -term 12/31/2099 -F https://${SYSTEM}:${PORT_GLOBUS}/wsrf/services/ManagedJobFactoryService -factory-type $FACTORYTYPE -f $xmlfile > $eprfile\n";
	print $cmd;
	print `$cmd` if $execute;
	if ($default_system =~ /bcf.uthscsa.edu/) {
	    # special bcf handling 
	    print "special bcf handling\n";
	    # check for immediate fail
	    $ecount = 0;
	    $max_ecount = 10;
	    print "check the nodes\n";
	    $cmd = qq/$gsi[$usesys]ssh bcf.uthscsa.edu 'rcom "uname -n; ls .globus"'\n/;
	    print $cmd;
	    print `$cmd`;
	
	    do {
		print "check submit 0:\n";
		sleep 10;
		$status = `globusrun-ws -status -job-epr-file $eprfile 2> /lustre/tmp/gc_tigre_${id}_globus_stderr`;
		print "status is <$status>\n";
		$stderr = `cat /lustre/tmp/gc_tigre_${id}_globus_stderr`;
		print "stderr is <$stderr>\n";
		print STDERR $stderr;
		if($stderr =~ /SOAP Fault/ &&
		   $status != /^$/) {
		    print "Soap Fault: ignoring bogus status <$status>\n";
		    $status = '';
		}
		$lastfail = 0;
		if(
		   $status =~ /Failed/ ||
		   $status =~ /FAILED/ ||
		   $stderr =~ /failed/ ) {
		    # kill & restart
		    $lastfail = 1;
		    $ecount++;
		    print "resubmitting $ecount of $max_ecount bcf initial job fail\n";
		    $cmd = "globusrun-ws -kill -job-epr-file $eprfile 2>&1";
		    print $cmd;
		    print `$cmd` if $execute;
		    sleep $ecount * 10;
		    &check_is_resubmit();
		    $cmd = "globusrun-ws -submit -batch -term 12/31/2099 -F https://${SYSTEM}:${PORT_GLOBUS}/wsrf/services/ManagedJobFactoryService -factory-type $FACTORYTYPE -f $xmlfile > $eprfile\n";
		    print $cmd;
		    print `$cmd` if $execute;
		}
	    } while($lastfail && $ecount && $ecount <= $max_ecount);
	}
    } else {
	$cmd = "perl $US/etc/us_asta_run.pl $SYSTEM ${WORKRUN}/experiments${timestamp}.dat ${WORKRUN}/solutes${timestamp}.dat $jid $np $max_time[$usesys] $queues[$usesys] $gfac_hc\n";
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
	    $status = `globusrun-ws -status -job-epr-file $eprfile 2> /lustre/tmp/gc_tigre_${id}_globus_stderr`;
	    $stderr = `cat /lustre/tmp/gc_tigre_${id}_globus_stderr`;
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
	    &cancelmsg("Tigre job was canceled or terminated abnormally\n");
	    die "tigre job was canceled or abnormally terminated\n";
	}
    } else {
	$dups_blank = 0;
    }
    if($status eq 'Failed' ||
       $status eq 'FAILED') {
	&if_gfac_set_workrun();
	if($default_system ne 'meta') {
	    if ( $gfac[$usesys] ) {
		if ( length($WORKRUN) ) {
		    &gsiget($GSI_SYSTEM, $PORT_SSH, "${WORKRUN}/UltraScan_MPI_Program.stderr", "/lustre/tmp/us_job${id}.stderr");
		    &gsiget($GSI_SYSTEM, $PORT_SSH, "${WORKRUN}/UltraScan_MPI_Program.stdout", "/lustre/tmp/us_job${id}.stdout");
		} else {
		    $cmd = "$US/etc/us_asta_message.pl $exp_tag\n";
		    $gfac_out = $cmd;
		    $gfac_out .= "GFAC Message:----------\n";
		    $gfac_out .=  `$cmd`;
		    $gfac_out .=  "-----------------------\n";
		    print $gfac_out;
		    open (GFACERR, ">/lustre/tmp/us_job${id}.stderr");
		    print GFACERR $gfac_out;
		    close GFACERR;
		}
	    } else {
		&gsiget($GSI_SYSTEM, $PORT_SSH, "${WORKRUN}/us_job${id}.stderr", "/lustre/tmp/us_job${id}.stderr");
		&gsiget($GSI_SYSTEM, $PORT_SSH, "${WORKRUN}/us_job${id}.stdout", "/lustre/tmp/us_job${id}.stdout");
	    }
		
#	    $cmd = 
#"$gsi[$usesys]scp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/us_job${id}.stderr /lustre/tmp/
#$gsi[$usesys]scp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/us_job${id}.stdout /lustre/tmp/
#";
#	    print $cmd;
#	    print `$cmd` if $execute;
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
# get results ($gsi[$usesys]scp models & emails)

&if_gfac_set_workrun();

sub retrieve_result_files {
# step 1, get list of files in workrun
    print "sub: retrieve results files:\n";
    my $tosleep;
    my $retry = 0;
    my $result;
    my @stderr;
    my @results;
    my $i;
    my @local_files;
    my @missing;

    my $cmd = 
	"$gsi[$usesys]ssh -p $PORT_SSH $GSI_SYSTEM 'bash -c \"ls $WORKRUN 2>/dev/null\"'";
    
    do {
	$tosleep = $retry * 30;
	if ( $retry ) {
	    print "can not read results directory, retry $retry, sleeping $tosleep\n";
	    sleep $tosleep;
	}
	capture sub {
	    system($cmd);
	} => \$result, \$stderr;
	$stderr =~ s/\s*//g;
	$stderr =~ s/Warning:Noxauthdata;usingfakeauthenticationdataforX11forwarding.//g;
	$stderr =~ s/\s*//g;
	$retry++;
	print "$cmd results:\n\tstdout <$result>\n\tstderr <$stderr>\n";
	if ( $retry > $MAX_RETRIES ) {
	    print "ERROR too many retries\n";
	}
    } while (length($stderr) && $retry <= $MAX_RETRIES);

#    print "result <$result>\n";

# prune result files list
    @results = split /\n/, $result;
    grep chomp, @results;
    for($i = 0; $i < @results; $i++) {
	print "$i: $results[$i]\n";
    }
    @results = grep (/(^email_|\.simulation_parameters$|^checkpoint.*dat$|noise|\.model)/, @results);
    for($i = 0; $i < @results; $i++) {
	print "$i: $results[$i]\n";
    }
    my @getlist;
    push @getlist, 'email_*' if grep /^email_/, @results;
    push @getlist, '*.simulation_parameters' if grep /\.simulation_parameters$/, @results;
    push @getlist, 'checkpoint*.dat' if grep /^checkpoint.*dat$/, @results;
    push @getlist, '*noise*' if grep /noise/, @results;
    push @getlist, '*.model*' if grep /\.model/, @results;

    for($i = 0; $i < @getlist; $i++) {
	print "$i: $getlist[$i]\n";
    }

# now get these files
    for ( $i = 0; $i < @getlist; $i++ ) {
	$cmd = 
	    "$gsi[$usesys]scp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/$getlist[$i] .\n";
	print "$cmd";
	$retry = 0;
	do {
	    $tosleep = $retry * 30;
	    if ( $retry ) {
		print "can not get files $getlist[i], retry $retry, sleeping $tosleep\n";
		sleep $tosleep;
	    }
	    capture sub {
		system($cmd);
	    } => \$result, \$stderr;
	    $stderr =~ s/\s*//g;
	    $stderr =~ s/Warning:Noxauthdata;usingfakeauthenticationdataforX11forwarding.//g;
	    $stderr =~ s/\s*//g;
	    $retry++;
	    print "$cmd results:\n\tstdout <$result>\n\tstderr <$stderr>\n";
	    if ( $retry > $MAX_RETRIES ) {
		print "ERROR too many retries\n";
	    }
	} while (length($stderr) && $retry <= $MAX_RETRIES);
    }

# did we get them all?
    @local_files = `ls`;
    @missing;
    grep chomp, @local_files;
    for($i = 0; $i < @results; $i++) {
	push @missing, $results[$i] if !grep(/$results[$i]/, @local_files);
    }
    print "files missing:\n";
    for($i = 0; $i < @missing; $i++) {
	print "$i: $missing[$i]\n";
    }

# reget the missing ones
    for ( $i = 0; $i < @missing; $i++ ) {
	$cmd = 
	    "$gsi[$usesys]scp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/$missing[$i] .\n";
	print "$cmd";
	$retry = 0;
	do {
	    $tosleep = $retry * 30;
	    if ( $retry ) {
		print "can not get files $missing[i], retry $retry, sleeping $tosleep\n";
		sleep $tosleep;
	    }
	    capture sub {
		system($cmd);
	    } => \$result, \$stderr;
	    $stderr =~ s/\s*//g;
	    $stderr =~ s/Warning:Noxauthdata;usingfakeauthenticationdataforX11forwarding.//g;
	    $stderr =~ s/\s*//g;
	    $retry++;
	    print "$cmd results:\n\tstdout <$result>\n\tstderr <$stderr>\n";
	    if ( $retry > $MAX_RETRIES ) {
		print "ERROR too many retries\n";
	    }
	} while (length($stderr) && $retry <= $MAX_RETRIES);
    }

# check one more time
    @local_files = `ls`;
    undef @missing;
    my @missing;
    grep chomp, @local_files;
    for($i = 0; $i < @results; $i++) {
	push @missing, $results[$i] if !grep(/$results[$i]/, @local_files);
    }
    if ( @missing ) {
	print "ERROR: files still missing:\n";
	for($i = 0; $i < @missing; $i++) {
	    print "$i: $missing[$i]\n";
	}
    }
}

if($default_system eq 'meta') {
    $cmd = 
"mv us_job${id}.stderr /lustre/tmp/us_job${id}.stderr
mv us_job${id}.stdout /lustre/tmp/us_job${id}.stdout
";
    print $cmd;
    print `$cmd` if $execute;
} else {
    if ( $gfac[$usesys] ) {
	&gsiget($GSI_SYSTEM, $PORT_SSH, "${WORKRUN}/UltraScan_MPI_Program.stderr", "/lustre/tmp/us_job${id}.stderr");
	&gsiget($GSI_SYSTEM, $PORT_SSH, "${WORKRUN}/UltraScan_MPI_Program.stdout", "/lustre/tmp/us_job${id}.stdout");
    } else {
	&gsiget($GSI_SYSTEM, $PORT_SSH, "${WORKRUN}/us_job${id}.stderr", "/lustre/tmp/us_job${id}.stderr");
	&gsiget($GSI_SYSTEM, $PORT_SSH, "${WORKRUN}/us_job${id}.stdout", "/lustre/tmp/us_job${id}.stdout");
    }
#    $cmd = "$gsi[$usesys]scp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/us_job${id}.stderr /lustre/tmp/
#    $gsi[$usesys]scp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/us_job${id}.stdout /lustre/tmp/
    &retrieve_result_files();

#    $cmd =
#	"$gsi[$usesys]scp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/email_* .
#$gsi[$usesys]scp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/*.model* .
#$gsi[$usesys]scp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/*noise* .
#$gsi[$usesys]scp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/*.simulation_parameters .
#$gsi[$usesys]scp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/checkpoint*.dat .
# ";
}

print "----tail us_job${id}.stderr --- from $default_system ---\n";
print `tail /lustre/tmp/us_job${id}.stderr` if $execute;
print "----end us_job${id}.stderr ---\n";

# check for 'job complete'
$cmd = "grep -l 'job complete' /lustre/tmp/us_job${id}.stdout";
$res = `$cmd`;
chomp $res;
print "$cmd: $res\n";
if ( $res ne "/lustre/tmp/us_job${id}.stdout" ) 
{
    &failmsg("Your job possibly terminated without a 'job complete' status");
# don't die until we have tested this a bit
#    if($default_system eq 'meta') {
#	print `echo tigre_job_end $grms_id > $ENV{'ULTRASCAN'}/etc/us_gridpipe`;
#    } else {
#	print `echo tigre_job_end $cwd/$eprfile > $ENV{'ULTRASCAN'}/etc/us_gridpipe`;
#    }
#    die "tigre job failed submission on $default_system\n";
}    

# email results
print "emailing results\n";
print `cp email_text_* email_msg
echo "Results processed by TIGRE on $default_system" >> email_msg`;
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

    @text = `perl $ULTRASCAN/etc/us_old_format.pl email_text_$tid`;
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
