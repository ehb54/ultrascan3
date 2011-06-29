#!/usr/bin/perl

$system = shift;
$count = shift || die "usage: $0 systemname count\n";

$system .= ".uthscsa.edu" if !($system =~ /\.uthscsa\.edu/);

$xmlfile = "/tmp/_check_tigre_np$count$system.xml";
$stdoutfile = "_check_tigre_np$count$system.stdout";
$stderrfile = "_check_tigre_np$count$system.stderr";
unlink $stdoutfile;
unlink $stderrfile;
$xml = "<job>
  <executable>/home/tigre/bin/mpihello</executable>
  <directory>/home/tigre/</directory>
  <environment>
  <name>BLAH2</name>
  <value>123</value>
  </environment>
  <stdout>$stdoutfile</stdout>
  <stderr>$stderrfile</stderr>
  <count>$count</count>
  <maxWallTime>240</maxWallTime>
  <jobType>mpi</jobType>
</job>
";

open(XML, ">$xmlfile");
print XML $xml;


$globusrun = "globusrun-ws -submit -F https://${system}:9443/wsrf/services/ManagedJobFactoryService -factory-type PBS -f $xmlfile\n";

print $globusrun;
system($globusrun);
`rm /tmp/$stdoutfile /tmp/$stderrfile 2> /dev/null`;

$gsiscp = "gsiscp $system:$stdoutfile /tmp/$stdoutfile
gsiscp $system:$stderrfile /tmp/$stderrfile";

`$gsiscp`;

print `echo ----stdout----
cat /tmp/$stdoutfile
echo ----stderr----
cat /tmp/$stderrfile
echo --------------
`;


