#!/usr/bin/perl -w

$us = $ENV{'ULTRASCAN'} || die "The environment variable ULTRASCAN must be set.  Terminating\n";

# this currently works only for lonestar
$PORT_SSH = 22;
$default_system = $GSI_SYSTEM = "lonestar.tacc.utexas.edu";

$WORK = '/work/teragrid/tg457210/tmp/';

while ( $id = shift ) {
    chdir "$us/etc";
    $eprline = `perl us_gridpipe_db_dump.pl | grep '<$id>'`;
    chomp $eprline;
#    <1636>:</var/www/html/ultrascan/cauma/data/f0a7f2d5363788abba0157e93d482cdc/090522081217/us_tigre_epr090522081217.xml>
    ( $base, $ts ) = $eprline =~ />:<(.*\/)(\d+)\/us_tigre_epr/;
    if ( !$ts )
    {
	print "$id not found in us_gridpipe.db\n";
	next;
    }
    $WORKRUN = "$WORK$ts";
    print "$id: <$base><$ts><$WORKRUN>\n";
    chdir "$base/$ts" || die "couldn't chdir $base/$ts";
    $cmd = "
gsiscp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/us_job${ts}.stdout /lustre/tmp/
gsiscp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/email_* .
gsiscp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/*.model* .
gsiscp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/*noise* .
gsiscp -P $PORT_SSH ${GSI_SYSTEM}:${WORKRUN}/*.simulation_parameters .
" ;
    print $cmd;
    print `$cmd`;
    print `cp email_text_* email_msg
echo "Results processed by TIGRE on $default_system" >> email_msg`;
    $cmd =
"perl $us/bin64/us_email.pl email_list_* email_msg
";
    print $cmd;
    print `$cmd`;
}

