#!/usr/bin/perl

$notes = "usage: $0 filename(s)
outputs a .xyz file of CA's
";

while ( my $f = shift )
{
    die "$0: $f not found\n" if !-e $f;

    my $c;
    my $out;

    open IN, $f || die;
    @l = <IN>;
    close IN;


    for ( $i = 0; $i < @l; ++$i )
    {
        $t = $l[ $i ];
        if ( $t =~ /^REMARK/ )
        {
            next;
        }

        if ( $t =~ /^END/ )
        {
            next;
        }
        
        ( $atom, $chain, $resno, $x, $y, $z ) = $t =~ /^.{12}(.{4}).{5}(.)(.{4}).{4}(.{8})(.{8})(.{8})/;
        $atom =~ s/^ *//;
        $atom =~ s/ *$//;
        $resno =~ s/^ *//;
        $x =~ s/^ *//;
        $y =~ s/^ *//;
        $z =~ s/^ *//;

        next if $atom ne 'CA';
        $c++;
        $out .= "C $x $y $z\n";
    }


    $fo = $f;
    $fo =~ s/\.pdb$//;
    $fo .= '.xyz';

    print ">$fo\n";
    open OUT, ">$fo" || die;
    print OUT "$c\n\n$out";
    close OUT;
}
