#!/usr/bin/perl

$us = $ENV{'ULTRASCAN'} || die "\$ULTRASCAN must be set\n";
$ush = "$ENV{'HOME'}/ultrascan";

# $debug++;
$do_somo++;
$do_grid++;
# $hydro_opts = "-d";
$maxtimes = 1;

while ($ARGV[0] =~ /^-/) {
    $_ = shift;
    last if /^--/;
    if (/^-d/) {
	$debug++;
        print "debug on\n";
	next;
    }
    if (/^-D/) {
	$hydro_opts = "-d";
        print "hydrodyn debug on\n";
	next;
    }
    if (/^-s/) {
	undef $do_somo;
        print "somo testing disabled\n";
	next;
    }
    if (/^-g/) {
	undef $do_grid;
        print "grid testing disabled\n";
	next;
    }
    if (/^-r/) {
	$maxtimes = shift;
        print "maximum repetitions $maxtimes\n";
	next;
    }
    if (/^-m/) {
	$mwtest++;
        print "molecular weight testing mode\n";
	next;
    }
    if (/^-(h|\?)/) {
        print "usage:
$0 (-d|-D|-m|-r count|-s|-g)
options:
    -d debug on
    -D hydrodyn debug on
    -m special mw testing
    -r set maximum repetitions (default 1)
    -s turn off somo checks
    -g turn off grid checks
";
        exit;
    }
    die "I don't recognize this switch: $_\n";
}

