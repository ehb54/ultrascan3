#!/usr/bin/perl

# *** config ***
$mustang = "mustang-3.2.3";
$parallel = 16;

$notes = "usage: $0 SOL|WAT pdbfilenames
uses mustang to relign all pdbs to first pdb
puts in subdir aligned
";

$SOLWAT = shift || die $notes;
die "first argument must be SOL or WAT\n" if $SOLWAT !~ /^(SOL|WAT)$/;
$issol = $SOLWAT eq "SOL";

die $notes if @ARGV < 3;


sub docmd {
    my $cmd = shift;
    print "$cmd\n";
    print `$cmd`;
    die "command returned bad status $!\n" if $?;
}


# stage 1, copy all files to align subdir

docmd( "rm -fr align" );
docmd( "mkdir align" );

while ( $f = shift ) {
    die "!exist $f\n" if !-e $f;
    open my $fh, $f;
    @l = <$fh>;
    close $fh;
    
    push @f, $f;

    my @lnew;

    for $l ( @l ) {
        if ( $l !~ /^ATOM/ ) {
            push @lnew, $l;
            next;
        }

        $orgchain = substr( $l, 21, 1 );

        $resnam = substr( $l, 17, 3 );
        if ( $resnam !~ /^$SOLWAT/ ) {
            push @lnew, $l;
            next;
        }

        $atomname = substr( $l, 13, 4 );

        if ( $atomname =~ /^MW  / ) {
            next;
        }

        $l =~ s/OW  $SOLWAT/N   GLY/;
        $l =~ s/^(.{55}) 1.00 /\1 0.71 /;

        push @lnew, $l;
    }

    $fo = "align/$f";
    open OUT, ">$fo";
    print OUT join '', @lnew;
    close OUT;
}

chdir "align" || die "chdir align $!\n";

# stage2a run in parallel
$cmd = "";

for ( $i = 1; $i < @f; ++$i ) {
    $cmd .= "$mustang -o mustang-$i -i $f[0] $f[$i]&\n";
    if ( !( ($i - 1 ) % $parallel ) ) {
        $cmd .= "wait\n";
    }
}

docmd( $cmd );

# stage 2b, extract results

for ( $i = 1; $i < @f; ++$i ) {
    $f = "mustang-$i.pdb";
    die "!exist $f\n" if !-e $f;

    open my $fh, $f;
    @l = <$fh>;
    close $fh;

    my @lnew;

    $atomspushed = 0;

    for $l ( @l ) {
        if ( !$atomspushed && $l =~ /^TER/ ) {
            next;
        }

        if ( $l !~ /^ATOM/ ) {
            if ( $l =~ /^TER/ ) {
                $l =~ s/^(.{21})./\1$orgchain/;
            }
            push @lnew, $l;
            next;
        }

        $chain = substr( $l, 21, 1 );
        if ( $chain ne 'B' ) {
            next;
        }
        $l =~ s/^(.{21})./\1$orgchain/;

        $atomspushed++;

        $resnam = substr( $l, 17, 3 );
        $atomname = substr( $l, 13, 4 );
        $occ = substr( $l, 56, 4 );
        if ( $resnam !~ /^GLY/ ||
             $atomname ne "N   " ||
             $occ ne "0.71" ) {
            push @lnew, $l;
            next;
        }

        $l =~ s/N   GLY/OW  $SOLWAT/;
        $l =~ s/^(.{55}) 0.71 /\1 1.00 /;

        push @lnew, $l;
    }

    $fo = "$f[$i]";
    open OUT, ">$fo";
    print OUT join '', @lnew;
    close OUT;
    docmd( "rm mustang-$i.pdb mustang-$i.html" );
}

# stage 3 clean up

docmd( "cp ../$f[0] ." );
