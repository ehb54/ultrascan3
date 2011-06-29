#!/usr/bin/perl -w 

use strict;
use SOAP::Lite;
# To debug soap request and response
SOAP::Lite->import(+trace => qw(debug));
use Data::Uniqid qw ( suniqid uniqid luniqid );

# GFAC Location. Need to change based on where we host gfac but going to be one most of the time
my $WSDL; 
my $ENDPOINT;

my $gfactarget = "gw33";  # or bcf, when working

if ( $gfactarget eq "bcf" )
{
    $WSDL = 'http://localhost:5678/?wsdl';
    $ENDPOINT = 'http://localhost:5678/';
}

if ( $gfactarget eq "gw33" )
{
    $WSDL = 'http://gw33.quarry.iu.teragrid.org:5678/?wsdl';
    $ENDPOINT = 'http://gw33.quarry.iu.teragrid.org:5678/';
}

# Parameter setting. Paramerters passed to run the job. These are used to create soap request.
#Host to run the job

# File locations on the resource. Make sure files are transferred to the resource and we are passing the current file name.

my $hostName = shift;                                                            # my $hostName = 'tg-login.ranger.tacc.teragrid.org'; 'queenbee.loni-lsu.teragrid.org';
my $experimentFileLoc = shift;                                                   # '/work/rfnu/experiments091204143912.dat';
my $solutesFileLoc = shift;                                                      # '/work/rfnu/solutes091204143912.dat';
my $expParam = shift;                                                            # '777'
my $noofProcess = shift;                                                         # '32'
my $walltime = shift || die "usage: $0 hostname experimentfileloc solutefileloc jobid np walltime queuename\n"; # '200';
my $queueName = shift;
my $noofHosts = shift;

my $id = uniqid;
my $experimentID = "Experiment-$id" ;

# Soap call
my $soap = SOAP::Lite->service($WSDL) 
	->proxy($ENDPOINT)
	->on_fault(sub { 
	# SOAP fault handler
        my $soap = shift;
        my $res = shift;
        # Map faults to exceptions
        if(ref($res) eq '') {
            die($res);
        } else {
            die($res->faultstring);
        }
        return new SOAP::SOM;
    	}
      );

# Building SOAP Body
my $method = SOAP::Data
       ->name("Runmpi_InputParams")
       ->prefix("n1")
       ->uri("http://www.extreme.indiana.edu/namespaces/2004/01/gFac");
# Building SOAP Body parameters
my $XMLData = "<n1:ExperimentDataLocation>$experimentFileLoc</n1:ExperimentDataLocation>\n<n1:SolutesDataLocation>$solutesFileLoc</n1:SolutesDataLocation>\n<n1:Param>$expParam</n1:Param>";
my $SOAPHeader = "<wsa:To xmlns:wsa=\"http://www.w3.org/2005/08/addressing\"
            >$ENDPOINT</wsa:To>
        <wsa:Action xmlns:wsa=\"http://www.w3.org/2005/08/addressing\"
            >http://www.extreme.indiana.edu/namespaces/2004/01/gFac/UltraScan_MPI/Runmpi</wsa:Action>
        <wsa:ReplyTo xmlns:wsa=\"http://www.w3.org/2005/08/addressing\">
            <wsa:Address>$ENDPOINT</wsa:Address>
        </wsa:ReplyTo>
        <wsa:MessageID xmlns:wsa=\"http://www.w3.org/2005/08/addressing\"
            >uuid:40bfbf00-4335-11df-842d-87be0134de67</wsa:MessageID>
	<lh:context
              xmlns:lh=\"http://lead.extreme.indiana.edu/namespaces/2005/10/lead-context-header\" xmlns:wsa=\"http://www.w3.org/2005/08/addressing\">
            <lh:experiment-id>$experimentID</lh:experiment-id>
            <lh:event-sink-epr>
                <wsa:Address>http://ogceportal.iu.teragrid.org:12346/topic/$experimentID</wsa:Address>
            </lh:event-sink-epr>
            <lh:user-dn>/C=US/O=National Center for Supercomputing Applications/CN=Raminderjeet Fnu</lh:user-dn>
            <lh:workflow-instance-id>$experimentID</lh:workflow-instance-id>
           <lh:workflow-time-step>0</lh:workflow-time-step> <lrm:resource-mapping ";
my $headercond ='';
if($walltime != 0){
        $headercond = $headercond. "max-wall-time=\"$walltime\" ";
}
if($noofProcess != 0){
        $headercond = $headercond. "node-count=\"$noofProcess\" ";
}
if ($noofHosts !=''){
        $headercond = $headercond. "cpu-count=\"$noofHosts\" ";
}
if($queueName ne ''){
        $headercond = $headercond. "queue-name=\"$queueName\" ";
}

$SOAPHeader = $SOAPHeader. $headercond. " xmlns:lrm=\"http://lead.extreme.indiana.edu/namespaces/2006/lead-resource-mapping/\">$hostName</lrm:resource-mapping>   
</lh:context>";

my $header = SOAP::Header-> type(xml => $SOAPHeader);

my @params = (
   $header,
   SOAP::Data->type(xml => $XMLData)
 );


my $result = $soap-> call($method => @params);

print "$experimentID\n";