@test = ( # file, testsomo, testgrid, config file, expect failure
         '0GGG.pdb', 1, 1, 'dflt', 0,
         '0GGG.pdb', 1, 1, 'rrr', 0,
         '1HEL.pdb', 0, 1, 'rrr', 0,
         '0AP.pdb', 1, 1, 'dflt', 0,
         '0APA.pdb', 1, 1, 'dflt', 0,
         '0APAoxt.pdb', 1, 1, 'dflt', 0,
         '0APP.pdb', 1, 1, 'dflt', 0,
         '0APPoxt.pdb', 1, 1, 'dflt', 0,
         '0APoxt.pdb', 1, 1, 'dflt', 0,
         '0G.pdb', 1, 1, 'dflt', 0,
         '0GAUGG.pdb', 1, 1, 'dflt', 1,
         '0GAUGG.pdb', 1, 1, 'res1', 1,
         '0GAUGG.pdb', 1, 1, 'res2', 1,
         '0GAUGG.pdb', 1, 1, 'atom1', 0,
         '0GAUGG.pdb', 1, 1, 'atom2', 0,
         '0GG.pdb', 1, 1, 'dflt', 0,
         '0GGG.pdb', 1, 1, 'dflt', 0,
         '0GGGG.pdb', 1, 1, 'dflt', 0,
         '0GGGGoxt.pdb', 1, 1, 'dflt', 0,
         '0GGGoxt.pdb', 1, 1, 'dflt', 0,
         '0GGoxt.pdb', 1, 1, 'dflt', 0,
         '0GP.pdb', 1, 1, 'dflt', 0,
         '0GPG.pdb', 1, 1, 'dflt', 0,
         '0GPGoxt.pdb', 1, 1, 'dflt', 0,
         '0GPP.pdb', 1, 1, 'dflt', 0,
         '0GPPoxt.pdb', 1, 1, 'dflt', 0,
         '0GPoxt.pdb', 1, 1, 'dflt', 0,
         '0GU2GG.pdb', 1, 1, 'dflt', 1,
         '0GU3GG.pdb', 1, 1, 'dflt', 1,
         '0GU4GG.pdb', 1, 1, 'dflt', 1,
         '0GUGG.pdb', 1, 1, 'dflt', 1,
         '0GUGGoxt.pdb', 1, 1, 'dflt', 1,
         '0Goxt.pdb', 1, 1, 'dflt', 0,
         '0NRN.pdb', 1, 1, 'dflt', 0,
         '0NRxCBN.pdb', 1, 1, 'dflt', 1,
         '0P.pdb', 1, 1, 'dflt', 0,
         '0PA.pdb', 1, 1, 'dflt', 0,
         '0PAP.pdb', 1, 1, 'dflt', 0,
         '0PAPoxt.pdb', 1, 1, 'dflt', 0,
         '0PAoxt.pdb', 1, 1, 'dflt', 0,
         '0PG.pdb', 1, 1, 'dflt', 0,
         '0PGP.pdb', 1, 1, 'dflt', 0,
         '0PGPoxt.pdb', 1, 1, 'dflt', 0,
         '0PGoxt.pdb', 1, 1, 'dflt', 0,
         '0PP.pdb', 1, 1, 'dflt', 0,
         '0PPA.pdb', 1, 1, 'dflt', 0,
         '0PPAoxt.pdb', 1, 1, 'dflt', 0,
         '0PPG.pdb', 1, 1, 'dflt', 0,
         '0PPGoxt.pdb', 1, 1, 'dflt', 0,
         '0PPP.pdb', 1, 1, 'dflt', 0,
         '0PPPoxt.pdb', 1, 1, 'dflt', 0,
         '0PPoxt.pdb', 1, 1, 'dflt', 0,
         '0Poxt.pdb', 1, 1, 'dflt', 0,
         '0AP.pdb', 1, 1, 'rrr', 0,
         '0APA.pdb', 1, 1, 'rrr', 0,
         '0APAoxt.pdb', 1, 1, 'rrr', 0,
         '0APP.pdb', 1, 1, 'rrr', 0,
         '0APPoxt.pdb', 1, 1, 'rrr', 0,
         '0APoxt.pdb', 1, 1, 'rrr', 0,
         '0G.pdb', 1, 1, 'rrr', 0,
         '0GAUGG.pdb', 1, 1, 'rrr', 1,
         '0GAUGG.pdb', 1, 1, 'res1', 1,
         '0GAUGG.pdb', 1, 1, 'res2', 1,
         '0GAUGG.pdb', 1, 1, 'atom1', 0,
         '0GAUGG.pdb', 1, 1, 'atom2', 0,
         '0GG.pdb', 1, 1, 'rrr', 0,
         '0GGG.pdb', 1, 1, 'rrr', 0,
         '0GGGG.pdb', 1, 1, 'rrr', 0,
         '0GGGGoxt.pdb', 1, 1, 'rrr', 0,
         '0GGGoxt.pdb', 1, 1, 'rrr', 0,
         '0GGoxt.pdb', 1, 1, 'rrr', 0,
         '0GP.pdb', 1, 1, 'rrr', 0,
         '0GPG.pdb', 1, 1, 'rrr', 0,
         '0GPGoxt.pdb', 1, 1, 'rrr', 0,
         '0GPP.pdb', 1, 1, 'rrr', 0,
         '0GPPoxt.pdb', 1, 1, 'rrr', 0,
         '0GPoxt.pdb', 1, 1, 'rrr', 0,
         '0GU2GG.pdb', 1, 1, 'rrr', 1,
         '0GU3GG.pdb', 1, 1, 'rrr', 1,
         '0GU4GG.pdb', 1, 1, 'rrr', 1,
         '0GUGG.pdb', 1, 1, 'rrr', 1,
         '0GUGGoxt.pdb', 1, 1, 'rrr', 1,
         '0Goxt.pdb', 1, 1, 'rrr', 0,
         '0NRN.pdb', 1, 1, 'rrr', 0,
         '0NRxCBN.pdb', 1, 1, 'rrr', 1,
         '0P.pdb', 1, 1, 'rrr', 0,
         '0PA.pdb', 1, 1, 'rrr', 0,
         '0PAP.pdb', 1, 1, 'rrr', 0,
         '0PAPoxt.pdb', 1, 1, 'rrr', 0,
         '0PAoxt.pdb', 1, 1, 'rrr', 0,
         '0PG.pdb', 1, 1, 'rrr', 0,
         '0PGP.pdb', 1, 1, 'rrr', 0,
         '0PGPoxt.pdb', 1, 1, 'rrr', 0,
         '0PGoxt.pdb', 1, 1, 'rrr', 0,
         '0PP.pdb', 1, 1, 'rrr', 0,
         '0PPA.pdb', 1, 1, 'rrr', 0,
         '0PPAoxt.pdb', 1, 1, 'rrr', 0,
         '0PPG.pdb', 1, 1, 'rrr', 0,
         '0PPGoxt.pdb', 1, 1, 'rrr', 0,
         '0PPP.pdb', 1, 1, 'rrr', 0,
         '0PPPoxt.pdb', 1, 1, 'rrr', 0,
         '0PPoxt.pdb', 1, 1, 'rrr', 0,
         '0Poxt.pdb', 1, 1, 'rrr', 0,
         '1HEL.pdb', 0, 1, 'dflt', 0,
         '1HEL.pdb', 1, 1, 'res1', 0,
         '1HEL.pdb', 1, 1, 'res2', 0,
         '1HEL.pdb', 1, 1, 'atom1', 0,
         '1HEL.pdb', 1, 1, 'atom2', 0,
         '1HEL.pdb', 1, 1, 'res1atom1', 0,
         '1HEL.pdb', 1, 1, 'res1atom2', 0,
         '1HEL.pdb', 1, 1, 'res2atom1', 0,
         '1HEL.pdb', 1, 1, 'res2atom2', 0,
         '1AKI.pdb', 1, 1, 'dflt', 0,
         );

