#!/usr/bin/perl

$|=1;

use MIME::Lite;

sub do_email {
    $em = MIME::Lite->new(From    => 'gridcontrol@ultrascan.uthscsa.edu',
			   To      =>  'ebrookes@cs.utsa.edu, jeremy@biochem.uthscsa.edu, demeler@biochem.uthscsa.edu',
			   Subject =>  "TIGRE ERROR on $system for user apache",
			   Data    => "us_check_tigre noticed the following problem
--------------------------------------------------------------------------------
$_[0]
--------------------------------------------------------------------------------
$_[1]
" );
  
    $em->send('smtp', 'smtp.uthscsa.edu');
}

    
$system = shift;
if($system eq '-e') {
    $email++;
    $system = shift;
}

$system .= ".uthscsa.edu" if !($system =~ /\./);
$system = 'bcf.uthscsa.edu' if !$system;

$proxy = `grid-proxy-info 2>&1`;

if($proxy =~ /ERROR/) {
    $msg = "ERROR: no valid proxy. run 'grid-proxy-init'\n";
    print $msg;
    &do_email($proxy, $msg) if $email;
    exit(-1);
}

( $timeleft ) = $proxy =~ /timeleft : (.*)$/;

( $hh, $mm, $ss ) = $timeleft =~ /(\d*):(\d\d):(\d\d)/;

# print "$hh:$mm:$ss\n";

if($hh == 0) {
    if($mm < 10) {
	$msg = "ERROR: insufficient time left on grid-proxy $timeleft. run 'grid-proxy-init'\n";
	print $msg;
	&do_email($proxy, $msg) if $email;
	exit(-10);
    }
    if($mm < 30) {
	$msg = "WARNING: minimal time left on grid-proxy $timeleft. run 'grid-proxy-init'\n";
	print $msg;
	&do_email($proxy, $msg) if $email;
    }
} else {
    print "proxy is good for another $timeleft\n";
}


$datefcmd = 'date +%F%H%M%S%N';
$datef = `$datefcmd`;
chomp $datef;
#print "datef: <$datef>\n";
$date = `date`;
chomp $date;
#print "date: <$date>\n";

@systems = (
	    'lonestar.tacc.utexas.edu' , 
	    'cosmos.tamu.edu' ,
	    'antaeus.hpcc.ttu.edu' ,
	    'gridgate.rtc.rice.edu' ,
	    'eldorado.acrl.uh.edu' ,
	    'a01.hlrb2.lrz-muenchen.de' ,
	    'bcf.uthscsa.edu' ,
	    'alamo.uthscsa.edu' ,
	    'laredo.uthscsa.edu' ,
	    'ng2.vpac.monash.edu.au'
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
		 8443 , 
		 9443 ,
		 9443 ,
		 9443 ,
		 8443 
		 );

@ports_ssh = (
	      22 ,
	      2222 ,
	      49922 ,
	      22 ,
	      22 ,
	      2222 ,
	      22 ,
	      22 ,
	      22 ,
	      22
	      );

$home[$reversesystems{'ng2.vpac.monash.edu.au'}] = "/home/grid-ultrascan/";
$gsi_system = $system;
$gsi_system = "brecca.vpac.monash.edu.au";

$gsissh = `gsissh -p $ports_ssh[$reversesystems{$system}] -v $gsi_system echo $home[$reversesystems{$system}]test123 2>&1`;
if(!($gsissh =~ /test123/)) {
    $msg = "ERROR: The command 'gsissh $gsi_system echo test123' failed as follows:\n$gsissh\n";
    print $msg;
    &do_email($msg, '') if $email;
    exit(-1);
}
    
print "gsissh ok\n";

$globusruncmd = 
"globusrun-ws -submit -term 12/31/2099 -F https://${system}:$ports_globus[$reversesystems{$system}]/wsrf/services/ManagedJobFactoryService -c $home[$reversesystems{$system}]ultrascan/etc/datetest '$date' $home[$reversesystems{$system}]$datef 2>&1\n";
print $globusruncmd;
$globusrun = `$globusruncmd`;

if($system =~ /lonestar/) {
    print "globusrun-ws default factory ok\n";
    exit;
}
    
#if(!($globusrun =~ /Active/ && $globusrun =~ /Done/)) {
if(!($globusrun =~ /Done/)) {
    $msg = "ERROR: The globusrun test failed as follows:\n$globusrun\n";
    print $msg;
    &do_email($msg, '') if $email;
    exit(-1);
}


$gsissh = `gsissh -p $ports_ssh[$reversesystems{$system}] -v $gsi_system cat $home[$reversesystems{$system}]$datef 2>&1`;
if(!($gsissh =~ /test $date endtest/)) {
    $msg = "ERROR: gsissh did not return the correct file.  Output follows:\n$gsissh\n";
    print $msg;
    &do_email($msg, '') if $email;
    exit(-1);
}
print "globusrun-ws default factory ok\n";
#print "gsissh returned the correct data\n";

$gsissh = `gsissh -p $ports_ssh[$reversesystems{$system}] -v $gsi_system rm $datef 2>&1`;

__END__

