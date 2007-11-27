#!/usr/bin/perl

sub extract_usage {
    open(RSS, "grep rss $_[0] | awk '{ print \$4 }' | sort -n | tail -1 |"); 
    $maxrss = <RSS>;
    close(RSS);
    chomp $maxrss;

    open(JT, "grep 'job time' $_[0] | awk '{ print \$4 }' |"); 
    $jobtime = <JT>;
    close JT;
    chomp $jobtime;
    $jobtime /= 1000000;
    $extract_usage_title = "maxrss|jobtime";
    $extract_usage = "$maxrss|$jobtime";
}

return 1;
# comment __END__ for testing
__END__
$file = shift;
&extract_usage($file);
print "<$extract_usage_title>\n<$extract_usage>\n";