for ($i = 0; $i < @test; ) {
    push @pdbs, $test[$i++];
    push @testsomo, $test[$i++];
    push @testgrid, $test[$i++];
    push @config, $test[$i++];
    push @expectfailure, $test[$i++];
}

sub testsomo { # file, config, expectfailure
    my $tmpf = "$us/somo/test/test.cmds";
    open(OUT, ">$tmpf") || die "can not open $tmpf for writing $!\n";
    print OUT "reset\n";

    my $i = 0;
    my $config = "$_[1]";
    my $expectfailure = $_[2];
    my $fconfig = "$us/somo/test/configs/${config}.config";
    die "configuration file <$fconfig> missing!\n" if !-e $fconfig;
    my $logf = "$us/somo/test/log/somo-$config-$_[$i]";
    print "logf is <$logf>\n" if $debug;

    print "somo testing $_[$i] config $config:\n";
    my $f = "$us/somo/test/structures/$_[$i]";
    die "$f does not exist!\n" if !-e $f;
    print OUT "load $fconfig\nload $f\n";
    for(my $i = 0; $i < $times; $i++) {
        print OUT "somo\nhydro\n";
    }

    print OUT "exit\n";
    close OUT;

    my @refs = ( "_1-so.hydro_res",
                 "_1-so.asa_res",
                 "_1-so.bead_model" );

    # backup existing output files

    {
        my $fbase = $_[$i];
        $fbase =~ s/\.pdb$//;
        for ( my $j = 0; $j < @refs; $j++) 
        {
            my $forg = "$ush/somo/$fbase$refs[$j]";
            my $fbak = "$us/somo/test/bak/$fbase$refs[$j]";
            if ( -e $fbak )
            {
                $cmd = "rm $fbak\n";
                print $cmd if $debug;
                print `$cmd`;
            }
            if ( -e $forg )
            {
                $cmd = "mv $forg $fbak\n";
                print $cmd if $debug;
                print `$cmd`;
            }
        }
    }
            
    my $cmd = "us_hydrodyn $hydro_opts -c $tmpf > $logf 2>&1\n";
    print $cmd if $debug;
    print `$cmd`;
    
    {
        my $fbase = $_[$i];
        $fbase =~ s/\.pdb$//;
        for ( my $j = 0; $j < @refs; $j++) 
        {
            $total++;
            my $fnew = "$ush/somo/$fbase$refs[$j]";
            my $fref = "$us/somo/test/ref/somo/$config-$fbase$refs[$j]";
            `mkdir -p $us/somo/test/ref/somo > /dev/null 2>&1`;
            if (!-e $fnew) 
            {
                if ( $expectfailure )
                {
                    $success++;
                } else {
                    $errors++;
                    $erlist{$_[$i]}++;
                    print "expected file $fnew is missing, error\n";
                }
            } else {
                if ( $expectfailure )
                {
                    $errors++;
                    $erlist{$_[$i]}++;
                    print "expected file $fnew is present but we expected failure!\n";
                } else {
                    if (!-e $fref) 
                    {
                        $copys++;
                        print "$fref is missing, can not compare so copying as reference\n";
                        $cmd = "cp $fnew $fref\n";
                        print $cmd if $debug;
                        print `$cmd`;
                    } else {
                        if ( $refs[$j] =~  /hydro_res$/ )
                        {
                            my $t1 = `tempfile`;
                            my $t2 = `tempfile`;
                            chomp $t1;
                            chomp $t2;
                            $cmd = "sed '7d' $fnew > $t1\nsed '7d' $fref > $t2\ndiff $t1 $t2 | head -10\nrm $t1 $t2\n";
                        } else {
                            $cmd = "diff $fnew $fref | head -10\n";
                        }
                        print $cmd if $debug;
                        @results = `$cmd`;
                        print @results;
                        if ( @results > 0 ) {
                            $errors++;
                            $erlist{$_[$i]}++;
                        } else {
                            $success++;
                        }
                    }
                }
            }
        }
    }

    # restore pre-existing output files

    {
        my $fbase = $_[$i];
        $fbase =~ s/\.pdb$//;
        for ( my $j = 0; $j < @refs; $j++) 
        {
            my $forg = "$ush/somo/$fbase$refs[$j]";
            my $fbak = "$us/somo/test/bak/$fbase$refs[$j]";
            if ( -e $forg ) 
            {
                $cmd = "rm $forg\n";
                print $cmd if $debug;
                print `$cmd`;
            }
            if ( -e $fbak ) 
            {
                $cmd = "mv $fbak $forg\n";
                print $cmd if $debug;
                print `$cmd`;
            }
        }
    }
}

