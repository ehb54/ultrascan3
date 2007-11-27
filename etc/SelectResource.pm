
#
# select_tigre is a simple resource selector that selects a tigre resource to run a job
#
# Inputs:
#   hostfile - a file containing candidate compute resource names
#   reqcpus  - the number of cpus requested
#   esttime  - the estimated run time of the application
#
# Output is a list containing 2 values:
#   hostname - name of selected resource (empty string if no resource was found)
#   numcpus  - number of cpus to be used (for now, this is always equal to the requested number of CPUs) 
#

package SelectResource;

use SOAP::Lite;             # MGSI SOAP interface
use XML::DOM;

$statefile = "$ENV{'ULTRASCAN'}/etc/.tigre_lsr";
$stype = "static";
$ltype = "load";
$VO = "TIGRE";
$url = "http://cuero.tacc.utexas.edu:12080/gpir/webservices";

@hostlist = ();
@hostcpus = ();
@hoststatus = ();
$max_hosts;

sub select_tigre
{

my ($hostfile, $reqcpus, $esttime) = @_;

if (not defined $hostfile or
    not defined $reqcpus or
    not defined $esttime ) {
        return ("", $reqcpus);
}

if (not (-e $hostfile))  {
#   print "'$hostfile' not found\n";
    return ("", $reqcpus);
}

# Read host list
$max_hosts = read_hostnames($hostfile);
return ("", $reqcpus) if ($max_hosts <= 0);

# Query GPIR Database for Static and Load information
$soap = SOAP::Lite
  -> uri('GPIRQuery')
  -> proxy($url);

my $static_result = $soap->getQueryByVo($stype,$VO);
if ($static_result->fault) {
   print "Could not get static resource information from GPIR\n";
   print join ', ',
       $static_result->faultcode,
       $static_result->faultstring;
   print "\n";
}

my $load_result = $soap->getQueryByVo($ltype,$VO);
if ($load_result->fault) {
   print "Could not get resource load information from GPIR\n";
   print join ', ',
       $load_result->faultcode,
       $load_result->faultstring;
   print "\n";
}

# Parse XML strings and extract resource information
parse_static($static_result->result());
parse_load($load_result->result());

#dumptables();

# select resource
$last_host_idx = get_last_host($statefile);
$selected_host = select_host($last_host_idx, $reqcpus);
set_last_host($statefile, $hostlist[$selected_host]);

my @retval = ($hostlist[$selected_host], $reqcpus);
return @retval;
}


# subroutines

sub read_hostnames
{
   my ($filename) = @_;
   open(HOSTFILE, $filename) || die "couldn't open file $filename!";

   my $numhosts = 0;
   while ($name = <HOSTFILE>) {
      chomp($name);
      next if ($name =~ /^#/);
      ($hname) = ($name =~ m/(\S+)/);
      $hostlist[++$#hostlist] = $hname;
      $numhosts++;
   }
   close(HOSTFILE);
   for (my $i=0; $i <= $#hostlist; $i++) {
      $hoststatus[$i] = "up";
      $hostcpus[$i] = 128;
   }
   return $#hostlist +1;
}

sub parse_static
{
  my ($xmlstr) = @_;

  my $parser = new XML::DOM::Parser;
  my $doc = $parser->parse ($xmlstr);

  my $resource_nodeList = $doc->getElementsByTagName("ComputeResourceStatic");

  for(my $i=0; $i< $resource_nodeList->getLength; $i++){
      $el_resource = $resource_nodeList->item($i);
      my $hostname = $el_resource->getAttribute("hostname");
      if ((my $index = inhostlist($hostname)) >= 0) {
          my @NumProcs_nodes = $el_resource->getElementsByTagName("NumProcessors");
          if(defined $NumProcs_nodes[0]){
            $num_procs = $NumProcs_nodes[0]->getFirstChild->getData();
          } else {
            $num_procs = 0;
          }
          $hostcpus[$index] = $num_procs;
      }
  }
}

sub parse_load
{
  my ($xmlstr) = @_;

  my $parser = new XML::DOM::Parser;
  my $doc = $parser->parse ($xmlstr);

  my $loadinfo_nodeList = $doc->getElementsByTagName("LoadInfo");

  for(my $i=0; $i< $loadinfo_nodeList->getLength; $i++){
      $el_loadinfo = $loadinfo_nodeList->item($i);
      if (defined $el_loadinfo) {
         my $hostname = $el_loadinfo->getAttribute("hostname");
         if ((my $index = inhostlist($hostname)) >= 0) {
             my $status = $el_loadinfo->getAttribute("status");
             $hoststatus[$index] = $status;
         } 
      }
  }
}

sub inhostlist
{
  my ($hostname) = @_;

  for (my $i=0; $i<$max_hosts; $i++) {
    if ($hostname eq $hostlist[$i]) {
       return $i;
    }
  }
  return (-1);
}

sub dumptables
{
  for (my $i=0; $i < $max_hosts; $i++) {
    print "$hostlist[$i] $hostcpus[$i] $hoststatus[$i]\n";
  }
}

sub select_host
{
  my ($lastidx, $reqcpus) = @_;

  my $idx = ($lastidx+1)%$max_hosts;
  my $i = 0;
  my $selectidx= -1;
  while (($i < $max_hosts)&&($selectidx<0)) {
     if (($hoststatus[$idx] eq "up") && ($hostcpus[$idx] >= $reqcpus)) {
        $selectidx = $idx;
     }
     $i++;
     $idx = ($idx + 1)%$max_hosts;
  }
  $selectidx = 0 if (selectidx < 0);
  return $selectidx;
}

sub get_last_host
{
  my ($fn) = @_;
  return 0 if (not (-r $fn));
  open(STATEFILE, $fn);
  my $hname = <STATEFILE>;
  close STATEFILE;
  my $hidx = inhostlist($hname); 
  if ($hidx < 0) {$hidx = 0;}
  return ($hidx);
}

sub set_last_host
{
  my ($fn, $hname) = @_;
  if (open (STATEFILE, ">$fn")) {
     print STATEFILE $hname;
     close STATEFILE;
  }
}

1;
