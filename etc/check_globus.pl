#!/usr/local/bin/perl

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
v}

$arg = shift;

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


@systems = (
	    'lonestar.tacc.utexas.edu' , 
	    'cosmos.tamu.edu' ,
	    'minigar.hpcc.ttu.edu' ,
	    'gridgate.rtc.rice.edu' ,
	    'eldorado.acrl.uh.edu' ,
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
		 8443 ,
		 9443 ,
		 9443 ,
		 9443 ,
		 9443 ,
		 9443 ,
		 8443 ,
		 );

@ports_ssh = (
	      22 ,
	      22 ,
	      40022 ,
	      22 ,
	      22 ,
	      22 ,
	      22 ,
	      22 ,
	      22
	      );

$home[$reversesystems{'ng2.vpac.monash.edu.au'}] = "/home/grid-ultrascan/";


$globusruncmd = 
    "globusrun-ws -submit -F https://${system}:$ports_globus[$reversesystems{$system}]/wsrf/services/ManagedJobFactoryService -c /bin/touch $home[$reversesystems{$system}]me$arg\n";
print $globusruncmd;
print `$globusruncmd`;

__END__