sub testgrid { # file, config, expectfailure
    my $tmpf = "$us/somo/test/test.cmds";
    open(OUT, ">$tmpf") || die "can not open $tmpf for writing $!\n";
    print OUT "reset\n";

    my $i = 0;
    my $config = "$_[1]";
    my $expectfailure = $_[2];
    my $fconfig = "$us/somo/test/configs/${config}.config";
    die "configuration file <$fconfig> missing!\n" if !-e $fconfig;

    my $logf = "$us/somo/test/log/grid-$config-$_[$i]";
    print "logf is <$logf>\n" if $debug;

    print "grid testing $_[$i] config $config:\n";
    my $f = "$us/somo/test/structures/$_[$i]";
    die "$f does not exist!\n" if !-e $f;
    print OUT "load $fconfig\nload $f\n";
    for(my $i = 0; $i < $times; $i++) {
        print OUT "grid\nhydro\n";
    }

    print OUT "exit\n";
    close OUT;

    my @refs = ( "_1-a2b.hydro_res",
                 "_1-a2b.bead_model" );

    # backup existing output files

    {
        my $fbase = $_[$i];
        $fbase =~ s/\.pdb$//;
        for ( my $j = 0; $j < @refs; $j++) 
        {
            my $forg = "$ush/somo/$fbase$refs[$j]";
            my $fbak = "$us/somo/test/bak/$fbase$refs[$j]";
            if ( -e $fbak )
            {
                $cmd = "rm $fbak\n";
                print $cmd if $debug;
                print `$cmd`;
            }
            if ( -e $forg )
            {
                $cmd = "mv $forg $fbak\n";
                print $cmd if $debug;
                print `$cmd`;
            }
        }
    }
            
    my $cmd = "us_hydrodyn $hydro_opts -c $tmpf > $logf 2>&1\n";
    print $cmd if $debug;
    print `$cmd`;
    
    {
        my $fbase = $_[$i];
        $fbase =~ s/\.pdb$//;
        for ( my $j = 0; $j < @refs; $j++) 
        {
            $total++;
            my $fnew = "$ush/somo/$fbase$refs[$j]";
            my $fref = "$us/somo/test/ref/grid/$config-$fbase$refs[$j]";
            `mkdir -p $us/somo/test/ref/grid > /dev/null 2>&1`;
            if (!-e $fnew) 
            {
                if ( $expectfailure )
                {
                    $success++;
                } else {
                    $errors++;
                    $erlist{$_[$i]}++;
                    print "expected file $fnew is missing, error\n";
                }
            } else {
                if ( $expectfailure )
                {
                    $errors++;
                    $erlist{$_[$i]}++;
                    print "expected file $fnew is present but we expected failure!\n";
                } else {
                    if (!-e $fref) 
                    {
                        $copys++;
                        print "$fref is missing, can not compare so copying as reference\n";
                        $cmd = "cp $fnew $fref\n";
                        print $cmd if $debug;
                        print `$cmd`;
                    } else {
                        if ( $refs[$j] =~  /hydro_res$/ )
                        {
                            my $t1 = `tempfile`;
                            my $t2 = `tempfile`;
                            chomp $t1;
                            chomp $t2;
                            $cmd = "sed '7d' $fnew > $t1\nsed '7d' $fref > $t2\ndiff $t1 $t2\nrm $t1 $t2\n";
                        } else {
                            $cmd = "diff $fnew $fref\n";
                        }
                        print $cmd if $debug;
                        @results = `$cmd`;
                        print @results;
                        if ( @results > 0 ) {
                            $errors++;
                            $erlist{$_[$i]}++;
                        } else {
                            $success++;
                        }
                    }
                }
            }
        }
    }

    # restore pre-existing output files

    {
        my $fbase = $_[$i];
        $fbase =~ s/\.pdb$//;
        for ( my $j = 0; $j < @refs; $j++) 
        {
            my $forg = "$ush/somo/$fbase$refs[$j]";
            my $fbak = "$us/somo/test/bak/$fbase$refs[$j]";
            if ( -e $forg ) 
            {
                $cmd = "rm $forg\n";
                print $cmd if $debug;
                print `$cmd`;
            }
            if ( -e $fbak ) 
            {
                $cmd = "mv $fbak $forg\n";
                print $cmd if $debug;
                print `$cmd`;
            }
        }
    }
}

