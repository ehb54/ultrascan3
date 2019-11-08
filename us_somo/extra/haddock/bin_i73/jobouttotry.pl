#!/usr/bin/perl

while ( $f = shift )
{
    open IN, $f || die;
    @l = <IN>;
    close IN;
    for ( $i = 0; $i < @l; ++$i )
    {
        ( $try, $w ) = $l[ $i ] =~ /^(try1\d)_\d+(w|)-/;
        print "<$l[$i]>\n" if $debug;
        print "$try $w\n" if $debug;

        $out{ "$try$w" } .= $l[ $i ];
    }
}

foreach $k ( keys %out )
{
    $f = "$k.cluster";
    print ">$f\n";
    open OUT, ">$f" || die;
    print OUT $out{ $k };
    close OUT;
}

