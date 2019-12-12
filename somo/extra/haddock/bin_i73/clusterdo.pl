#!/usr/bin/perl

$notes = "usage: $0 threshold clusterfiles
groups all structures with pairwise distance of threshold
haddock-scores.txt must be in current directory
";

$thresh = shift || die $notes;

$hsf = "haddock-scores.txt";
die "no haddock-scores.txt (run collectscores.pl) in current directory\n" if !-e $hsf;
{
    open IN, $hsf || die "$0 open $hsf $!\n";
    my @l = <IN>;
    close IN;
    grep chomp, @l;

    for ( my $i = 0; $i < @l; ++$i )
    {
        my @j = split /\s+/, $l[ $i ];
        $j[ 0 ] =~ s/\.pdb$//;

        $hs{ "$j[ 0 ]-B" } = $j[ 1 ];
        $hs{ "$j[ 0 ]-C" } = $j[ 1 ];
    }
}

while ( $f = shift )
{
    print '-'x20 . " $f " . '-'x20 . "\n";
    my %cluster;
    my %incluster;
    my %emptycluster;
    $nextc = 1;
#    my %dist;
    open IN, $f || die;
    @l = <IN>;
    close IN;
    my $min = 1e99;
    my $max = -1e99;
    for ( $i = 0; $i < @l; ++$i )
    {
        @j = split /\s+/, $l[$i];
        $f1 = $j[ 0 ];
        $f2 = $j[ 1 ];
        $d = $j[ 2 ];

#        $dist{ "$f1:$f2" } = $d;

        if ( $d <= $thresh )
        {
            if ( $incluster{ $f1 } )
            {
                if ( $incluster{ $f2 } )
                {
                    if ( $incluster{ $f1 } == $incluster{ $f2 } )
                    {
                        next;
                    }
                    # merge clusters
                    my $from = $incluster{ $f2 };
                    my $to   = $incluster{ $f1 };
                    
                    foreach my $k ( keys %incluster )
                    {
                        if ( $incluster{ $k } == $from )
                        {
                            $incluster{ $k } = $to;
                        }
                    }
                    $emptycluster{ $from }++;
                    
#                    die "both $f1 $f2 dist $d already in cluster $incluster{ $f1 } $incluster{ $f2 } ?\n";
                }
                $incluster{ $f2 } = $incluster{ $f1 };
                next;
            } else {
                if ( $incluster{ $f2 } )
                {
                    $incluster{ $f1 } = $incluster{ $f2 };
                    next;
                }
                # new cluster
                $incluster{ $f1 } = $nextc;
                $incluster{ $f2 } = $incluster{ $f1 };
                $nextc++;
                next;
            }
        }
    }

    $uc = 0;
    for ( my $c = 1; $c < $nextc; ++$c )
    {
        if ( $emptycluster{ $c } )
        {
            next;
        }
        $uc++;
        print "cluster $uc";

# analyze cluster
        {
            my @mem;
            my $hs_avg;
            my $hs_sum;
            my $hs_sum2;
            my $hs_sd;
            my $hs_min = 1e99;
            my $hs_max = -1e99;
            my $hs_count;

            foreach $k ( keys %incluster )
            {
                if ( $incluster{ $k } == $c )
                {
                    die "$0: haddock score not found for '$k'\n" if !$hs{ $k };
                    my $score = $hs{ $k };

                    $hs_min = $score if $hs_min > $score;
                    $hs_max = $score if $hs_max < $score;

                    push @mem, "$score $k";
                    $hs_sum += $score;
                    $hs_sum2 += $score * $score;
                    $hs_count++;
                }
            }
            
            $hs_avg = $hs_sum / $hs_count;
            $hs_sd  = sqrt( ( $hs_sum2 / $hs_count ) - $hs_avg * $hs_avg );
            my @mems = sort { $a <=> $b } @mem;
            
            print " $hs_count pdbs haddock score average $hs_avg sd $hs_sd min $hs_min max $hs_max\n";

            grep s/^/  /, @mems;
            grep s/$/.pdb/, @mems;
            print join "\n", @mems;
            print "\n";
        }
    }
}


