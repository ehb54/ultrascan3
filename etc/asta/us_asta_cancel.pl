#!/usr/bin/perl -w
use SOAP::Lite;

# Parameter name
my $experimentID = $ARGV[0];

open(OUT, ">>/tmp/us_asta_cancel.log");
print OUT "$0 $experimentID\n";
close OUT;

# Webserice parameters
my $WSDL = 'http://gw33.quarry.iu.teragrid.org:8080/axis2/services/CancelJob?wsdl';
my $namespace = 'http://jobsubmittion.ogce.org';

# Soap call
my $soap = SOAP::Lite->proxy($WSDL)
 		     ->uri($namespace);
#- Calling the RPC
$result = $soap -> cancelJob($experimentID);

#- Showing the result
print "$result\n";

