#!/usr/bin/perl

$notes = 
"usage: $0 airlist
write air .tbl to stdout
format of airlist

selection 1
segid A
active #,#,#
passive #,#,#
partners #,#,#

selection 2
etc.

e.g.
selection 1
segid A
active 1,2,3
passive 4,5,6,7
partners 2

selection 2
segid B
active 8,9,10
passive 11,12,13,14
partners 1

notes: they must be in selection # order
"
    ;

$f = shift || die $notes;
open IN, $f || die "$0: $f $!\n";
@l = <IN>;
close IN;
grep chomp, @l;

$last_selection = 0;
for ( $i = 0; $i < @l; ++$i )
{
    $l[ $i ] =~ s/#.*$//;

    next if $l[ $i ] =~ /^\s*$/;

    $l[ $i ] =~ s/^\s*//;
    $l[ $i ] =~ s/\s*$//;

    ( $tok, $args ) = $l[ $i ] =~ /^(\w*)\s+(\S+)$/;

    $line = $i + 1;

    if ( $tok eq 'selection' )
    {
        # accum for selection
        die "error: line $line: selection must be strictly unity incremental\n" if $args != $last_selection + 1;
        $last_selection = $args;
        next;
    }

    die "error: line $line: no selection defined\n" if !$last_selection;

    $tm_key = "$tok:$last_selection";

    die "error: line $line: duplicate '$tok' token for selection $last_selection\n" if $used{ $tm_key };
    $used{ $tm_key } = true;

    die "error: line $line: empty args\n" if !length( $args );

    $args =~ s/\s+//g;

    @args = split ',', $args;

    if ( $tok eq 'active' )
    {
        $active{ $last_selection } = [ map { $_ } @args ];
        next;
    }

    if ( $tok eq 'passive' )
    {
        $passive{ $last_selection } = [ map { $_ } @args ];
        next;
    }

    if ( $tok eq 'segid' )
    {
        $segid{ $last_selection } = [ map { $_ } @args ];
        next;
    }

    if ( $tok eq 'partners' )
    {
        $partners{ $last_selection } = [ map { $_ } @args ];
        next;
    }

    die "$0: unrecognized token $tok on line " . ( $i + 1 ) . "\n";
}

# - check valid partners

for ( $m = 1; $m <= $last_selection; ++$m )
{
    for ( $i = 0; $i <= @{$partners{ $m }}; ++$i )
    {
        die "error: checking partners for selection $m: invalid partner ${$partners{ $m }}[ $i ]\n" 
            if ${$partners{ $m }}[ $i ] > $last_selection;
        die "error: checking partners for selection $m: invalid self-partner ${$partners{ $m }}[ $i ]\n" 
            if ${$partners{ $m }}[ $i ] == $m;
    }
}

@m_to_txt = ( "0th", "1st", '2nd', '3rd' );

# - produce haddock style output
print "! HADDOCK AIR restraints\n";
for ( $m = 1; $m <= $last_selection; ++$m )
{
    $mtxt = $m_to_txt[ $m ];
    $mtxt = "${m}th" if !length( $mtxt );
    print "! HADDOCK AIR restraints for $mtxt selection\n!\n";

# foreach active in the selection
    $seg = ${$segid{ $m }}[ 0 ];
    for ( $i = 0; $i < @{$active{ $m }}; ++$i )
    {
        $res = ${$active{ $m }}[ $i ];
#        print "\n" if $i;
        print
"assign ( resid $res  and segid $seg)
       (\n"
         ;
# for each partner (one for now)
        for ( $p = 0; $p < @{$partners{ $m }}; ++$p )
        {
            $pm = ${$partners{ $m }}[ $p ];
            $p_seg = ${$segid{ $pm }}[ 0 ];
# partner actives
            for ( $j = 0; $j < @{$active{ $pm }}; ++$j )
            {
                $p_res = ${$active{ $pm }}[ $j ];
                print "     or\n" if $j || $p;
                print
"        ( resid $p_res  and segid $p_seg)\n"
                ;
            }
# partner passives
            for ( $j = 0; $j < @{$passive{ $pm }}; ++$j )
            {
                $p_res = ${$passive{ $pm }}[ $j ];

                print "     or\n";
                print
"        ( resid $p_res  and segid $p_seg)\n"
                ;
            }
        }            
        print "       )  2.0 2.0 0.0\n\n";
    }
    print "\n";
}
print "\n";
