#!/usr/bin/perl

# $debug++;

$dir = shift || die "usage: $0 develop -or- develop_stage\n";
die "usage: $0 develop -or- $0 develop_stage\n" if $dir !~ /^develop(_stage|)$/;

$us3 = $ENV{'us3'} || die "can't find env variable us3\n";

$fconfig = "$us3/somo/us3add.txt";

$f = $fconfig;
open IN, $f || die "$0: $f $!\n";

sub errmsg {
    return "$0: Error: line $l of $fconfig: $_[0]\n";
}

while ( $_ = <IN> )
{
    chomp;
    $l++;

    next if /^(\s*$|\s*#)/;

    $fin = "$us3/$_";
    $f = $fin;
    die errmsg( "$f does not exist" ) if !-e $f;
    open FIN, $f || die errmsg( "$f reading: $!" );
    print "input $f on line $l\n" if $debug;

    $_ = <IN>;
    chomp;
    $l++;

    $fout = "$us3/$_";
    $fout =~ s/___dev___/$dir/;
    $f = $fout;
    die errmsg( "$f exists" ) if -e $f;
    open FOUT, ">$f" || die errmsg( "$f writing: $!" );
    print "output $f on line $l\n" if $debug;

    undef @evals;

    while ( <IN> )
    {
        chomp;
        $l++;
        next if /^(\s*$|\s*#)/;

        if ( /^end\s*$/ )
        {
            print "end of evals for $fin/$fout on line $l\n" if $debug;

            while ( <FIN> )
            {
                for ( $i = 0; $i < @evals; $i++ )
                {
                    eval( $evals[ $i ] );
                }
                print FOUT $_;
            }
            close FIN;
            close FOUT;
            print ">$fout\n";
            last;
        }
        
        push @evals, $_;
    }
}
