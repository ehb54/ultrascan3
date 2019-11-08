#!/usr/bin/perl

$notes = "usage: $0 files
finds funky html characters are fixes them
e.g. &iacute; etc
";

die $notes if !@ARGV;

mkdir "new";

my %m;

while ( $f = shift ) {
    die "$f not not found or not readable\n" if !-e $f || !-r $f;

    open my $fh, $f || die "can not open $f $!\n";
    my @l = <$fh>;
    close $fh;

    undef @any;

    push @any, grep s/\xb0/\&deg;/g, @l;
    push @any, grep s/\xec/\&igrave;/g, @l;
    push @any, grep s/\xed/\&iacute;/g, @l;
    push @any, grep s/\xe8/\&egrave;/g, @l;
    push @any, grep s/\xe9/\&eacute;/g, @l;
    push @any, grep s/\xf3/\&oacute;/g, @l;
    push @any, grep s/\xa0/\&nbsp;/g, @l;
    push @any, grep s/\xb1/\&plusmn;/g, @l;
    push @any, grep s/\xc2/\&Acirc;/g, @l;
    push @any, grep s/\xc5/\&Aring;/g, @l;
    push @any, grep s/\xd7/\&times;/g, @l;

    my @left = map /([\x80-\xff])/, @l;
    if ( @left ) {
        for my $k ( @left ) {
            $m{$k}++;
        }
    }

    if ( @any ) {
        print "$f\n";
        open my $fh, ">new/$f";
        print $fh join '', @l;
        close $fh;
    }
}


if ( keys %m ) {
    print "high bit characters left:\n";
    for my $k ( keys %m ) {
        print sprintf( "0x%lx\n", ord $k );
    }
    print "you may be able to find info on https://www.fileformat.info/info/unicode/char/00XY/index.htm\ne.g.\n";
    for my $k ( keys %m ) {
        print sprintf( "https://www.fileformat.info/info/unicode/char/00%lx/index.htm\n", ord $k );
    }
}
