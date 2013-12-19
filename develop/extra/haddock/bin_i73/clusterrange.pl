#!/usr/bin/perl

while ( $f = shift )
{
    open IN, $f || die;
    @l = <IN>;
    close IN;
    my $min = 1e99;
    my $max = -1e99;
    for ( $i = 0; $i < @l; ++$i )
    {
        @j = split /\s+/, $l[$i];
        $j[ 0 ] =~ s/(B|C)$//;
        $j[ 1 ] =~ s/(B|C)$//;
        next if $j[ 0 ] eq $j[ 1 ];

        if ( $min > $j[ 2 ] )
        {
            $min = $j[ 2 ];
        }
        if ( $max < $j[ 2 ] )
        {
            $max = $j[ 2 ];
        }
    }
    print "$f min $min max $max\n";
}


