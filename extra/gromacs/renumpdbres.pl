#!/usr/bin/perl

$notes = "usage: $0 pdbfilename
Renumber all resnums of SOLs only
starts with resnum of 1st SOL
";

$f = shift || die $notes;

sub docmd {
    my $cmd = shift;
    print "$cmd\n";
    print `$cmd`;
    die "command returned bad status $!\n" if $?;
}

do {
    die "!exist $f\n" if !-e $f;
    open my $fh, $f;
    @l = <$fh>;
    close $fh;
    
    my @lnew;
    $resnum = 0;

    for $l ( @l ) {
        if ( $l !~ /^ATOM/ ) {
            push @lnew, $l;
            next;
        }

        $resnam = substr( $l, 17, 3 );
        if ( $resnam !~ /^SOL/ ) {
            push @lnew, $l;
            next;
        }

        if ( !$resnum ) {
            $resnum = substr( $l, 22, 4 );
            $resnum =~ s/ //g;
            push @lnew, $l;
            next;
        }

        $resnum++;
        $l = substr( $l, 0, 22 ) . sprintf( "%4d", $resnum ) . substr( $l, 26 );
        push @lnew, $l;
    }

    $fo = $f;
    open OUT, ">$fo";
    print OUT join '', @lnew;
    close OUT;
} while ( $f = shift );


