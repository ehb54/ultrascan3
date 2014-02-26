#!/usr/bin/perl

$notes = "usage: $0 start-threshold end-threshold delta-threshold clusterfiles
groups all structures with pairwise distance of threshold
haddock-scores.txt must be in current directory
puts results in cluster_results_threshhold
and cluster_results_summary
";

$thresh_start = shift || die $notes;
$thresh_end = shift || die $notes;
$thresh_delta = shift || die $notes;

die "threshhold settings error\n" if $thresh_end < $thresh_start || $thresh_delta <= 0;

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

for ( $thresh = $thresh_start; $thresh <= $thresh_end; $thresh += $thresh_delta )
{
    @basef = @ARGV;

    while ( $f = shift @basef )
    {
        my $out;

        my $fu = $f;
        $fu =~ s/\/cluster_res\.txt//;
        $out .= '-'x20 . " RMSD clustering threshold $thresh $fu " . '-'x20 . "\n";
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
        
        my $s_total_pdbs;
        my $s_max_cluster_size = -1e99;

        my $uc = 0;
        my $wiki_adds;

        for ( my $c = 1; $c < $nextc; ++$c )
        {
            if ( $emptycluster{ $c } )
            {
                next;
            }
            $uc++;
            $out .= "cluster $uc";
            
# analyze cluster
            {
                my @mem;
                my @memw;
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
                        my $altk = $k;
                        $altk =~ s/-(B|C)$//;
                        push @memw, "$score [htdocs:pdb/$k.pdb $k.pdb] [htdocs:pdb/$altk.pdb $altk.pdb][[br]]";
                        $hs_sum += $score;
                        $hs_sum2 += $score * $score;
                        $hs_count++;
                        $s_total_pdbs++;
                    }
                }
                
                $hs_avg = $hs_sum / $hs_count;
                $hs_sd  = sqrt( ( $hs_sum2 / $hs_count ) - $hs_avg * $hs_avg );
                my @mems = sort { $a <=> $b } @mem;
                my @memsw = sort { $a <=> $b } @memw;

                $wiki_adds .= " $uc || $hs_min ||";

                splice @memsw, 3;
                $wiki_adds .= join ' ', @memsw;
                $wiki_adds .= " ||";
                
                $out .= " $hs_count pdbs haddock score average $hs_avg sd $hs_sd min $hs_min max $hs_max\n";
                
                grep s/^/  /, @mems;
                grep s/$/.pdb/, @mems;
                $out .= join "\n", @mems;
                $out .= "\n";

                $s_max_cluster_size = $hs_count if $s_maxs_cluster_size < $hs_count;

            }
        }
        $s_avg_pdbs = $s_total_pdbs;
        if ( $uc )
        {
            $s_avg_pdbs /= $uc;
            $s_avg_pdbs = sprintf( "%.1f", $s_avg_pdbs );
        }
        my $fux = $fu;
        if ( $fux =~ /water/ )
        {
            $fux =~ s/\/1\/water/ stage 3/;
        } else {
            $fux =~ s/\/1/ stage 2/;
        }
            
        my $fuf = $fu;
        $fuf =~ s/\//_/g;
        $f = "cluster_results_${thresh}_${fuf}.txt";
        print ">$f\n";
        open OUT, ">$f" || die;
        print OUT $out;
        close OUT;

        $s_cc{ "$fu" } .= "|| $fux || $thresh || $uc || $s_total_pdbs || $s_avg_pdbs || $s_max_cluster_size || [htdocs:data/$f $f ] || $wiki_adds\n";

    }
}

$f = "cluster_results_wiki";
print ">$f\n";
open OUT, ">$f" || die;

print OUT "|| run || RMSD threshold || number of clusters || total pdbs clustered || avg cluster size || max cluster size || detailed results in || cluster number || haddock score || best 3 haddock pdbs || ... ||\n";
foreach my $k ( keys %s_cc )
{
    push @mo, $s_cc{ $k };
}

print OUT join '', sort @mo;
print OUT "\n";
close OUT;

