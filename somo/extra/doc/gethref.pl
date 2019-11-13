#!/usr/bin/perl

while ( $f = shift )
{
    open IN, $f || die "open $f\n";
    @l = <IN>;
    close IN;
    $document = join '', @l;
    while ( $document =~ m/\s+(?:href|src)\s*=\s*"([^"\s]+)"/gi ) 
    {
       next if $1 =~ /^mailto:/;
       print "$1\n";
    }
}
