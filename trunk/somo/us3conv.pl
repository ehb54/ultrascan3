#!/usr/bin/perl

# $debug++;

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
            die errmsg( "AUTFBACK must be YES or no" ) if $_[ 4 ] !~ /^(YES|no)$/;
            
            $te++;

            if ( length( $_[ 2 ] ) )
            {
                if ( $pcg{ $_[ 2 ] } && $pcg{ $_[ 2 ] } != $_[ 3 ] )
                {
                    die errmsg( "duplicate cg_ types point to different PALET_ types" );
                }
                $pcg{ $_[ 2 ] } = $_[ 3 ];

                if ( $acg{ $_[ 2 ] } && $acg{ $_[ 2 ] } != $_[ 4 ] )
                {
                    die errmsg( "duplicate cg_ types point to different AUTFBACK disposition" );
                }
                $acg{ $_[ 2 ] } = $_[ 4 ];
                next;
            }

            if ( $pt{ $_[ 0 ] } && $pt{ $_[ 0 ] } != $_[ 3 ] )
            {
                die errmsg( "duplicate types with n/a cg_ point to different PALET_ types" );
            }
            $pt{ $_[ 0 ] } = $_[ 3 ];

            if ( $at{ $_[ 0 ] } && $at{ $_[ 0 ] } != $_[ 4 ] )
            {
                die errmsg( "duplicate types with n/a cg_ point to different AUTFBACK disposition" );
            }
            $at{ $_[ 0 ] } = $_[ 4 ];
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

print "pcg:\n";
foreach $k ( keys %pcg )
{
    print "$k $pcg{ $k }\n";
}

print "pt:\n";
foreach $k ( keys %pt )
{
    print "$k $pt{ $k }\n";
}

# go through the files

for ( $i = 0; $i < @files; $i++ )
{
    $fin = $files[ $i ];
    $f = $fin;
    open IN, $f || die "$0: $f $!\n";
    print "processing $f\n";

    $fout = "$files[ $i ]-u3conv";
    $f = $fout;
    open OUT, ">$f" || die "$0: $f $!\n";

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
            undef $ptype;
            undef $autfb;

            if ( $pcg{ $cgtype } )
            {
                $ptype = $pcg{ $cgtype };
                $auftb = ( $acg{ $cgtype } =~ /^YES$/ ) ? 1 : 0;
            } else {
                if ( $pt{ $lasttype } )
                {
                    $ptype = $pt{ $lasttype };
                    $auftb = ( $at{ $lasttype } =~ /^YES$/ ) ? 1 : 0;
                } else {
                    $warns{ "type: $lasttype cg_: $cgtype" }++;
                    warn "warning: $fin line $l: no table entry for type: $lasttype cg_: $cgtype\n";
                }
            }
                    
            if ( $debug )
            {
                print "line: $l\n";
                print sprintf( "setpalette: %s\n", substr( $_, 0, 80 ) );
                print "    cgtype: $cgtype\n";
                print "   lastnew: $lastnew\n";
                print "  lasttype: $lasttype\n";
                print "     ptype: $ptype\n";
                print "     autfb: $autfb\n";
            }
            $used_types{ $lasttype }++;
        } else {
            print OUT $_;
        }
    }

    close IN;
    close OUT;
    print "created $fout";
}

print "used object types:\n";
foreach $k ( keys %used_types )
{
    print "$k\n";
}

print "missing object types:\n";
foreach $k ( keys %warns )
{
    print "$k\n";
}

# TODO:
# process attribs, write out
