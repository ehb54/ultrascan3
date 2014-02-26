#!/usr/bin/perl

# todo:
# black box nodes with no edges


# use strict;
# use warnings;

use File::Spec;

use GraphViz2;

use Log::Handler;

# ---------------

$notes = 
"usage: $0 file format {format2 {format3 ... }}
file contains list, space indent separated to determine linkage
duplicate names ok, they will be multi-mapped
"
;

require "colors.pl";

$f = shift || die $notes;
die $notes if @ARGV < 1;

$fb = $f;
$fb =~ s/\.\w*$//;

open IN, $f || die "$0: $f $!\n";
@l = <IN>;
close IN;
grep chomp, @l;

sub get_depth {
    my $s = $_[0];
    my $d = 0;
    while ( $s =~ /^\s/)
    {
        $d++;
        $s = substr $s, 1;
    }
    $d;
}
        
# build info

for ( $i = 0; $i < @l; ++$i )
{
    $d[ $i ] = get_depth( $l[ $i ] );
    $l[ $i ] =~ s/^\s+//;
    $l[ $i ] =~ s/~/\\n/;
    $color[ $i ] = $colors[ $i % @colors ];
#    print "$d[ $i ] $color[ $i ] <$l[ $i ]>\n";
}


#make tree

my($logger) = Log::Handler -> new;

$logger -> add
    (
     screen =>
     {
         maxlevel       => 'debug',
         message_layout => '%m',
         minlevel       => 'error',
     }
    );

my($graph) = GraphViz2 -> new
    (
     edge   => {color => 'black'},
     global => {directed => 1},
#     graph  => {label => 'US-SOMO Programs', rankdir => 'LR'},
     graph  => { rankdir => 'LR' },
     logger => $logger,
#     concentrate => true,
     node   => {shape => 'box'},
    );

# check for nodes with no outgoing edges

for ( $i = 0; $i < @l; ++$i )
{
    if ( !$nodes{ lc( $l[ $i ] ) } )
    {
        $nodes{ lc( $l[ $i ] ) } = 1;
    }
    if ( !$i )
    {
        push @lp, 0;
    } else {
        if( $d[ $i - 1 ] == $d[ $i ] - 1 )
        {
            push @lp, $i - 1;
        } else {
            if ( $d[ $i ] > $d[ $i - 1 ] )
            {
                die "$0: depth change by more than 2 on line $i\n";
            }
            for ( $k = 0; $k <  $d[ $i - 1 ] - $d[ $i ]; $k++ )
            {
                pop @lp;
            }
        }

        if ( $d[ $i ] && !$og{ $l[ $lp[ -1 ] ] } )
        {
            $og{ $l[ $lp[ -1 ] ] } = 1;
#            print "has outgoing  $lp[ -1 ] <$l[ $lp[ -1 ] ]>\n";
        }
    }
}

for ( $i = 0; $i < @l; ++$i )
{
    if ( !$og{ $l[ $i ] } )
    {
        $color[ $i ] = 'black';
    }
#    print "color now $i $color[ $i ] <$l[ $i ]>\n";
}

$p = 1;
for ( $i = 0; $i < @l; ++$i )
{
    if ( $color[ $i ] ne 'black' )
    {
        $color[ $i ] = $colors[ $p++ % @colors ];
    }
    print "$d[ $i ] $color[ $i ] <$l[ $i ]>\n";
#    print "color now $i $color[ $i ] <$l[ $i ]>\n";
}

undef @lp;
undef %edges;
undef %nodes;
undef %outgoing;

for ( $i = 0; $i < @l; ++$i )
{
    if ( !$nodes{ lc( $l[ $i ] ) } )
    {
        $graph -> add_node( name => $l[ $i ], color => $color[ $i ] );
        $nodes{ lc( $l[ $i ] ) } = 1;
    }
    if ( !$i )
    {
        push @lp, 0;
    } else {
        if( $d[ $i - 1 ] == $d[ $i ] - 1 )
        {
            push @lp, $i - 1;
        } else {
            if ( $d[ $i ] > $d[ $i - 1 ] )
            {
                die "$0: depth change by more than 2 on line $i\n";
            }
            for ( $k = 0; $k <  $d[ $i - 1 ] - $d[ $i ]; $k++ )
            {
                pop @lp;
            }
        }

        if ( $d[ $i ] )
        {
            $fe = $l[ $lp[ -1 ] ];
            $ee = $l[ $i ];
            if ( !$edges{ lc( "${fe}:::${ee}" ) } )
            {
                $graph -> add_edge( from => $fe, to => $ee, arrowsize => 1, color => $color[ $lp[ -1 ] ], weight => 2 );
                print "add_edge( from => \"$fe\", to => \"$ee\", arrowsize => 1, color => $color[ $lp[ -1 ] ], weight => 2 )\n";
                $edges{ lc ( "${fe}:::${ee}" ) } = 1;
            } else {
                print "skipped: add_edge( from => \"$fe\", to => \"$ee\", arrowsize => 1, color => $color[ $lp[ -1 ] ], weight => 2 )\n";
            }
        }
    }
}
        
while( $arg = shift )
{
    my($format)      = $arg;
    my($output_file) = File::Spec -> catfile('html', "${fb}.$format");

    print ">$output_file\n";
    $graph -> run(format => $format, output_file => $output_file);
}

__END__