if ( $mwtest ) {
    foreach $file ( "1AKI.pdb", "1HEL.pdb", "dimer.pdb" )
#    foreach $file ( "0AP.pdb", "0GGGGoxt.pdb" ) 
    {
        print "trying $file\n" if $debug;
        for ($times = 1; $times <= $maxtimes; $times++) {
            for ( $a1 = 5; $a1 < 95; $a1 += 5 ) {
                for ( $a2 = 5; $a2 < 95; $a2 += 5 ) {
                    $cfg = "sa${a1}r${a2}";
                    &testsomo($file, $cfg, 0 ) if $do_somo;
                }
            }
            for ( $g = 1; $g < 10; $g++ ) {
                $cfg = "g${g}";
                &testgrid($file, $cfg, 0 ) if $do_grid;
            }
        }
    }
    exit;
}

$success = $errors = $copys = 0;

for($i = 0; $i < @pdbs; $i++) {
    print "trying $pdbs[$i]\n" if $debug;
    for ($times = 1; $times <= $maxtimes; $times++) {
        if ( $testsomo[$i] && $do_somo) {
            &testsomo($pdbs[$i], $config[$i], $expectfailure[$i] );
        }
        if ( $testgrid[$i] && $do_grid) {
            &testgrid($pdbs[$i], $config[$i], $expectfailure[$i] );
        }
    }
}

print "summary:
successful: $success of $total
errors:     $errors of $total
copies:     $copys
";

print "Files with errors:\n" if $errors;
foreach $i (keys %erlist)
{
    print "$i\n";
}
