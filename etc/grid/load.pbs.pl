#!/usr/bin/perl

##########################################################################################
# ATTENTION!!!                                                                           #
# This module is provided as an example and may need to be modified for your environment #
#                                                                                        #
# This script utilizes the pbs pbsnodes command to obtain the ratio of free processors	 #
# to total processors on a resource.							 #
# THIS SCRIPT ASSUMES THE RESOURCE IS A DUAL-CPU PER NODE CLUSTER!!!			 # 
# It is preferable to go directly to the scheduler to obtain this information, but in	 #
# instances where the scheduler showq command is unavailable, this script will obtain	 #
# a reasonably accurate load estimate for a dual-cpu per node resource.			 #
##########################################################################################


$PROCS_PER_NODE = 2;

$busy_exclusive = `/opt/torque/bin/pbsnodes -a | grep -c 'job-exclusive'`;
$busy_nonexclusive = `/opt/torque/bin/pbsnodes -a | grep -c 'jobs ='`;
$total_nodes = `/opt/torque/bin/pbsnodes -a | grep 'state =' | grep -cv 'offline'`;

$busy_single = $busy_nonexclusive - $busy_exclusive;

$procs_busy = $busy_single + ($busy_exclusive * $PROCS_PER_NODE);
$procs_total = $total_nodes * $PROCS_PER_NODE;


### calculate the load
$load_result = "";
if($procs_total == 0){
       	$load_result = 0;
}else{
       	$load_result = $procs_busy / $procs_total;
}
$load_result = int($load_result*100);

print "$load_result\n";

