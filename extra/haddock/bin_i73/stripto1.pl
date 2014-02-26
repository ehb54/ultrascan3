#!/usr/bin/perl

$notes = "usage: $0 filename
special run to strip off one of the 2 tgfb dimers that is furthest from the B chain

so chain A has 1-112 as one dimer, B has 113-224 as the other
find centroid CA's of A:1 A:2 and centroid of CA's of B and see which is closer, strip off other
remove 45-79 of A from centroid computation
";

$f = shift || die $notes;
die "$0: $f not found\n" if !-e $f;

open IN, $f || die;
@l = <IN>;
close IN;


for ( $i = 0; $i < @l; ++$i )
{
    $t = $l[ $i ];
    if ( $t =~ /^REMARK/ )
    {
        $hdr .= $t;
        next;
    }

    if ( $t =~ /^END/ )
    {
        $foot .= $t;
        next;
    }
    
    ( $atom, $chain, $resno, $x, $y, $z ) = $t =~ /^.{12}(.{4}).{5}(.)(.{4}).{4}(.{8})(.{8})(.{8})/;
    $atom =~ s/^ *//;
    $atom =~ s/ *$//;
    $resno =~ s/^ *//;
    $x =~ s/^ *//;
    $y =~ s/^ *//;
    $z =~ s/^ *//;

#    print "atom <$atom> resno <$resno> $chain <$chain> coord<$x:$y:$z>\n";
#    exit if $c++ > 10;

    $cu = $chain;
    if ( $chain eq 'A' )
    {
        if ( $resno <= 112 )
        {
            $cu = "A:1";
        } else {
            $cu = "A:2";
            $resno -= 112;
            $xresno = ' 'x( 4 - length( $resno ) ) . $resno;
            $t = substr( $t, 0, 22 ) . $xresno . substr( $t, 26 );
        }
    }

    $out{ $cu } .= $t;

    if ( $atom eq 'CA' &&
         ( $chain ne 'A' ||
           $resno < 45 ||
           $resno > 79 )
        )
    {
        $x{ $cu } += $x;
        $y{ $cu } += $y;
        $z{ $cu } += $z;
        $c{ $cu }++;
        if ( $chain eq 'B' )
        {
            push @xb, $x;
            push @yb, $y;
            push @zb, $z;
        }
    }
}

foreach $k ( keys %c )
{
    $x{ $k } /= $c{ $k };
    $y{ $k } /= $c{ $k };
    $z{ $k } /= $c{ $k };

    print sprintf( "centroid CA's of $k %.3f %.3f %.3f\n", $x{ $k }, $y{ $k }, $z{ $k } );
}

$d1 = 
    sqrt( 
        ( $x{ 'B' } - $x{ 'A:1' } ) * ( $x{ 'B' } - $x{ 'A:1' } ) +
        ( $y{ 'B' } - $y{ 'A:1' } ) * ( $y{ 'B' } - $y{ 'A:1' } ) +
        ( $z{ 'B' } - $z{ 'A:1' } ) * ( $z{ 'B' } - $z{ 'A:1' } ) 
    );

$d2 = 
    sqrt( 
        ( $x{ 'B' } - $x{ 'A:2' } ) * ( $x{ 'B' } - $x{ 'A:2' } ) +
        ( $y{ 'B' } - $y{ 'A:2' } ) * ( $y{ 'B' } - $y{ 'A:2' } ) +
        ( $z{ 'B' } - $z{ 'A:2' } ) * ( $z{ 'B' } - $z{ 'A:2' } ) 
    );

# alt method, find closest CA of B to each

for ( $i = 0; $i < @xb; ++$i )
{
    $bd1 = 
        sqrt( 
            ( $xb[ $i ] - $x{ 'A:1' } ) * ( $xb[ $i ] - $x{ 'A:1' } ) +
            ( $yb[ $i ] - $y{ 'A:1' } ) * ( $yb[ $i ] - $y{ 'A:1' } ) +
            ( $zb[ $i ] - $z{ 'A:1' } ) * ( $zb[ $i ] - $z{ 'A:1' } ) 
        );
    $bd2 = 
        sqrt( 
            ( $xb[ $i ] - $x{ 'A:2' } ) * ( $xb[ $i ] - $x{ 'A:2' } ) +
            ( $yb[ $i ] - $y{ 'A:2' } ) * ( $yb[ $i ] - $y{ 'A:2' } ) +
            ( $zb[ $i ] - $z{ 'A:2' } ) * ( $zb[ $i ] - $z{ 'A:2' } ) 
        );

    if ( !$i || $mbd1 > $bd1 )
    {
        $mbd1 = $bd1;
    }
    if ( !$i || $mbd2 > $bd2 )
    {
        $mbd2 = $bd2;
    }
}    

print "distance B->A:1 $d1 B->A:2 $d2\n";
print "min distance B->A:1 $mbd1 B->A:2 $mbd2\n";

if ( $d1 < $d2 )
{
    print "A:1 prefered\n";
    $choice_std = "A:1";
} 
if ( $d1 > $d2 )
{
    print "A:2 prefered\n";
    $choice_std = "A:2";
} 
if ( $d1 == $d2 )
{
    print "either preferred!\n";
    die 'x'x80 . "\n";
} 
if ( $mbd1 < $mbd2 )
{
    print "alt A:1 prefered\n";
    $choice_alt = "A:1";
} 
if ( $mbd1 > $mbd2 )
{
    print "alt A:2 prefered\n";
    $choice_alt = "A:2";
} 

if ( $choice_std ne $choice_alt )
{
    print "NOTE: different choice!\n";
}

# `mkdir new 2> /dev/null`;
# $f = "new/$f";

print ">$f\n";
open OUT, ">$f" || die;
print OUT $hdr;
print OUT $out{ $choice_alt };
print OUT $out{ 'B' };
print OUT $foot;
close OUT;

    
