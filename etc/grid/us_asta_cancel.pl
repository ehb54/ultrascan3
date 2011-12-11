#!/usr/bin/perl -w
use SOAP::Lite;

# Parameter name
my $experimentID = $ARGV[0];

my $us = $ENV{'ULTRASCAN'} || die "The environment variable ULTRASCAN must be set.  Terminating\n";
my $logfiledir = "$us/etc/grid/log";    

open(OUT, ">>$logfiledir/us_asta_cancel.log");
print OUT "$0 $experimentID\n";
close OUT;

# Webserice parameters
my $WSDL = 'http://gf5.ucs.indiana.edu:7070/axis2/services/CancelJob?wsdl';
my $namespace = 'http://jobsubmittion.ogce.org';

# Soap call
my $soap = SOAP::Lite->proxy($WSDL)
 		     ->uri($namespace)
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

#- Calling the RPC
$result = $soap -> cancelJob($experimentID) -> result;

#- Showing the result
print "$result\n";
open(OUT, ">>$logfiledir/us_asta_cancel.log");
print OUT "$0 $experimentID cancel result message: $result\n";
close OUT;
