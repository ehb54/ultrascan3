#!/usr/bin/perl

# just collect everything up in the outermost qstrings until closing
# $debug++;

sub is_ok {
    my $l = $_[0];
    my @n;
    my @l = split //, $l;
    for ( my $i = 0; $i < @l; $i++ ) {
        if ( $l[$i] =~ /%/ ) {
            push @n, sprintf("%d", substr($l,$i+1));
        }
    }

    my $ok = 1;
    for ( my $i = 0; $i < @n; $i++ )
    {
        if ( $i != $n[$i] - 1 ) {
            $ok = 0;
            break;
        }
    }
    $ok;
}

while ( $f = shift ) {
    open IN, $f || die "$0 $f $!\n";
    @l = <IN>;
    close IN;
    grep chomp, @l;
    grep s/^\s+//, @l;
    grep s/\s+$//, @l;
    for ( $i = 0; $i < @l; $i++ ) {
        print "$f line $i <$l[$i]>\n" if $debug;
        $_ = $l[$i];
        if ( /QString\s*\(/ ) {
            $p = $i;
            s/.*QString\s*\(//;
            $l[$i] = $_;
            print "$f found qstring, now line $i <$l[$i]>\n" if $debug;
            @c = split //;
            $s = '';
            $parens = 1;
            print "paren == $parens\n" if $debug;
            do {
                for ($j = 0; $parens && $j < @c; $j++ ) {
                    print "paren == $parens char $c[$j]\n" if $debug;
                    if ( $c[$j] eq ')' )
                    {
                        $parens--;
                    }
                    if ( $parens ) {
                        $s .= $c[$j];
                    } else {
                        break;
                    }
                }
                if ( $parens ) {
                    if ( $p < @l ) {
                        $_ = $l[++$p];
                        @c = split //;
                    } else {
                        for ( $i = 0; $i < @r; $i++ ) {
                            print $r[$i];
                        }
                        die "unclosed paren\n";
                    }
                }
            } while ( $parens );
            $s =~ s/\n/ /g;
            if ( $s =~ /\%/ && !is_ok($s) ) {
                push @r, "$f $i: $s\n";
            }
            --$i;
        }
    }
}

for ( $i = 0; $i < @r; $i++ ) {
    print $r[$i];
}

                
            
