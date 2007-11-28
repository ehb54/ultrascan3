#!/usr/local/bin/perl

$system = shift;
$port = shift;
$dir = '/opt/torque/bin';

$nodes_cmd = "ssh $system $dir/pbsnodes -s $system:$port | grep ' np = ' | awk '{ tot += \$3 } END { print tot }'";
$running_cmd = "ssh $system $dir/qstat \@$system:$port | awk '{ print \$5 }' | grep R | wc -l";
$queue_cmd = "ssh $system $dir/qstat \@$system:$port | awk '{ print \$5 }' | grep Q | wc -l";

$nodes = `$nodes_cmd`;
chomp $nodes;
$running = `$running_cmd`;
chomp $running;
$queue = `$queue_cmd`;
chomp $queue;
print "$running/$queue\n";


