#!/usr/bin/perl

$notes =
    "usage: $0 exp.dat files {file2 {...}}
computes HADDOCK score
";

$exp = shift || die $notes;

die "$0: $exp does not exist\n" if !-e $exp;

die $notes if ! scalar @ARGV;

# ----------- config haddock score ----------

$m{ 'vdw'               } = 0.01;
$m{ 'elec'              } = .2;
# $m{ 'buriedsurfacearea' } = -0.01;
$m{ 'desolvationenergy' } = 1.0;
$m{ 'symmetryenergy'    } = 0.1;
$m{ 'air'               } = 0.1;
$m{ 'saxs-chi'          } = 50.0;

$p{ 'haddock'           }++;
$p{ 'haddock+saxs'      }++;
# $p{ 'vdw'               }++;
# $p{ 'elec'              }++;
# $p{ 'buriedsurfacearea' }++;
# $p{ 'desolvationenergy' }++;
# $p{ 'symmetryenergy'    }++;
# $p{ 'air'               }++;
$p{ 'saxs-chi'          }++;

while ( $f = shift )
{
    undef %v;
    die "$0: $f does not exist\n" if !-e $f;
    open IN, $f || die "$0: error opening $f $!\n";

    @l = <IN>;

    close IN;

    $h = $l[ 5 ];
    $e = $l[ 6 ];

    chomp $h;
    chomp $e;
    
    $h =~ s/^REMARK\s*//;
    $e =~ s/^REMARK energies:\s*//;
    $e =~ s/\s+//g;

    @h = split ',', $h;
    @e = split ',', $e;

    print "h = $h; e = $e\n" if $debug;
    for ( $i = 0; $i < @h; $i++ )
    {
        print "$i: $h[ $i ] $e[ $i ]\n" if $debug;
        $v{ $h[ $i ] } = $e[ $i ];
    }

    $h = $l[ 8 ];
    $e = $l[ 9 ];

    chomp $h;
    chomp $e;
    
    $h =~ s/^REMARK\s*//;
    $e =~ s/^REMARK rms-dev\.:\s*//;
    $e =~ s/\s+//g;

    @h = split ',', $h;
    @e = split ',', $e;
    grep s/^/rms-dev:/, @h;

    for ( $i = 0; $i < @h; $i++ )
    {
        $v{ $h[ $i ] } = $e[ $i ];
    }
    
    $h = $l[ 11 ];
    $e = $l[ 13 ];

    chomp $h;
    chomp $e;
    
    $h =~ s/^REMARK\s*//;
    $e =~ s/^REMARK violations\.:\s*//;
    $e =~ s/\s+//g;

    @h = split ',', $h;
    @e = split ',', $e;
    grep s/^/viol:/, @h;

    for ( $i = 0; $i < @h; $i++ )
    {
        $v{ lc( $h[ $i ] ) } = $e[ $i ];
    }
    
    $h = $l[ 15 ];
    $e = $l[ 16 ];

    chomp $h;
    chomp $e;
    
    $h =~ s/^REMARK\s*//;
    $e =~ s/^REMARK AIRs cross-validation:\s*//;
    $e =~ s/\s+//g;

    @h = split ',', $h;
    @e = split ',', $e;

    for ( $i = 0; $i < @h; $i++ )
    {
        $v{ lc( $h[ $i ] ) } = $e[ $i ];
    }
    
    foreach $i ( 18, 20, 22, 23, 24, 25, 27 )
    {
        ( $h, $e ) = $l[ $i ] =~ /^REMARK (.*): (.*)$/;
        chomp $e;
        $h =~ s/ //g;
        
        $v{ lc( $h ) } = $e;
    }

    $h = "haddock";

    $e = 0.0;
    foreach $k ( keys %m )
    {
        $e += $m{ $k } * $v{ $k };
    }

    $v{ lc( $h ) } = $e;

    # run crysol

    $tag = $f;
    $tag =~ s/\.pdb//;
    $tag .= "00";
    $cmd = "rm $tag.fit $tag.log crysol_summary.txt 2> /dev/null; crysol $f $exp /cst y /ns 256 > /dev/null\n";
    print $cmd if $debug;
    print `$cmd`;
    # get chi from crysol
    
    $cmd = "head -1 $tag.fit | awk -F: '{ print \$7 }'\n";
    print $cmd if $debug;
    $v{ "saxs-chi" } = `$cmd`;
    chomp $v{ "saxs-chi" };

    $h = "haddock+saxs";
    $e = 0.0;
    foreach $k ( keys %m )
    {
        $e += $m{ $k } * $v{ $k };
    }

    $v{ lc( $h ) } = $e;
    
    print '-'x10 . "$f" . '-'x( 80 - length( $f ) ) . "\n";
    $out = "REMARK ===============================================================\n";

    foreach $k ( keys %v )
    {
        print "$k $v{$k}\n" if $p{ $k } || $debug;
        $out .= "REMARK $k $v{$k}\n" if $p{ $k };
    }
    $out .= "REMARK ===============================================================\n";

    mkdir "saxs";
    open OUT, ">saxs/$f" || die "$0: error opening >saxs/$f $!\n";
    for ( $i = 0; $i < 40; ++$i )
    {
        print OUT $l[ $i ];
    }
    print OUT $out;
    for ( $i = 40; $i < @l; ++$i )
    {
        print OUT $l[ $i ];
    }

    close $out;
}
