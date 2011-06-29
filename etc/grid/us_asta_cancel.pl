#!/usr/bin/perl -w
use SOAP::Lite;

# Parameter name
my $experimentID = $ARGV[0];

open(OUT, ">>/lustre/tmp/us_asta_cancel.log");
print OUT "$0 $experimentID\n";
close OUT;

# Webserice parameters
my $WSDL = 'http://localhost:5680/axis2/services/CancelJob?wsdl';
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
