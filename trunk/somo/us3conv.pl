#!/usr/bin/perl

$debug++;

$us3 = $ENV{'us3'} || die "can't find env variable us3\n";

$fconfig = "$us3/somo/us3conv.txt";

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

    undef $ok;

    if ( /^\s*table\s*$/ )
    {
        $ok++;

        # read table
        while ( $_ = <IN> )
        {
            chomp;
            $l++;

            next if /^(\s*$|\s*#)/;

            last if /^\s*end\s*$/;

            split /\s+/;
            die errmsg( sprintf( "too many tokens for table entry %d", @_ ) ) if @_ != 5;
            
            $cgt = "~$_[ 2 ]";
            $cgt = '' if $_[ 2 ] eq 'n/a';
            $key = "$_[ 0 ]$cgt";

            die errmsg( "duplicate table key $key" ) if $tg{ $key };
            $tg{ $key } = $_[ 1 ];
            $tp{ $key } = $_[ 3 ];
            $ta{ $key } = $_[ 4 ];
            $te++;
        }

        next;
    }

    if ( /^\s*files\s*$/ )
    {
        $ok++;

        # read files
        while ( $_ = <IN> )
        {
            chomp;
            $l++;

            next if /^(\s*$|\s*#)/;

            last if /^\s*end\s*$/;

            push @files, "$us3/somo/develop/$_";
        }

        next;
    }

    die errmsg( "unrecognized token(s)" ) if !$ok;
}

close IN;

# check files

for ( $i = 0; $i < @files; $i++ )
{
    $error .= "$files[ $i ] not found\n" if !-e $files[ $i ];
}

die $error if $error;

print sprintf( "%d table entries found\n", $te );
print sprintf( "%d files found\n"        , scalar @files );

# go through the files

for ( $i = 0; $i < @files; $i++ )
{
    $f = $files[ $i ];
    open IN, $f || die "$0: $f $!\n";
    print "processing $f\n";

    undef $l;
    while ( <IN> )
    {
        $l++;
        if ( /(\S+)\s*=\s*new\s+(\S+)/ )
        {
            $lastnew =  $1;
            $lasttype = $2;
            $lasttype =~ s/(;|\().*$//;
        }

        if ( /->\s*setPalette/ && !/PALET_/ )
        {
            ( $cgtype ) = $_ =~ /cg_(\w+)/;
            chomp;
            print "line: $l\n";
            print sprintf( "setpalette: %s\n", substr( $_, 0, 80 ) );
            print "    cgtype: $cgtype\n";
            print "   lastnew: $lastnew\n";
            print "  lasttype: $lasttype\n";
        }
    }
    exit;
}

# TODO:
# process attribs, write out
