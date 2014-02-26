#!/usr/bin/perl

$notes =
    "usage: $0 files {file2 {...}}
removed chainid & segid
";

die $notes if ! scalar @ARGV;

while ( $f = shift )
{
    undef %v;
    die "$0: $f does not exist\n" if !-e $f;
    open IN, $f || die "$0: error opening $f $!\n";

    @l = <IN>;

    close IN;

    for ( $i = 0; $i < @l; ++$i )
    {
        if ( $l[ $i ] =~ /^(ATOM|HETATM)/ )
        {
            $l[ $i ] = substr( $l[ $i ], 0, 21 ) . " " . substr( $l[ $i ], 22, 51 ) . "\n";
        }
    }

    mkdir "tmp";
    print ">tmp/$f\n";
    open OUT, ">tmp/$f" || die "$0: error opening tmp/$f $!\n";

    print OUT join '', @l;

    close OUT;
}
